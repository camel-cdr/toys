#include "textflag.h"
#include "funcdata.h"
#include "go_asm.h"


TEXT ·histogramIntelPopcAsm(SB), NOSPLIT|NOFRAME, $0-24


    // set constants
     VPTERNLOGD      $0xff, Z10, Z10, Z10
     VPABSD          Z10, Z10                // Z10 = uint32(1)
     VPSLLD          $5, Z10, Z11            // Z11 = uint32(32)

     // load parameters
     MOVQ            ptr+0(FP), R10          // R10 = &input[0]
     MOVQ            count+8(FP), BX         // BX  = len(input)
     MOVQ            output+16(FP), R15      // R15 = &output[0]

     XORQ            CX, CX

histogram_loop:
    // load input
    VMOVDQU32       (R10)(CX*4), Z0         // Z0 = input bins

    VPCONFLICTD     Z0, Z1                  // Z1 - conflict maps
    VPOPCNTD        Z1, Z2
    VPADDD          Z10, Z2, Z2             // Z2 - increment


    // load partial conuter and update
    KXNORQ          K1, K1, K1
    KMOVQ           K1, K2
    VPXORD          Z1, Z1, Z1
    VPGATHERDD      (R15)(Z0*4), K1, Z1
    VPADDD          Z2, Z1, Z1
    VPSCATTERDD     Z1, K2, (R15)(Z0*4)

    // loop handling
    ADDQ            $16, CX
    CMPQ            CX, BX
    JB              histogram_loop

    RET
