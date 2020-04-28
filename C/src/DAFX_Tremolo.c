//
//  DAFX_Tremolo.c
//  Tremolo~
//

#include "DAFX_Tremolo.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_InitTremolo.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>


bool SetRate(t_DAFXTremolo *pTREM, int rate_bpm)
{
    float f = DAFX_MAX((float)rate_bpm, 0.0) * 0.01666667; // *(1/60)
    SetFrequency(pTREM->p_LFO, f);
    
    pTREM->rate_bpm = rate_bpm;
    
    return true;
}

bool SetDepth(t_DAFXTremolo *pTREM, int depth_percent)
{
    pTREM->depth_percent = DAFX_MAX(DAFX_MIN(depth_percent, 100.0), 0.0);
    
    //100% depth corresponds to 1.0 peak to peak amplitude --> 0.5 amplitde --> 0.25 offset
    float depth = (float) pTREM->depth_percent * 0.01;
    float offset = 1.0 - 0.5 * depth;
    float amp = (1.0 - offset) * pTREM->amplification;
    float clip_h = 1.0; //This is always at 1.0    // TODO: necessary?
    float clip_l = 1.0 - depth; //bottom value (tip of the trough, when wave is a sine)
    
    SetOffset(pTREM->p_LFO, offset);
    SetAmplitude(pTREM->p_LFO, amp);    
    SetClipHigh(pTREM->p_LFO, clip_h); // TODO: necessary?
    SetClipLow(pTREM->p_LFO, clip_l);
    
    return true;
}

bool SetSharpness(t_DAFXTremolo *pTREM, float sharpness)
{
    // sharpness value of (0-1) maps to LFO amplification value of (1-5)
    pTREM->amplification = 4.0 * (float)sharpness + 1.0;
    
    float amp = (float) pTREM->depth_percent * 0.01 * 0.5 * pTREM->amplification; //Peak-to peak amplude * 1/2
    SetAmplitude(pTREM->p_LFO, amp);
    
    pTREM->sharpness = sharpness;
    
    return true;
}

bool InitDAFXTremolo(t_DAFXTremolo *pTREM)
{
    // ---- general, wrapper ---- //
    int block_size = pTREM->block_size;
    pTREM->p_input_block = (float *) calloc(block_size, sizeof(float));
    pTREM->p_output_block = (float *) calloc(block_size, sizeof(float));
    
    //allocate and init LFO
    pTREM->p_LFO = (t_DAFXLowFrequencyOscillator *) malloc(sizeof(t_DAFXLowFrequencyOscillator));
    pTREM->p_LFO->fs = pTREM->fs;
    pTREM->p_LFO->block_size = block_size;
    InitDAFXLowFrequencyOscillator(pTREM->p_LFO);
    SetMode(pTREM->p_LFO, LFO_ALGO_SELECT_SIN);
    
    // -- Tremolo params -- //
    SetRate(pTREM, TREM_INIT_DEFAULT_RATE_BMP);
    SetSharpness(pTREM, TREM_INIT_DEFAULT_SHARPNESS);
    SetDepth(pTREM, TREM_INIT_DEFAULT_DEPTH_PERCENT);
    
    return true;
}

bool DAFXTremolo(t_DAFXTremolo *pTREM)
{
    int block_size = pTREM->block_size;
    float *pInput = pTREM->p_input_block;
    float *pOutput = pTREM->p_output_block;
    float *p_lfo_buff = pTREM->p_LFO->p_output_block;

    //First, generate the LFO signal with a single call to its sample generator function
    DAFXLowFrequencyOscillator(pTREM->p_LFO);
    
    for (int i = 0; i < block_size; i++) {
        pOutput[i] = pInput[i] * p_lfo_buff[i];
    }
    
    return true;
}

bool DAFXBypassTremolo(t_DAFXTremolo *pTREM)
{
    memcpy(pTREM->p_output_block, pTREM->p_input_block, sizeof(float) * pTREM->block_size);
    return true;
}

void DeallocDAFXTremolo(t_DAFXTremolo *pTREM)
{
    FREE(pTREM->p_input_block);
    FREE(pTREM->p_output_block);
}
