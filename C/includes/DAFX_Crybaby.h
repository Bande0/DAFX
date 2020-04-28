//
//  DAFX_Crybaby.h
//  Crybaby~
//


#ifndef DAFX_Crybaby_h
#define DAFX_Crybaby_h


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
    
    typedef enum
    {
        CB_BYPASS_CRYBABY = 0,
        CB_MANUAL_CRYBABY,
        CB_AUTO_CRYBABY,
        Crybaby_N_ALGOS,
    }t_cb_algo_select;
    
    typedef struct{
        
        int block_size;
        int fs;
        
        float *p_input_block;
        float *p_output_block;
        float *p_biquad_coeffs;
        
        t_DAFX_BiquadFilter *p_biquad;
        t_DAFXLowFrequencyOscillator *pLFO;
        
        float wah_balance;
        
        //knob value
        float gp;
        
        //coeffs
        float a0;
        float a0b;
        float a0c;
        float a1;
        float a1b;
        float a1c;
        float a2;
        float a2b;
        float a2c;
        float b0;
        float b1;
        float b2;
        
    }t_DAFXCrybaby;
    
    
    /*!
     * @brief Init Crybaby struct and allocate memory
     *
     * @param pointer on a Crybaby structure
     * @return process status
     */
    bool InitDAFXCrybaby( t_DAFXCrybaby *pCB);    
    
    /*!
     * @brief Updates the pedal position and generates new 
     * biquad coefficients accordingly
     *
     * @param pointer on Crybaby structure
     * @param pedal position (Must be between 0 and 1)
     * @return process status
     */
    bool UpdatePedalPos(t_DAFXCrybaby *pCB, float pedal_pos);
    
    /*!
     * @brief Process and Apply Crybaby to incoming signal
     *
     * @param pointer on Crybaby structure
     * @return process status
     */
    bool DAFXProcessCrybaby(t_DAFXCrybaby *pCB);
    
    /*!
     * @brief Process and Apply Auto-Crybaby to incoming signal
     *
     * @param pointer on Crybaby structure
     * @return process status
     */
    bool DAFXProcessAutoCrybaby(t_DAFXCrybaby *pCB);
    
    /*!
     * @brief Bypass Crybaby of incoming signal
     *
     * @param pointer on Crybaby structure
     * @return process status
     */
    bool DAFXBypassCrybaby(t_DAFXCrybaby *pCB);
    
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on Crybaby structure
     * @return void
     */
    void DeallocDAFXCrybaby(t_DAFXCrybaby *pCB);
    
    //Setters
    bool Crybaby_SetLFOMode(t_DAFXCrybaby *pCB, t_lfo_algo_select mode);
    bool Crybaby_SetLFORate(t_DAFXCrybaby *pCB, int rate_bpm);
    bool Crybaby_SetLFOAmplitude(t_DAFXCrybaby *pCB, float amp);
    bool Crybaby_SetLFOBalance(t_DAFXCrybaby *pCB, float bal);
    bool Crybaby_SetLFOOffset(t_DAFXCrybaby *pCB, float offset);
    bool Crybaby_SetLFOClipHigh(t_DAFXCrybaby *pCB, float clip_h);
    bool Crybaby_SetLFOClipLow(t_DAFXCrybaby *pCB, float clip_l);
    bool Crybaby_ReinitLFOPhase(t_DAFXCrybaby *pCB);
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_Crybaby_h */
