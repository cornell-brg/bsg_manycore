#include "brg_hcc_utils.h"

int main()
{
  brg_hcc_set_tile_x_y();
  // int i = 0;
  // char stderr_msg[] = "Hello world from BRG-HCC tile!\n";
  // while(stderr_msg[i]) {
  //   bsg_putchar_err(stderr_msg[i]);
  //   cache_flush();
  //   i++;
  // }

  /************************************************************************
    Terminates the Simulation
  *************************************************************************/

  brg_hcc_finish(); // similar to bsg_finish() but with appropriate cache flush()

  // should not be reachable
  bsg_wait_while(1);
  return 0;
}

