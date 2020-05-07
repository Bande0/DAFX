//
//  DAFX_IntegerSampleDelayLine.c
//

#include "DAFX_IntegerSampleDelayLine.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>


bool SetDelayMs(t_DAFXIntegerSampleDelayLine *pDEL, float delay_ms)
{
    float d_ms = DAFX_MAX(delay_ms, 0.0);  //delay can't be negative
    
    //convert to samples
    int d_samples = (int)(d_ms * 0.001 * pDEL->fs + 1);
    
    //can't be larger than the buffer size
    pDEL->delay_samples = DAFX_MIN(d_samples, pDEL->buf_size);
    
    //update the read pointer with the new delay
    pDEL->rp = (pDEL->wp - pDEL->delay_samples) % pDEL->buf_size;
  
    return true;
}

bool SetMaxDelayMs(t_DAFXIntegerSampleDelayLine *pDEL, float max_delay_ms)
{
   //TODO: This one has to reallocate the signal buffer!
    return true;
}

bool InitDAFXIntegerSampleDelayLine(t_DAFXIntegerSampleDelayLine *pDEL, int fs)
{
    pDEL->fs = fs;
    
    //Allocate to inital max delay value - can be changed later
    pDEL->buf_size = (int)(INIT_DELAYLINE_MAX_DELAY_MS * 0.001 * fs) + 1;
    pDEL->p_delay_buffer = (float *) calloc(pDEL->buf_size, sizeof(float));
    
    //init to default delay
    pDEL->delay_samples = (int)(INIT_DELAYLINE_DELAY_MS * 0.001 * fs);
    
    //init read and write pointers
    pDEL->wp = 0;
    pDEL->rp = pDEL->buf_size - pDEL->delay_samples;
    
    return true;
}

float DAFXProcessDelaySingleSample(t_DAFXIntegerSampleDelayLine *pDEL, float x)
{
    //write new sample into delay line
    pDEL->p_delay_buffer[pDEL->wp] = x;
    
    //advance read and write pointers circularly
    pDEL->wp = (pDEL->wp + 1) % pDEL->buf_size;
    pDEL->rp = (pDEL->rp + 1) % pDEL->buf_size;
    
    //read out buffered sample
    return pDEL->p_delay_buffer[pDEL->rp];
}

float DAFXBypassDelaySingleSample(t_DAFXIntegerSampleDelayLine *pDEL, float x)
{
    return x;
}

void DeallocDAFXIntegerSampleDelayLine(t_DAFXIntegerSampleDelayLine *pDEL)
{
    FREE(pDEL->p_delay_buffer);
}
