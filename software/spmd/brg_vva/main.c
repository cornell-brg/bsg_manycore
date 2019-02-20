
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"

#define TILE_DIM        4
#define NUM_X_TILES     TILE_DIM
#define NUM_Y_TILES     TILE_DIM
#define N         	16*TILE_DIM


int Source1 [N]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
int Source2 [N]={10,20,30,40,50,60,70,80,90,100,110,120,130,140,150};
int Result [N];

void vector_vector_add( int * source_1, int * source_2, int * dest)
  {
//    bsg_printf("\n Core (%d, %d).\n", bsg_x, bsg_y);
    for (int i = 0; i<TILE_DIM; i++)
      {
        *(dest+i) = *(source_1+i) + *(source_2+i);

      }	
  }

int main()
  {
    
    bsg_set_tile_x_y();
   //  if((bsg_x==0)&&(bsg_y==0)) 
    vector_vector_add( &Source1[0], &Source2[0],&Result[0]); 
   //  if((bsg_x==0)&&(bsg_y==1))
    vector_vector_add( &Source1[4], &Source2[4],&Result[4]);
  //  if((bsg_x==0)&&(bsg_y==2))
    vector_vector_add( &Source1[8], &Source2[8],&Result[8]);
   // if((bsg_x==0)&&(bsg_y==3))
    vector_vector_add( &Source1[12], &Source2[12],&Result[12]);
   // if((bsg_x==1)&&(bsg_y==0))
    vector_vector_add( &Source1[16], &Source2[16],&Result[16]);
   // if((bsg_x==1)&&(bsg_y==1))
    vector_vector_add( &Source1[20], &Source2[20],&Result[20]);
  //  if((bsg_x==1)&&(bsg_y==2))
    vector_vector_add( &Source1[24], &Source2[24],&Result[24]);
  //  if((bsg_x==1)&&(bsg_y==3))
    vector_vector_add( &Source1[28], &Source2[28],&Result[28]);
  //  if((bsg_x==2)&&(bsg_y==0))
    vector_vector_add( &Source1[32], &Source2[32],&Result[32]);
  //  if((bsg_x==2)&&(bsg_y==1))
    vector_vector_add( &Source1[36], &Source2[36],&Result[36]);
  //  if((bsg_x==2)&&(bsg_y==2))
    vector_vector_add( &Source1[40], &Source2[40],&Result[40]);
  //  if((bsg_x==2)&&(bsg_y==3))
    vector_vector_add( &Source1[44], &Source2[44],&Result[44]);
  //  if((bsg_x==3)&&(bsg_y==0))
    vector_vector_add( &Source1[48], &Source2[48],&Result[48]);
  //  if((bsg_x==3)&&(bsg_y==1))
    vector_vector_add( &Source1[52], &Source2[52],&Result[52]);
  //  if((bsg_x==3)&&(bsg_y==2))
    vector_vector_add( &Source1[56], &Source2[56],&Result[56]);
  //  if((bsg_x==3)&&(bsg_y==3))
    vector_vector_add( &Source1[60], &Source2[60],&Result[60]);

    if((bsg_x==0)&&(bsg_y==0))
      {
       for (int i =0; i<16; i++ )
        bsg_printf("\n %d \n ",Result[i] );
        bsg_finish(); 
      }

  bsg_wait_while(1);
  }

