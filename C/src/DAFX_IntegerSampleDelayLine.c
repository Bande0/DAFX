//
//  DAFX_IntegerSampleDelayLine.c
//

#include "DAFX_IntegerSampleDelayLine.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>

//crazy modulo function that works for both negative and positive numbers
static int modulo (int x,int N)
{
    return (x % N + N) % N;
}

bool DEL_SetDelayMs(t_DAFXIntegerSampleDelayLine *pDEL, float delay_ms)
{
    //delay can't be negative and can't be larger than the max
    pDEL->delay_ms = DAFX_MIN(DAFX_MAX(delay_ms, 0.0), pDEL->max_delay_ms);
    
    //convert to samples
    int d_samples = (int)(pDEL->delay_ms * 0.001 * pDEL->fs);
    
    //can't be larger than the buffer size
    //TODO: NEEDED?
    pDEL->delay_samples = DAFX_MIN(d_samples, pDEL->buf_size - 1);
    
    //update the read pointer with the new delay
    pDEL->rp = modulo(pDEL->wp - pDEL->delay_samples, pDEL->buf_size);
  
    return true;
}

bool DEL_SetMaxDelayMs(t_DAFXIntegerSampleDelayLine *pDEL, float max_delay_ms)
{
    //max delay cant be lower than the minimum size or the current delay
    float d_max_ms = DAFX_MAX(max_delay_ms, MIN_DELAYLINE_SIZE_MS);
    pDEL->max_delay_ms = DAFX_MAX(d_max_ms, pDEL->delay_ms);
    
    // free up current buffer
    FREE(pDEL->p_delay_buffer);
    
    //Allocate new buffer to new size
    pDEL->buf_size = (int)(pDEL->max_delay_ms * 0.001 * pDEL->fs) + 1;
    pDEL->p_delay_buffer = (float *) calloc(pDEL->buf_size, sizeof(float));
    
    //reinit read and write pointers
    pDEL->wp = 0;
    pDEL->rp = pDEL->buf_size - pDEL->delay_samples;

    return true;
}

bool InitDAFXIntegerSampleDelayLine(t_DAFXIntegerSampleDelayLine *pDEL, int fs)
{
    pDEL->fs = fs;
    
    //Allocate to inital max delay value - can be changed later
    pDEL->buf_size = (int)(INIT_DELAYLINE_MAX_DELAY_MS * 0.001 * fs) + 1;
    pDEL->max_delay_ms = INIT_DELAYLINE_MAX_DELAY_MS;
    pDEL->p_delay_buffer = (float *) calloc(pDEL->buf_size, sizeof(float));
    
    //init to default delay
    pDEL->delay_samples = (int)(INIT_DELAYLINE_DELAY_MS * 0.001 * fs);
    pDEL->delay_ms = INIT_DELAYLINE_DELAY_MS;
    
    //init read and write pointers
    pDEL->wp = 0;
    pDEL->rp = pDEL->buf_size - pDEL->delay_samples;
    
    return true;
}

float DAFXProcessDelaySingleSample(t_DAFXIntegerSampleDelayLine *pDEL, float x)
{
    float y;
    
    //write new sample into delay line
    pDEL->p_delay_buffer[pDEL->wp] = x;
    
    //read out buffered sample
    y = pDEL->p_delay_buffer[pDEL->rp];
    
    //advance read and write pointers circularly
    pDEL->wp = (pDEL->wp + 1) % pDEL->buf_size;
    pDEL->rp = (pDEL->rp + 1) % pDEL->buf_size;
    
    //read out buffered sample
    return y;
}

float DAFXBypassDelaySingleSample(t_DAFXIntegerSampleDelayLine *pDEL, float x)
{
    return x;
}

void DeallocDAFXIntegerSampleDelayLine(t_DAFXIntegerSampleDelayLine *pDEL)
{
    FREE(pDEL->p_delay_buffer);
}
