//
//  DAFX_Vibrato.c
//  Vibrato~
//

#include "DAFX_Vibrato.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_InitVibrato.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>


bool SetRate(t_DAFXVibrato *pVIB, int rate_bpm)
{
    float f = DAFX_MAX((float)rate_bpm, 0.0) * 0.01666667; // *(1/60)
    //LFO_SetFrequency(pVIB->p_LFO, f);
    
    pVIB->rate_bpm = rate_bpm;
    
    return true;
}

bool SetDepth(t_DAFXVibrato *pVIB, float depth)
{
    pVIB->depth = DAFX_MAX(DAFX_MIN(depth, 100.0), 0.0);
    // LFO_SetAmplitude(pVIB->p_LFO, amp);
       
    return true;
}

bool InitDAFXVibrato(t_DAFXVibrato *pVIB)
{
    // ---- general, wrapper ---- //
    int block_size = pVIB->block_size;
    pVIB->p_input_buffer = (float *) calloc(block_size, sizeof(float));
    pVIB->p_output_buffer = (float *) calloc(block_size, sizeof(float));
    
    //allocate and init Delay Line
    pVIB->pDEL = (t_DAFXIntegerSampleDelayLine *) malloc(sizeof(t_DAFXIntegerSampleDelayLine));
    InitDAFXIntegerSampleDelayLine(pVIB->pDEL, pVIB->fs);
    
    // -- Vibrato params -- //
    SetRate(pVIB, VIB_INIT_DEFAULT_RATE_BPM);
    SetDepth(pVIB, VIB_INIT_DEFAULT_DEPTH);
    
    return true;
}

bool DAFXVibrato(t_DAFXVibrato *pVIB)
{
    int block_size = pVIB->block_size;
    float *pInput = pVIB->p_input_buffer;
    float *pOutput = pVIB->p_output_buffer;
    //float *p_lfo_buff = pVIB->p_LFO->p_output_block;
    t_DAFXIntegerSampleDelayLine *pDEL = pVIB->pDEL;

    //First, generate the LFO signal with a single call to its sample generator function
    //DAFXLowFrequencyOscillator(pVIB->p_LFO);
    
    for (int i = 0; i < block_size; i++) {
        pOutput[i] = DAFXProcessDelaySingleSample(pDEL, pInput[i]);
    }
    
    return true;
}

bool DAFXBypassVibrato(t_DAFXVibrato *pVIB)
{
    memcpy(pVIB->p_output_buffer, pVIB->p_input_buffer, sizeof(float) * pVIB->block_size);
    return true;
}

void DeallocDAFXVibrato(t_DAFXVibrato *pVIB)
{
    FREE(pVIB->p_input_buffer);
    FREE(pVIB->p_output_buffer);
}
