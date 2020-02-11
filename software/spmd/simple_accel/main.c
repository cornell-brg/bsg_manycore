
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_token_queue.h"

int foo = 23;
int bar = 14;

#ifndef bsg_tiles_X
#error bsg_tiles_X must be defined
#endif

#ifndef bsg_tiles_Y
#error bsg_tiles_Y must be defined
#endif

#if bsg_tiles_X == 1
#define bsg_noc_xbits 1
#elif bsg_tiles_X == 2
#define bsg_noc_xbits 1
#elif bsg_tiles_X == 3
#define bsg_noc_xbits 2
#elif bsg_tiles_X == 4
#define bsg_noc_xbits 2
#elif bsg_tiles_X == 5
#define bsg_noc_xbits 3
#elif bsg_tiles_X == 6
#define bsg_noc_xbits 3
#elif bsg_tiles_X == 7
#define bsg_noc_xbits 3
#elif bsg_tiles_X == 8
#define bsg_noc_xbits 3
#elif bsg_tiles_X == 9
#define bsg_noc_xbits 4
#elif bsg_tiles_X == 16
#define bsg_noc_xbits 4
#elif
#error Unsupported bsg_tiles_X
#endif

#if bsg_tiles_Y == 1
#define bsg_noc_ybits 1
#elif bsg_tiles_Y == 2
#define bsg_noc_ybits 2
#elif bsg_tiles_Y == 3
#define bsg_noc_ybits 2
#elif bsg_tiles_Y == 4
#define bsg_noc_ybits 3
#elif bsg_tiles_Y == 5
#define bsg_noc_ybits 3
#elif bsg_tiles_Y == 6
#define bsg_noc_ybits 3
#elif bsg_tiles_Y == 7
#define bsg_noc_ybits 3
#elif bsg_tiles_Y == 8
#define bsg_noc_ybits 4
#elif bsg_tiles_Y == 9
#define bsg_noc_ybits 4
#elif bsg_tiles_Y == 16
#define bsg_noc_ybits 5
#elif
#error Unsupported bsg_tiles_Y
#endif

#define byte_to_word_address(x)  (( (unsigned int) (x)) >> 2)
#define word_to_byte_address(x)  (( (unsigned int) (x)) << 2)

#define create_pkt_yx(y,x) (((y) << bsg_noc_xbits) | (x))

int main()
{
  // set global address and print "i'm alive" message
  bsg_x = 0; bsg_y = 0;
  bsg_print_time();

  // connect the accelerators in a chain
  for (int accel=1; accel<3; accel++)
  {
    // X Y addr
    bsg_remote_int_ptr remote_pkt_addr_ptr  = bsg_remote_ptr(accel,0,0);
    bsg_remote_int_ptr remote_pkt_dest_ptr  = bsg_remote_ptr(accel,0,word_to_byte_address(1));

    // set the destination address for the accelerator
    // the two low bits are dropped; this is a word, not byte address
    *remote_pkt_addr_ptr = 2;

    // output ID is (Y,X)=(0,accel)
    *remote_pkt_dest_ptr  = create_pkt_yx(0,accel+1);
  }

  int accel=3;
  bsg_remote_int_ptr remote_pkt_addr_ptr  = bsg_remote_ptr(accel,0,0);

  {
    // have the last accelerator send its data back to us
    bsg_remote_int_ptr remote_pkt_dest_ptr  = bsg_remote_ptr(accel,0,word_to_byte_address(1));

    *remote_pkt_addr_ptr = byte_to_word_address(&foo);
    *remote_pkt_dest_ptr = create_pkt_yx(0,0);
  }

  // get a pointer to accelerator at tile 1
  bsg_remote_int_ptr remote_pkt_input_ptr = bsg_remote_ptr(1,0,word_to_byte_address(2));

  // send the accelerator 1 the value BEEEEEEF
  *remote_pkt_input_ptr = 0xBEEEEEEF;

  // helpful "printfs"
  bsg_remote_ptr_io_store(0,0xDEB0,&foo);
  bsg_remote_ptr_io_store(0,0xDEB0,foo);

  // wait until the value has changed (us -> accel 1 -> accel 2 -> accel 3 -> us
  bsg_wait_while (bsg_volatile_access(foo) == 23);

  // helpful "printfs"
  bsg_remote_ptr_io_store(0,0xDEB0,foo);

  bsg_wait_while (bsg_volatile_access(foo) != 0xBEEEEEEF);

  // set the destination address for the accelerator
  *remote_pkt_addr_ptr  = byte_to_word_address(&bar);

  // send the accelerator the value CAFEC0DE
  *remote_pkt_input_ptr  = 0xCAFEC0DE;

  // wait until the value has changed
  // wait until the value has changed (us -> accel 1 -> accel 2 -> accel 3 -> us
  bsg_wait_while (bsg_volatile_access(bar) == 14);

  // helpful "printfs"
  bsg_remote_ptr_io_store(0,0xDEB0,foo);

  bsg_wait_while (bsg_volatile_access(bar) != 0xCAFEC0DE);

  bsg_finish();

  bsg_wait_while(1);
}

