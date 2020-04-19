//
//  DAFX_InitBiquadFilter.h
//  BiquadFilter~
//
//  Created by Sebastian Curdy on 13/09/16.
//
//

#ifndef DAFX_InitBiquadFilter_h
#define DAFX_InitBiquadFilter_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "DAFX_definitions.h"

#define BIQUAD_BUFFER_LEN           DAFX_BLOCK_SIZE
    
#define BIQUAD_NUMERATOR_SIZE       3   //number of b's
#define BIQUAD_DENOMINATOR_SIZE     3   //number of a's
#define BIQUAD_FILTER_ORDER         2
    
#ifdef __cplusplus
}
#endif

#endif /* InitDAFX_BiquadFilter_h */
