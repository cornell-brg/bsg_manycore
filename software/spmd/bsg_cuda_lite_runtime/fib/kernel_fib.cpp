// This kernel computes fib(n)
// Current design:
//  - per task queue locks in dram
//  - per tile task queue in dmem

#include <stdint.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_manycore_atomic.h"

#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

#define FIB_BASE 3

#define QUEUE_SIZE 32
#define MAX_WORKERS (bsg_tiles_X * bsg_tiles_Y)

// remote pointer calculation
#define GROUP_EPA_WIDTH 18
#define GROUP_X_CORD_WIDTH 6
#define GROUP_Y_CORD_WIDTH 5
#define GROUP_X_CORD_SHIFT (GROUP_EPA_WIDTH)
#define GROUP_Y_CORD_SHIFT (GROUP_X_CORD_SHIFT+GROUP_X_CORD_WIDTH)
#define GROUP_PREFIX_SHIFT (GROUP_Y_CORD_SHIFT+GROUP_Y_CORD_WIDTH)

// put locks in the dram space
int locks[MAX_WORKERS] __attribute__ ((section (".dram"))) = {0};
volatile uint32_t done __attribute__ ((section (".dram"))) = 0;

uint32_t seed = 0;

template<typename T>
T remote_ptr(T ptr, uint32_t x, uint32_t y) {
  T r_ptr = (T)( ((1 << GROUP_PREFIX_SHIFT)
                    | (y << GROUP_Y_CORD_SHIFT)
                    | (x << GROUP_X_CORD_SHIFT)
                    | ((unsigned int) ptr)));
  return r_ptr;
}

inline uint32_t fast_rand() {
  seed = ( 214013 * seed + 2531011 );
  return ( seed >> 16 ) & 0x7FFF;
}

struct Task {
  int32_t val;
  volatile uint32_t* ref_count;
};

void lock(int tid) {
  int lock_val = 1;
  do {
    lock_val = bsg_amoswap_aq(&(locks[tid]), 1);
  } while (lock_val != 0);
  return;
}

void unlock(int tid) {
  bsg_amoswap_rl(&(locks[tid]), 0);
}

struct Task queue[QUEUE_SIZE];
uint32_t head = 0;
uint32_t tail = 0;

// enqueue operates local queue only
void enqueue(struct Task val) {
  lock(__bsg_id);
  queue[tail++] = val;
  unlock(__bsg_id);
  return;
}

// dequeue operates on local queue only
struct Task dequeue() {
  struct Task retval = {-1, 0};
  lock(__bsg_id);
  if (tail > head) {
    retval = queue[--tail];
  }
  unlock(__bsg_id);
  return retval;
}

// steal operates on remote queue
struct Task steal(int v_tid) {
  struct Task retval = {-1, 0};
  // assemble remote tail, head and queue pointers using v_tid;
  uint32_t remote_x = v_tid % bsg_tiles_X;
  uint32_t remote_y = v_tid / bsg_tiles_X;
  uint32_t* r_tail = remote_ptr<uint32_t*>(&tail, remote_x, remote_y);
  uint32_t* r_head = remote_ptr<uint32_t*>(&head, remote_x, remote_y);
  struct Task* r_queue = remote_ptr<struct Task*>(queue, remote_x, remote_y);
  // try stealing the task
  lock(v_tid);
  if (*r_tail > *r_head) {
    retval = r_queue[*r_head];
    *r_head += 1;
  }
  unlock(v_tid);
  // patch retval if it contains pointers
  retval.ref_count = remote_ptr<volatile uint32_t*>(retval.ref_count, remote_x, remote_y);
  return retval;
}

// forward declaration
int32_t fib(int32_t n);

void wait_loop(volatile uint32_t* cond_i) {
  while (*cond_i == 0) {
    // check local queue
    struct Task t = {-1, 0};
    t = dequeue();
    if (t.val >= 0) {
      // execute the task
      *t.ref_count = fib(t.val);
    } else {
      // find victom
      int vid = __bsg_id;
      while (vid == __bsg_id) {
        vid = fast_rand() % MAX_WORKERS;
      }
      t = steal(vid);
      if (t.val >= 0) {
        bsg_print_int(3154);
        // execute stolen task
        *t.ref_count = fib(t.val);
      }
    }
  }
}

int32_t fib_base(int32_t n) {
  if (n < 2)
    return n;
  else
    return fib_base(n-1) + fib_base(n-2);
}

int32_t fib(int32_t n) {
  if (n < FIB_BASE) {
    return fib_base(n);
  } else {
    volatile uint32_t ref_count = 0;
    struct Task t = {n-1, &ref_count};
    enqueue(t);
    int32_t x = fib(n-2);
    wait_loop(&ref_count);
    return ref_count + x;
  }
}

extern "C" __attribute__ ((noinline))
int kernel_fib(int n) {

  if (__bsg_id == 0) {
    // try unfreeze other tiles
    uint32_t* ptr;
    ptr = (uint32_t*)(intptr_t)(1 << 27 | 0 << 22 | 1 << 16 | 0b10000000000000 << 2 | 0);
    *ptr = 0;
    ptr = (uint32_t*)(intptr_t)(1 << 27 | 1 << 22 | 0 << 16 | 0b10000000000000 << 2 | 0);
    *ptr = 0;
    ptr = (uint32_t*)(intptr_t)(1 << 27 | 1 << 22 | 1 << 16 | 0b10000000000000 << 2 | 0);
    *ptr = 0;
  }

  // init random seed to bsg_id
  seed = __bsg_id;

  // sync
  barrier.sync();
  bsg_fence();

  if (__bsg_id == 0) {
    int r = fib(n);
    done = 1;
    bsg_fence();
    bsg_print_int(r);
  } else {
    wait_loop(&done);
  }

  barrier.sync();
  return 0;
}
