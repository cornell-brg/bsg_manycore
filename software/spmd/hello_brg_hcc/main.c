#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

int main()
{
   int i;

  /************************************************************************
   Basic IO outputs bsg_remote_ptr_io_store(IO_X_INDEX, Address, Value)
   Every core will outputs once.
  *************************************************************************/
  bsg_remote_ptr_io_store(IO_X_INDEX,0x1260,__bsg_x);

  /************************************************************************
    Terminates the Simulation
  *************************************************************************/
  bsg_finish();

  bsg_wait_while(1);

  return 0;
}

