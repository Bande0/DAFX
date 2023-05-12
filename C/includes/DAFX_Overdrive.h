//
//  DAFX_Overdrive.h
//  Overdrive~
//


#ifndef DAFX_Overdrive_h
#define DAFX_Overdrive_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "DAFX_definitions.h"
#else
#include <libkern/OSAtomic.h>
#include <Accelerate/Accelerate.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        OD_ALGO_SELECT_TANH = 0,
        OD_ALGO_SELECT_SIN,
        OD_ALGO_SELECT_EXP,
        Overdrive_N_ALGOS,
    }t_od_algo_select;
    
    
    typedef struct{
        
        int block_size;
        int fs;  //TODO: is this needed?
        float *p_input_block;
        float *p_output_block;
        
        t_od_algo_select algo;
        
        float in_gain;
        float out_gain;
        float thresh;
        float inv_thresh;
        float tan_param;
        float exp_param;
        
        // TODO: move this to the wrapper layer
        // Dependency - the displayed gain curve will be the same size as the input signal
        float *p_gain_curve;
        
    }t_DAFXOverdrive;
    
    
    /*!
     * @brief Init Overdrive struct and allocate memory
     *
     * @param pointer on a Overdrive structure
     * @return process status
     */
    bool InitDAFXOverdrive( t_DAFXOverdrive *pOD);    
       
    /*!
     * @brief Process and Apply Overdrive to incoming signal
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    bool DAFXOverdrive(t_DAFXOverdrive *pOD);
    
    /*!
     * @brief Bypass Overdrive of incoming signal
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    bool DAFXBypassOverdrive(t_DAFXOverdrive *pOD);
    
    
    // TODO: Move this to wrapper layer
    /*!
     * @brief Redraws DisplayedGain Curve
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    bool ReDrawGainCurve(t_DAFXOverdrive *pOD);
    
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on Overdrive structure
     * @return void
     */
    void DeallocDAFXOverdrive(t_DAFXOverdrive *pOD);    
    
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_Overdrive_h */
