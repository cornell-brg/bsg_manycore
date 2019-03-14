# Instantiate Heterogeneous Nodes in The Network
---
## Quick Start

For more general setups, please refer to [HammerBlade Manycore 
Technical Reference Manual](https://docs.google.com/document/d/1b2g2nnMYidMkcn6iHJ9NGjpQYfZeWEmMdLeO_3nLtgo/edit?usp=sharing)


1. Checkout Repos:
```bash
    git clone git@bitbucket.org:taylor-bsg/bsg_manycore.git 
    git clone git@bitbucket.org:taylor-bsg/bsg_ip_cores.git 
    cd bsg_manycore; git checkout bsg_hetero_nodes
```

2. Setup VCS:

     In your local CAD directory, create a makefile called **cadenv.mk** and add following commands to it:
```bash
    export VCS = <path to your VCS binary>
    export DVE = <path to your DVE binary>
```
    Change the *CAD\_DIR* variable in *bsg_manycore/software/mk/Makefile.paths* to point to your local CAD directory

3. Build tool-chains:
```bash
     cd bsg_manycore/software/riscv-tools
     make checkout-all                          # Takes ~ 5 mins
     make build-riscv-tools                     # Takes ~ 6 mins
```
4. Run Simulation
```bash
     cd bsg_manycore/software/spmd/bsg_hetero_nodes
     make
```
   You should the following messages in the outputs:
```
First 2 data in tile (y,x)=(0, 0)= 0, 1
First 2 data in tile (y,x)=(1, 0)= 4, 5
First 2 data in tile (y,x)=(1, 1)= 6, 7
First 2 data in tile (y,x)=(0, 1)= 2, 3
Concatenated array data:
...
        0,      1,      2,      3,      4,      5,      6,      7,
...
Interleaved array data:
        0,      2,      4,      6,      1,      3,      5,      7,
```

## Configure the heterogeneous network nodes

We specify the type of nodes in Makefile, which will then be passed to the RTL. In *bsg_manycore/software/spmd/bsg\_hetero\_nodes/Makefile*  we specify following heterogeneous configurations:
```
\# The first row is assigned to IO router and will the configuration values are ignored.
bsg_hetero_type_vec =  0, 0, 0, 0,  \
                       0, 0, 0, 0,  \
                       0, 0, 0, 0,  \
                       1, 1, 1, 1
```
Where '0' stands for Vallina Core and '1' stands for gather/scatter accelerator in this example. 

Refer to *bsg\_manycore/bsg\_manycore\_hetero\_socket.v* for heterogeneous type definitions.

## Gather/Scatter Accelerator

The Gather/Scatter Accelerator is design for transfer data between a continuous address range and a scattered address range. **Right now it only supports Gather**

### Overview
The RTL module is  [bsg\_manycore/bsg\_manycore\_gather\_scatter.v](https://bitbucket.org/taylor-bsg/bsg_manycore/src/bsg_hetero_nodes/v/bsg_manycore_hetero_socket.v). Current feature includes:

1. Three dimension addressing:
      * **epa**: Endpoint Physical Address, which is the dimension inside a node
      * **X**  : X cord of the tiles.
      * **Y**  : Y cord of the tiles.
2. Each dimension has following atrributes:   
      * **addr**  : The start address of this dimension.
          * **epa** dimension use **byte** address. 
      * **incr**  : The address increment for each element in this dimension.
          * **epa** dimension use **byte** increment.
      * **dim**   : The total number of elements in this dimension. 
3. Dynamic dimension order
   
   We can specify the order of dimension when invoking the accelerator. The lower dimension will be transversed first, and then the higher dimension. Different dimensnion orders can generate different access patterns for the same data layout. 

   For example, if we specify:
```c++
      dma_order_s = (dma_cmd_order_s) {         .epa_order = 0
                                               ,.x_order   = 1
                                               ,.y_order   = 2
                                      };
```
 Then it will concatenate the arrays in each tile. 

  If we specify:
```c++
      dma_order_s = (dma_cmd_order_s) {         .epa_order = 2
                                               ,.x_order   = 0
                                               ,.y_order   = 1
                                      };
```
 Then it will interleave the arrays in each tile. 

### CSRs of the Gather/Scatter Accelerators.

```c++
 enum {
    CSR\_CMD\_IDX =0         //command, write to start the transcation, using dma_cmd_order_s 
                             //to specify the dimension order.
   ,CSR\_SRC\_ADDR\_HI\_IDX    //Source Address Configuration High, using Norm_NPA_s format
   ,CSR\_SRC\_ADDR\_LO\_IDX    //Source Address Configuration Low,  using Norm_NPA_s format
   ,CSR\_SRC\_DIM\_HI\_IDX     //Source Dimension Configuration High, using Norm_NPA_s format
   ,CSR\_SRC\_DIM\_LO\_IDX     //Source Dimension Configuration Low,  using Norm_NPA_s format
   ,CSR\_SRC\_INCR\_HI\_IDX    //Source Increasement Configuration High, using Norm_NPA_s format
   ,CSR\_SRC\_INCR\_LO\_IDX    //Source Increasement Configuration Low,  using Norm_NPA_s format

   ,CSR\_DST\_ADDR\_IDX       //Local Desitination  addr
   ,CSR\_SIG\_ADDR\_HI\_IDX    //Signal Addr High, using Norm_NPA_s format
   ,CSR\_SIG\_ADDR\_LO\_IDX    //Signal Addr Low, using Norm_NPA_s format
   ,CSR\_NUM\_lp
} CSR\_INDX;
```

The _CSR\_SRC\_ADDR\*_, _CSR\_SRC\DIM\*_ and _CSR\_SRC\_INCR\*_ specify the **addr**, **dim** and **incr** attributes of the three dimensions. 

### Signal finish

The CSR\_SIG\_ADDR\_\* specifies the address that will be write back to when the gather is done. The accelerator will write '1' to this address. 

### Normalized Network Physical Address (Norm_NPA)

The accelerator dimensions parameters and the signal address are configured with 64 bits **Norm_NPA**, which are defined as following:

```c++
typedef union {
        struct{ 
                unsigned int    epa32     ;               //LSB
                unsigned char   x8        ;
                unsigned char   y8        ;
                unsigned char   chip8     ;
                unsigned char   reserved8 ;               //MSB
        };
        struct{
               unsigned int LO;
               unsigned int HI;
        };
} Norm_NPA_s;
```
