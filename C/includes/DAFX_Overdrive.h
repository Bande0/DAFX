//
//  DAFX_Overdrive.h
//  Overdrive~
//


#ifndef DAFX_Overdrive_h
#define DAFX_Overdrive_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

#include <Accelerate/Accelerate.h>


#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct {
        
        float comp_lambda_rise;
        float comp_lambda_fall;
        float exp_lambda_rise;
        float exp_lambda_fall;
        int comp_or_exp;
        float yn_m1;
        
    }t_DAFXPowerSmoother;
    
    
    typedef struct {
        
        float *ingains;
        float *outgains;
        float *slopes;
        
    }t_DAFXGainLookupTable;
    
    
    typedef struct{
        
        int block_size;
        int fs;
        float sample_time;
        
        float *p_input_block;
        float *p_output_block;
        
        float comp_threshold;
        float comp_ratio;
        float exp_threshold;
        float exp_ratio;
        float knee_width_dB;
        float makeup;
        
        float comp_attack_time;
        float comp_release_time;
        float exp_attack_time;
        float exp_release_time;
        
        t_DAFXPowerSmoother *p_smoother;
        t_DAFXGainLookupTable *p_gain_lookup;
        
        float rms_power;
        float rms_smoothed;
        float AppliedGain_db; //for debug
        
        int use_xvad_or_vad;
        float VAD_measure;
        float VAD_measure_threshold;
        int VAD_label;
        
        float *p_displayed_gain_curve;        
        
        void * pf_gain_func;
        
    }t_DAFXOverdrive;
    
    //function pointer type that points to the method that calculates output gains from input gains
    typedef float (* tf_gain_function)(t_DAFXOverdrive *pOD, float inGain_lin);
   
    
    /*!
     * @brief Init Overdrive struct and allocate memory
     *
     * @param pointer on a Overdrive structure
     * @return process status
     */
    bool InitDAFXOverdrive( t_DAFXOverdrive *pOD);    
       
    /*!
     * @brief Process and Apply Compression to incoming signal
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    bool DAFXOverdrive(t_DAFXOverdrive *pOD);
    
    /*!
     * @brief Bypass Compression of incoming signal
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    bool DAFXBypassOverdrive(t_DAFXOverdrive *pOD);
    
    /**
     * @brief Recalculates the attack and release lambda coefficients
     *
     * @param pointer on Overdrive structure
     * @param block size
     * @return process status
     */
    bool RecalculateAttackAndReleaseTimes(t_DAFXOverdrive *pOD);
    
    /**
     * @brief Power smoother that takes an input sample and returns a smoothed output sample.
     *
     * @param x Input sample.
     * @param pSmooth Pointer to the parameter structure holding the settings for the Power Smoother.
     * @return Smoothed sample.
     */
    float PowerSmoother(float x, t_DAFXPowerSmoother *pSmooth);
    
    /*!
     * @brief Calculates output gain (linear) directly from the analytical formula
     *
     * @param pointer on Overdrive structure
     * @return output gain (linear)
     */
    float CalculateOutputGainFromFormula(t_DAFXOverdrive *pOD, float inGain_lin);
    
    /*!
     * @brief Calculates output gain (linear) by interpolating from a lookup table
     *
     * @param pointer on Overdrive structure
     * @return output gain (linear)
     */
    float CalculateOutputGainFromLookupTable(t_DAFXOverdrive *pOD, float inGain_lin);
    
    /*!
     * @brief Recalculates the gain lookup table
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    void RecalculateLookupTable(t_DAFXOverdrive *pOD);
    
    /*!
     * @brief Redraws Compressor Gain Curves - ideal and actual
     *
     * @param pointer on Overdrive structure
     * @return process status
     */
    bool ReDrawGainCurve(t_DAFXOverdrive *pOD);
    
    /*!
     * @brief Updates the compressor threshold from a dB input
     *
     * @param pointer on Overdrive structure
     * @param threshold value in dB
     * @return process status
     */
    void UpdateCompressorThreshold(t_DAFXOverdrive *pOD, float thresh_db);
    
    /*!
     * @brief Updates the compressor ratio
     *
     * @param pointer on Overdrive structure
     * @param ratio
     * @return process status
     */
    void UpdateCompressorRatio(t_DAFXOverdrive *pOD, float ratio);
    
    /*!
     * @brief Updates the expander threshold from a dB input
     *
     * @param pointer on Overdrive structure
     * @param threshold value in dB
     * @return process status
     */
    void UpdateExpanderThreshold(t_DAFXOverdrive *pOD, float thresh_db);
    
    /*!
     * @brief Updates the expander ratio
     *
     * @param pointer on Overdrive structure
     * @param ratio
     * @return process status
     */
    void UpdateExpanderRatio(t_DAFXOverdrive *pOD, float ratio);
    
    /*!
     * @brief Updates the compressor knee width from a dB input
     *
     * @param pointer on Overdrive structure
     * @param threshold value in dB
     * @return process status
     */
    void UpdateKneeWidth(t_DAFXOverdrive *pOD, float knee_width_db);
    
    /*!
     * @brief Updates the compressor knee width from a dB input
     *
     * @param pointer on Overdrive structure
     * @param new makeup gain value in dB
     * @return process status
     */
    void UpdateMakeupGain(t_DAFXOverdrive *pOD, float new_makeup_gain_db);
       
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
