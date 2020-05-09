//
//  DAFX_Crossover.h
//  Crossover~
//


#ifndef DAFX_Crossover_h
#define DAFX_Crossover_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

#include <Accelerate/Accelerate.h>

#include "DAFX_BiquadFilter.h"
#include "DAFX_LowFrequencyOscillator.h"

#ifdef __cplusplus
extern "C" {
#endif
        
    typedef struct{
        
        //general, wrapper
        int block_size;
        int fs;
        float *p_input_block;
        float **pp_output_blocks;
        
        //LP and HP butterworth biquad coefficients (need only one set of each, they are shared across biquads)
        float *p_lp_butter_coeffs;
        float *p_hp_butter_coeffs;
        
        //Chain of LP and HP butterworth biquads
        t_DAFX_BiquadFilter **pp_lp_biquads;
        t_DAFX_BiquadFilter **pp_hp_biquads;
        
        //higher orders are achievable by increasing the number of cascaded biquads
        int num_cascades;
        
        //cutoff frequency
        int fc;
        
    }t_DAFXCrossover;
    
    /*!
     * @brief Init Crossover struct and allocate memory
     *
     * @param pointer on a Crossover structure
     * @return process status
     */
    bool InitDAFXCrossover( t_DAFXCrossover *pXOVER);
    
    /*!
     * @brief Process and Apply Crossover to incoming signal
     *
     * @param pointer on Crossover structure
     * @return process status
     */
    bool DAFXProcessCrossover(t_DAFXCrossover *pXOVER);
    
    /*!
     * @brief Bypass Crossover of incoming signal
     *
     * @param pointer on Crossover structure
     * @return process status
     */
    bool DAFXBypassCrossover(t_DAFXCrossover *pXOVER);
    
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on Crossover structure
     * @return void
     */
    void DeallocDAFXCrossover(t_DAFXCrossover *pXOVER);
    
    //Setters
    bool XOVER_SetCutoffFrequency(t_DAFXCrossover *pXOVER, int fc);
    bool XOVER_SetCascadeOrder(t_DAFXCrossover *pXOVER, int order);
    
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_Crossover_h */
