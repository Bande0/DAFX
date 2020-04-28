//
//  DAFX_InitCrybaby.h
//  Crybaby~
//


#ifndef DAFX_InitCrybaby_h
#define DAFX_InitCrybaby_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "DAFX_definitions.h"
#include "DAFX_LowFrequencyOscillator.h"

//Numeric values calculated from the python prototype
    
//Electric Components
#define CB_INIT_GF      -36.0f     //forward gain of transistor stage
#define CB_INIT_CF      1e-8f
#define CB_INIT_CI      1e-8f
#define CB_INIT_RE      155000.0f // Resistance looking into BJT emitter
#define CB_INIT_RP      27207.44680851064f
#define CB_INIT_LP      0.5f
#define CB_INIT_RI      68000.0f
#define CB_INIT_RS      1500.0f
    
//Analog boiler-plate parameters
#define CB_INIT_RPRI    19432.370523492933f
#define CB_INIT_F0      2250.7907903927653f
#define CB_INIT_Q       2.748152194338287f
    
//Combined biquad for the feedback loop
#define CB_INIT_GI      0.02158273381294964f
#define CB_INIT_GBPF    0.10398629135096286f
    
// Auto-wah LFO init settings
#define CB_INIT_LFO_MODE        LFO_ALGO_SELECT_SAW
#define CB_INIT_LFO_FREQ_HZ     1.5 //90 BPM
#define CB_INIT_LFO_AMP         1.3
#define CB_INIT_LFO_OFFSET      0.0
#define CB_INIT_LFO_CLIP_H      1.4
#define CB_INIT_LFO_CLIP_L      -0.5
#define CB_INIT_LFO_BALANCE     0.8
    
// Pedal position limits
#define CB_PEDAL_MAX    0.99f
#define CB_PEDAL_MIN    0.01f
    
#define CB_INIT_WAH_BALANCE  0.75f

    
#ifdef __cplusplus
}
#endif

#endif /* InitAdspCrybaby_h */
