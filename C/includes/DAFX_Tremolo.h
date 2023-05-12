//
//  DAFX_Tremolo.h
//  Tremolo~
//


#ifndef DAFX_Tremolo_h
#define DAFX_Tremolo_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "DAFX_definitions.h"
#else
#include <libkern/OSAtomic.h>
#include <Accelerate/Accelerate.h>
#endif

#include "DAFX_LowFrequencyOscillator.h"


#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct{
        
        //wrapper, general
        int block_size;
        int fs;
        float *p_input_block;
        float *p_output_block;
        
        t_DAFXLowFrequencyOscillator *p_LFO;
        
        // tremolo params
        int rate_bpm;
        int depth_percent;
        float sharpness;
        float amplification;
        float post_gain;
        
        //function pointer to LFO sample generation function (sine or sawtooth)
        void * pf_process_func;
        
    }t_DAFXTremolo;
    

    //function pointer type for sample generation function
    typedef float (* tf_trem_process_function)(t_DAFXTremolo *pTREM);
    
    /*!
     * @brief Init Tremolo struct and allocate memory
     *
     * @param pointer on a Tremolo structure
     * @return process status
     */
    bool InitDAFXTremolo( t_DAFXTremolo *pTREM);    
       
    /*!
     * @brief Process and Apply Tremolo to incoming signal
     *
     * @param pointer on Tremolo structure
     * @return process status
     */
    bool DAFXTremolo(t_DAFXTremolo *pTREM);
    
    /*!
     * @brief Bypass Tremolo of incoming signal
     *
     * @param pointer on Tremolo structure
     * @return process status
     */
    bool DAFXBypassTremolo(t_DAFXTremolo *pTREM);
  
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on Tremolo structure
     * @return void
     */
    void DeallocDAFXTremolo(t_DAFXTremolo *pTREM);
    
    //Setters
    bool SetRate(t_DAFXTremolo *pTREM, int rate_bpm);
    bool SetDepth(t_DAFXTremolo *pTREM, int depth_percent);
    bool SetSharpness(t_DAFXTremolo *pTREM, float sharpness);
    bool SetPostGain(t_DAFXTremolo *pTREM, float gain);
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_Tremolo_h */
