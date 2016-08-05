#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "float_common.h"

extern float input[N];

///////////////////////////////////////////////////////////////////////////////////////////////
//    ALU to FPI bypass
//    floating load --> floating compare 
//    alu load      --> floating move
float        bypass_alu_fpi_output[N]   = {0.0};
unsigned int bypass_alu_fpi_expect[N]  = { 0x1,0x0,0x1,0x0,0x1,0x3F800000,0x40000000,0x40400000};

int bypass_alu_fpi(float *src, float *dst){

  __asm__ __volatile__ ("flw f0, 0(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("flw f1, 4(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("fle.s s2, f0, f1" );  // 1.0 < 2.0, FPI.mem->FIU.frs2 

  
  __asm__ __volatile__ ("flw f2, 8(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("flw f3, 12(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("fle.s s3, f3, f2" );  // 4.0 < 3.0, FPI.wb->FIU.frs1 


  __asm__ __volatile__ ("flw f4, 16(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("flw f5, 20(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("fle.s s4, f4, f5" );  // 5.0 < 6.0, FPI.wb1->FIU.frs2 

  __asm__ __volatile__ ("flw f6, 24(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("flw f7, 28(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("fle.s s5, f7, f6" );  // 2.0 < 1.0, FPI.wb1->frs1_to_exe 

  __asm__ __volatile__ ("flw f8, 32(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("flw f9, 36(%0)"::"r"(src) ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("fle.s s6, f8, f9" );  // 3.0 <4.0, No by pass

  __asm__ __volatile__ ("lw t0, 0(%0)"::"r"(src) ); //1.0, ALU.mem--> FIU.rs1
  __asm__ __volatile__ ("fmv.s.x f10, t0"); 

  __asm__ __volatile__ ("lw t1, 4(%0)"::"r"(src) ); //2.0, ALU.wb --> FIU.rs1
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("fmv.s.x f11, t1"); 

  __asm__ __volatile__ ("lw t2, 8(%0)"::"r"(src) ); //3.0, ALU.wb --> rs1_to_exe 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("fmv.s.x f12, t2"); 

  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 
  __asm__ __volatile__ ("nop"  ); 

  __asm__ __volatile__ ("sw s2, 0(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("sw s3, 4(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("sw s4, 8(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("sw s5, 12(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("sw s6, 16(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("fsw f10, 20(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("fsw f11, 24(%0)" : :"r"(dst) ); 
  __asm__ __volatile__ ("fsw f12, 28(%0)" : :"r"(dst) ); 

}

void bypass_alu_fpi_test(float *input){
    
    int i, error =0;
    unsigned int * int_output;

    bypass_alu_fpi( input, bypass_alu_fpi_output);
    
    int_output = (unsigned int *) bypass_alu_fpi_output;
    for( i=0; i<8; i++){
        if ( int_output[i]  !=  bypass_alu_fpi_expect[i] ) {
            error = 1; 
            break;
        }
    }

    if( error == 0 ){
        bsg_remote_ptr_io_store(0, BYPASS_ALU_FPI_TESTID, PASS_CODE );
    }else{
        bsg_remote_ptr_io_store(0, BYPASS_ALU_FPI_TESTID, ERROR_CODE );
        print_value( (unsigned int *) bypass_alu_fpi_output  );
        bsg_remote_ptr_io_store(0,0x0,0x11111111);
        print_value( bypass_alu_fpi_expect );
    }
}


