//
//  DAFX_Vibrato.c
//  Vibrato~
//

#include "DAFX_Vibrato.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_InitVibrato.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>


bool VIB_SetRate(t_DAFXVibrato *pVIB, int rate_bpm)
{
    float f = DAFX_MAX((float)rate_bpm, 0.0) * 0.01666667; // *(1/60)
    LFO_SetFrequency(pVIB->pLFO, f);
    VIB_SetDepth(pVIB, pVIB->depth);
    
    pVIB->rate_bpm = rate_bpm;
    
    return true;
}

bool VIB_SetDepth(t_DAFXVibrato *pVIB, float depth)
{
    float amp;
    
    pVIB->depth = DAFX_MAX(DAFX_MIN(depth, pVIB->pDEL->max_delay_ms), 0.0);
    amp = pVIB->depth * 0.5;  //convert peak-to peak to amplitude
    
    //we need to divide the resultant amplitude by the frequency because d(sin(Ax))/dx == A * cos(Ax)
    LFO_SetAmplitude(pVIB->pLFO, amp / pVIB->pLFO->f);
    LFO_SetOffset(pVIB->pLFO, amp / pVIB->pLFO->f); //offset equals peak-to-peak * 0.5 so bottom is always zero
       
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
    
    //allocate and init LFO
    pVIB->pLFO = (t_DAFXLowFrequencyOscillator *) malloc(sizeof(t_DAFXLowFrequencyOscillator));
    pVIB->pLFO->fs = pVIB->fs;
    pVIB->pLFO->block_size = block_size;
    InitDAFXLowFrequencyOscillator(pVIB->pLFO);
    LFO_SetMode(pVIB->pLFO, LFO_ALGO_SELECT_SIN);
    
    // -- Vibrato params -- //
    VIB_SetRate(pVIB, VIB_INIT_DEFAULT_RATE_BPM);
    VIB_SetDepth(pVIB, VIB_INIT_DEFAULT_DEPTH);
    
    // get these out of the way
    LFO_SetClipLow(pVIB->pLFO, -100000);
    LFO_SetClipHigh(pVIB->pLFO, 100000);
    
    return true;
}

bool DAFXVibrato(t_DAFXVibrato *pVIB)
{
    int block_size = pVIB->block_size;
    float *pInput = pVIB->p_input_buffer;
    float *pOutput = pVIB->p_output_buffer;
    float *p_lfo_buff = pVIB->pLFO->p_output_block;
    t_DAFXIntegerSampleDelayLine *pDEL = pVIB->pDEL;

    //First, generate the LFO signal with a single call to its sample generator function
    DAFXLowFrequencyOscillator(pVIB->pLFO);
    
    for (int i = 0; i < block_size; i++) {
        DEL_SetDelayMs(pDEL, p_lfo_buff[i]);
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
