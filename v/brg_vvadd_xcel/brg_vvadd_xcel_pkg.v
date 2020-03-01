// brg_vvadd_xcel_pkg.v
//
// Author : Peitian Pan
// Date   : Feb 20, 2020
//
package brg_vvadd_xcel_pkg;

  //--------------------------------------------------------------
  //  CSR definitions
  //--------------------------------------------------------------

  typedef enum {
      CSR_CMD_IDX = 0         // command, write to start the transcation

     ,CSR_DRAM_ENABLE_IDX     // DRAM Enable
     ,CSR_NELEM_IDX           // Number of Elements
     ,CSR_A_ADDR_HI_IDX       // A Address Configuration High, using Norm_NPA_s format
     ,CSR_A_ADDR_LO_IDX       // A Address Configuration Low,  using Norm_NPA_s format
     ,CSR_B_ADDR_HI_IDX       // B Address Configuration High, using Norm_NPA_s format
     ,CSR_B_ADDR_LO_IDX       // B Address Configuration Low,  using Norm_NPA_s format
     /* ,CSR_C_ADDR_HI_IDX       // C Address Configuration High, using Norm_NPA_s format */
     /* ,CSR_C_ADDR_LO_IDX       // C Address Configuration Low,  using Norm_NPA_s format */

     ,CSR_SIG_ADDR_HI_IDX     // Signal Addr High, using Norm_NPA_s format
     ,CSR_SIG_ADDR_LO_IDX     // Signal Addr Low,  using Norm_NPA_s format

     ,CSR_DST_ADDR_IDX        // Local Desitination addr

     ,CSR_NUM_lp
  } CSR_IDX_e;

  //--------------------------------------------------------------
  //  NPA packet format
  //--------------------------------------------------------------

  // The NPA packet format used in accelerators.
  // NOTE: the accelerators should work at EVA level which
  // means we eventually want to get rid of the NPA format
  typedef struct packed {
      logic [7 : 0 ] reserved8;               //MSB
      logic [7 : 0 ] chip8    ;
      logic [7 : 0]  y8       ;
      logic [7 : 0]  x8       ;
      logic [31 : 0] epa32    ;               //LSB
  } Norm_NPA_s;

endpackage
