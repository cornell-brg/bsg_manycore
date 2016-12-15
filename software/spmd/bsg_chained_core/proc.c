/******************************************************************************
 *  Different processs with different computation/memory operation ratios 
 *
*****************************************************************************/
#include "bsg_manycore.h"
#include "chained_core.h"

proc_func_ptr func_array[ bsg_num_tiles ] = { 0 };

/////////////////////////////////////////////////////////////////////////////
// Just passt the local data to the remote
// compute/mem = 1:1
void pass_proc( int *local_ptr, volatile int *remote_ptr, int num){
    int i;
    for( i=0; i< num; i++ )     *remote_ptr = *local_ptr;
}

/////////////////////////////////////////////////////////////////////////////
// Just passt the local data to the remote
// compute/mem = 0:1
static int local_buffer[BUF_LEN];
#pragma GCC push_options
#pragma GCC optimize ("O0")
void copy_proc( int *local_ptr, volatile int *remote_ptr, int num){
    int i;
    for( i=0; i< num; i++ ) {
        local_buffer[i] = *local_ptr;
    }
   pass_proc( local_ptr, remote_ptr, num);
}
#pragma GCC pop_options

//initialize the function array with different configurations
void init_func_array(config_enum config){
    int i;
    switch( config) {
        case( eALL_PASS_FUNCS ):
            for( i=0; i< bsg_num_tiles-1; i++) func_array[i] = pass_proc;
            break;
        case( eONE_COPY_FUNCS ):
            for( i=0; i< bsg_num_tiles-1; i++) func_array[i] = pass_proc;
            func_array[1] = copy_proc;
            break;
        default:
            break;
    }
}
