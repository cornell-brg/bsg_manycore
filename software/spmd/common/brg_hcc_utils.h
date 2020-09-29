#ifndef BRG_HCC_UTILS_H
#define BRG_HCC_UTILS_H

#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

//------------------------------------------------------------------------
// CSR supported by BrgHBTile
//------------------------------------------------------------------------

#define NUMTILES  0xfc2
#define TILEID    0xfc3
#define COREID    0xf14 // core id within a tile, do not use
#define NUMCORES  0xfc1 // num. cores per tile, do not use
#define PROC2MNGR 0x7c0 // not used
#define MNGR2PROC 0xfc0 // not used
#define TILEX     0xfc4
#define TILEY     0xfc5

static inline void cache_invalidate()
{
  __asm__ __volatile__ (
    "fence r, rw" :::
  );
}

static inline void cache_flush()
{
  __asm__ __volatile__ (
    "fence rw, w" :::
  );
}

static inline int tile_id()
{
  int ret;
  __asm__ __volatile__ (
    "csrr %0, %1"
    : "=r"(ret)
    : "i"(TILEID)
    :
  );
  return ret;
}

static inline int num_tiles()
{
  int ret;
  __asm__ __volatile__ (
    "csrr %0, %1"
    : "=r"(ret)
    : "i"(NUMTILES)
    :
  );
  return ret;
}

static inline int tile_x()
{
  int ret;
  __asm__ __volatile__ (
    "csrr %0, %1"
    : "=r"(ret)
    : "i"(TILEX)
    :
  );
  return ret;
}

static inline int tile_y()
{
  int ret;
  __asm__ __volatile__ (
    "csrr %0, %1"
    : "=r"(ret)
    : "i"(TILEY)
    :
  );
  return ret;
}

//------------------------------------------------------------------------
// SPMD utils
//------------------------------------------------------------------------

// Terminates the simulation by writing to a special global address. It is
// similar to bsg_finish() but with appropriate cache_flush()
static inline void brg_hcc_finish()
{
  bsg_remote_int_ptr ptr = bsg_remote_ptr_io(IO_X_INDEX,0xEAD0);
  *ptr = ((bsg_y << 16) + bsg_x);
  cache_flush();
  while (1);
}

// Set __bsg_x, __bsg_y, and some more varialbes. It is similar to
// bsg_set_tile_x_y(), but use CSR register instead of reading dmem (which
// BRG HCC tile lacks).
static inline void brg_hcc_set_tile_x_y()
{
  __bsg_x = tile_x();
  __bsg_y = tile_y() - 2;   // tiles start at y = 2, (y=0: VCACHE, y=1: IO)
  __bsg_id = tile_id();
  __bsg_grid_dim_x = 1;
  __bsg_grid_dim_y = 1;
  __bsg_tile_group_id_x = 0;
  __bsg_tile_group_id_y = 0;
  __bsg_tile_group_id = 0;
  // TODO: does not support TGO (origin of each tile group) yet, default to x=0, y=2
  __bsg_grp_org_x = 0;
  __bsg_grp_org_y = 2;
}

#endif
