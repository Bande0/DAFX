//
//  DAFX_Vibrato.h
//


#ifndef DAFX_Vibrato_h
#define DAFX_Vibrato_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "DAFX_definitions.h"
#else
#include <libkern/OSAtomic.h>
#include <Accelerate/Accelerate.h>
#endif

#include "DAFX_IntegerSampleDelayLine.h"
#include "DAFX_LowFrequencyOscillator.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct{
        
        int block_size;
        int fs;
        float *p_input_buffer;
        float *p_output_buffer;
        
        t_DAFXIntegerSampleDelayLine *pDEL;
        t_DAFXLowFrequencyOscillator *pLFO;
        
        // Vibrato params
        int rate_bpm;
        float depth;
        
    }t_DAFXVibrato;

    
    /*!
     * @brief Init Vibrato struct and allocate memory
     *
     * @param pointer on a Vibrato structure
     * @return process status
     */
    bool InitDAFXVibrato( t_DAFXVibrato *pVIB);    
       
    /*!
     * @brief Process and Apply Vibrato to incoming signal
     *
     * @param pointer on Vibrato structure
     * @return process status
     */
    bool DAFXVibrato(t_DAFXVibrato *pVIB);
    
    /*!
     * @brief Bypass Vibrato of incoming signal
     *
     * @param pointer on Vibrato structure
     * @return process status
     */
    bool DAFXBypassVibrato(t_DAFXVibrato *pVIB);
  
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on Vibrato structure
     * @return void
     */
    void DeallocDAFXVibrato(t_DAFXVibrato *pVIB);
    
    //Setters
    bool VIB_SetRate(t_DAFXVibrato *pVIB, int rate_bpm);
    bool VIB_SetDepth(t_DAFXVibrato *pVIB, float depth);
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_Vibrato_h */
