//
//  DAFX_LowFrequencyOscillator.h
//  LowFrequencyOscillator~
//


#ifndef DAFX_LowFrequencyOscillator_h
#define DAFX_LowFrequencyOscillator_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

#include <Accelerate/Accelerate.h>


#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        LFO_ALGO_SELECT_SIN = 0,
        LFO_ALGO_SELECT_SAW,
        LowFrequencyOscillator_N_ALGOS,
    }t_lfo_algo_select;
    
    typedef enum
    {
        LFO_STATE_RISING = 0,
        LFO_STATE_FALLING,
    }t_lfo_state;
    
    typedef struct{
        
        //wrapper, general
        int block_size;
        int fs; 
        float *p_output_block;
        
        //mode selector
        t_lfo_algo_select algo;
        
        // common params
        float f;
        float amp;
        float offset;
        float clip_h;
        float clip_l;
        
        //sine specific params
        float u;
        float v;  
        float k1;
        float k2;
        
        //sawtooth specific params
        float balance;
        float y;
        float d;
        float T;
        float t1;
        float t2;
        float d_rise;
        float d_fall;
        t_lfo_state d_state;
        
        //function pointer to sample generation function
        void * pf_process_func;
        
    }t_DAFXLowFrequencyOscillator;
    

    //function pointer type for sample generation function
    typedef float (* tf_process_function)(t_DAFXLowFrequencyOscillator *pLFO);
    
    /*!
     * @brief Init LowFrequencyOscillator struct and allocate memory
     *
     * @param pointer on a LowFrequencyOscillator structure
     * @return process status
     */
    bool InitDAFXLowFrequencyOscillator( t_DAFXLowFrequencyOscillator *pLFO);    
       
    /*!
     * @brief Process and Apply LowFrequencyOscillator to incoming signal
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return process status
     */
    bool DAFXLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO);
    
    /*!
     * @brief Bypass LowFrequencyOscillator of incoming signal
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return process status
     */
    bool DAFXBypassLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO);
  
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return void
     */
    void DeallocDAFXLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO);
    
    /*!
     * @brief Recalculate internal variables
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return process status
     */
    bool _RecalculatePrivateVariables(t_DAFXLowFrequencyOscillator *pLFO);
    
    /*!
     * @brief Triggers reinitialization of the phase
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return process status
     */
    bool _ReinitPhase(t_DAFXLowFrequencyOscillator *pLFO);
    
    /*!
     * @brief Sample Generation function for Sinusoidal LFO
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return process status
     */
    float _GenerateSinusoidalLFO(t_DAFXLowFrequencyOscillator *pLFO);
    
    /*!
     * @brief Sample Generation function for Sawtooth LFO
     *
     * @param pointer on LowFrequencyOscillator structure
     * @return process status
     */
    float _GenerateSawtoothLFO(t_DAFXLowFrequencyOscillator *pLFO);
    
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_LowFrequencyOscillator_h */
