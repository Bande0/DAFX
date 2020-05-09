//
//  DAFX_IntegerSampleDelayLine.h
//


#ifndef DAFX_IntegerSampleDelayLine_h
#define DAFX_IntegerSampleDelayLine_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

#include <Accelerate/Accelerate.h>


#define INIT_DELAYLINE_DELAY_MS          5.0
#define INIT_DELAYLINE_MAX_DELAY_MS      10.0
#define MIN_DELAYLINE_SIZE_MS            1.0


#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct{
        
        int buf_size;
        int fs;
        float *p_delay_buffer;
        
        float max_delay_ms;
        float delay_ms;
        
        int delay_samples; 
        int rp;
        int wp;
        
    }t_DAFXIntegerSampleDelayLine;
    
    
    /*!
     * @brief Init IntegerSampleDelayLine struct and allocate memory
     *
     * @param pointer on a IntegerSampleDelayLine structure
     * @param sampling rate
     * @return process status
     */
    bool InitDAFXIntegerSampleDelayLine( t_DAFXIntegerSampleDelayLine *pDEL, int fs);
       
    /*!
     * @brief Process and Apply delaying with integer sample to incoming signal
     * processes a single sample at a time
     *
     * @param pointer on IntegerSampleDelayLine structure
     * @param input sample
     * @return output sample
     */
    float DAFXProcessDelaySingleSample(t_DAFXIntegerSampleDelayLine *pDEL, float x);
    
    /*!
     * @brief Bypass IntegerSampleDelayLine of incoming signal
     * processes a single sample at a time
     *
     * @param pointer on IntegerSampleDelayLine structure
     * @param input sample
     * @return output sample
     */
    float DAFXBypassDelaySingleSample(t_DAFXIntegerSampleDelayLine *pDEL, float x);
  
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on IntegerSampleDelayLine structure
     * @return void
     */
    void DeallocDAFXIntegerSampleDelayLine(t_DAFXIntegerSampleDelayLine *pDEL);
    
    //Setters
    bool DEL_SetDelayMs(t_DAFXIntegerSampleDelayLine *pDEL, float delay_ms);
    bool DEL_SetMaxDelayMs(t_DAFXIntegerSampleDelayLine *pDEL, float max_delay_ms);
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_IntegerSampleDelayLine_h */
