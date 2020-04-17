//
//  DAFX_Overdrive.c
//  Overdrive~
//

#include "DAFX_Overdrive.h"
#include "DAFX_InitOverdrive.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>

bool InitDAFXOverdrive(t_DAFXOverdrive *pOD)
{
    //Signal vector size
    int block_size = pOD->block_size;
    
    //Sample time
    pOD->sample_time = 1.0 / (float) pOD->fs;
    
    // I/O buffers
    pOD->p_input_block = (float *) calloc(block_size, sizeof(float));
    pOD->p_output_block = (float *) calloc(block_size, sizeof(float));
    
    //OD params
    pOD->comp_threshold = OD_INIT_COMP_THRESHOLD;
    pOD->comp_ratio = OD_INIT_COMP_RATIO;
    pOD->exp_threshold = OD_INIT_EXP_THRESHOLD;
    pOD->exp_ratio = OD_INIT_EXP_RATIO;
    pOD->knee_width_dB = OD_INIT_KNEE_WIDTH_DB;
    pOD->makeup = OD_INIT_MAKEUP;
    
    //Lookup table
    pOD->p_gain_lookup = (t_DAFXGainLookupTable *) malloc(sizeof(t_DAFXGainLookupTable));
    pOD->p_gain_lookup->ingains = (float *) calloc(OD_LOOKUP_SIZE, sizeof(float));
    pOD->p_gain_lookup->outgains = (float *) calloc(OD_LOOKUP_SIZE, sizeof(float));
    pOD->p_gain_lookup->slopes = (float *) calloc(OD_LOOKUP_SIZE - 1, sizeof(float));
    RecalculateLookupTable(pOD);
    
    //Power smoother
    pOD->p_smoother = (t_DAFXPowerSmoother *) malloc(sizeof(t_DAFXPowerSmoother));
    pOD->p_smoother->yn_m1 = SMOOTHER_INIT_DELAY_VALUE;
    pOD->p_smoother->comp_or_exp = SMOOTHER_INIT_MODE;
    
    //initialize attack and release times and coeffs
    pOD->comp_attack_time = OD_INIT_COMP_ATTACK_TIME;
    pOD->comp_release_time = OD_INIT_COMP_RELEASE_TIME;
    pOD->exp_attack_time = OD_INIT_EXP_ATTACK_TIME;
    pOD->exp_release_time = OD_INIT_EXP_RELEASE_TIME;
    RecalculateAttackAndReleaseTimes(pOD);
    
    pOD->rms_power = 0.0;
    pOD->rms_smoothed = 0.0;
    pOD->AppliedGain_db = 0.0;
    
    pOD->use_xvad_or_vad = OD_INIT_USE_XVAD_OR_VAD;
    pOD->VAD_measure = 0.0f;    
    pOD->VAD_measure_threshold = (pOD->use_xvad_or_vad == OD_USE_XVAD) ? OD_INIT_XVAD_THRESHOLD : OD_INIT_VAD_THRESHOLD;
    pOD->VAD_label = 0;
    
    //set gain calculation method to "lookup approach"
    pOD->pf_gain_func = &CalculateOutputGainFromLookupTable;
    
    //Allocate memory for the displayed gain curves (ideal and actual)
    pOD->p_displayed_gain_curve = (float *) calloc(block_size, sizeof(float));    
    ReDrawGainCurve(pOD);
    
    return true;
}

bool DAFXOverdrive(t_DAFXOverdrive *pOD)
{
    int block_size = pOD->block_size;
    float *pInput = pOD->p_input_block;
    float *pOutput = pOD->p_output_block;
    t_DAFXPowerSmoother *pSmooth = pOD->p_smoother;
    tf_gain_function pf_calculate_gain = (tf_gain_function) pOD->pf_gain_func; //Gain function currently pointed at
    float lin_gain = pOD->makeup;
    
    if (pOD->use_xvad_or_vad == OD_USE_VAD)
    {
        pOD->VAD_label = (pOD->VAD_measure < pOD->VAD_measure_threshold) ? 1 : 0;   //VAD: lower the value --> higher the likelihood of voice
    } else
    {
        pOD->VAD_label = (pOD->VAD_measure > pOD->VAD_measure_threshold) ? 1 : 0;  //XVAD: higher the value --> higher the likelihood of voice
    }
    
    for (int i = 0; i < block_size; i++) {
            
        pOD->rms_power = fabsf(pInput[i]);
        pOD->rms_smoothed = PowerSmoother(pOD->rms_power, pSmooth);
        
        pOD->p_smoother->comp_or_exp = (pOD->rms_smoothed < pOD->exp_threshold) ? SMOOTHER_EXP_MODE : SMOOTHER_COMP_MODE;        
        
        //if VAD == 1 and we are in expander range: don't apply the expander (i.e. gain == 1.0)
        //in all other cases, apply the OD.
        if (!((pOD->VAD_label) && (pOD->rms_smoothed < pOD->exp_threshold))){
            //Call the method for calculating the output gain
            lin_gain = pf_calculate_gain(pOD, pOD->rms_smoothed);
        }
        
        pOutput[i] = lin_gain * pInput[i];
    }
    
    pOD->AppliedGain_db = 20*log10f(lin_gain); //Only for display purposes in Max
    
    return true;
}

bool DAFXBypassOverdrive(t_DAFXOverdrive *pOD)
{
    memcpy(pOD->p_output_block, pOD->p_input_block, sizeof(float) * pOD->block_size);
    pOD->AppliedGain_db = 0.0;
    
    return true;
}

float PowerSmoother(float x, t_DAFXPowerSmoother *pSmooth)
{
    float e = 0.0;
    float lambda = 0.0;
    float y = 0.0;
    
    e = x - pSmooth->yn_m1;
    
    if(pSmooth->comp_or_exp == SMOOTHER_COMP_MODE)
    {
        if(e > 0.0)
        { // Signal power is increasing
            lambda = pSmooth->comp_lambda_rise;
        }
        else
        { // Signal power is decreasing
            lambda = pSmooth->comp_lambda_fall;
        }
    }else
    {
        if(e > 0.0)
        { // Signal power is increasing
            lambda = pSmooth->exp_lambda_rise;
        }
        else
        { // Signal power is decreasing
            lambda = pSmooth->exp_lambda_fall;
        }
    }   
    
    y = pSmooth->yn_m1 + lambda * e;
    pSmooth->yn_m1 = y; // Store delay element
    
    return y;
}

bool RecalculateAttackAndReleaseTimes(t_DAFXOverdrive *pOD)
{
    pOD->p_smoother->comp_lambda_rise = (1.0 - expf(-2.3 * pOD->sample_time / pOD->comp_attack_time)) * SMOOTHER_LAMBDA_RISE_CORRECTION;
    pOD->p_smoother->comp_lambda_fall = (1.0 - expf(-2.3 * pOD->sample_time / pOD->comp_release_time)) * SMOOTHER_LAMBDA_FALL_CORRECTION;
    pOD->p_smoother->exp_lambda_rise = (1.0 - expf(-2.3 * pOD->sample_time / pOD->exp_attack_time)) * SMOOTHER_LAMBDA_RISE_CORRECTION;
    pOD->p_smoother->exp_lambda_fall = (1.0 - expf(-2.3 * pOD->sample_time / pOD->exp_release_time)) * SMOOTHER_LAMBDA_FALL_CORRECTION;
    
    return true;
}

float CalculateOutputGainFromFormula(t_DAFXOverdrive *pOD, float inGain_lin)
{
    float comp_slope = (1.0 / pOD->comp_ratio - 1.0); //TODO: STORE THIS IN THE STRUCT INSTEAD?
    float exp_slope = pOD->exp_ratio;
    
    float comp_thresh_db = 20*log10f(pOD->comp_threshold);
    float knee_width_db = pOD->knee_width_dB;
    float knee_lower_db = comp_thresh_db - (0.5 * knee_width_db);
    float knee_upper_db = comp_thresh_db + (0.5 * knee_width_db);
    float knee_lower = powf(10.0,knee_lower_db * 0.05);
    float knee_upper = powf(10.0,knee_upper_db * 0.05);
    
    float outGain_lin = pOD->makeup;
    float knee_factor = 1.0;
    float temp = 0.0;
    
    
    if (inGain_lin < pOD->exp_threshold)
    {
        outGain_lin *= powf((inGain_lin / pOD->exp_threshold), exp_slope);
    }
    else if (inGain_lin < knee_lower)
    {
        ;
    }
    else if (inGain_lin < knee_upper)
    {
        knee_factor = (inGain_lin > 0.0) ? 20.0 * log10f(inGain_lin) : 0.0;
        knee_factor = (knee_factor - knee_lower_db) / (2 * knee_width_db);
        temp = (inGain_lin / knee_lower);
        temp = powf(temp, comp_slope);
        temp = powf(temp, knee_factor);
        outGain_lin *= temp;
    }
    else
    {
       outGain_lin *= powf((inGain_lin / pOD->comp_threshold), comp_slope);
    }
    
    return outGain_lin;
}

float CalculateOutputGainFromLookupTable(t_DAFXOverdrive *pOD, float inGain_lin)
{
    float *p_ingains = pOD->p_gain_lookup->ingains;
    float *p_outgains = pOD->p_gain_lookup->outgains;
    float *p_slopes = pOD->p_gain_lookup->slopes;
    float slope = 0.0;
    float outGain_lin = 1.0;
    
    int h_idx = 0;
    int l_idx = 0;
    //searching in the lookup table
    while (inGain_lin > p_ingains[h_idx]) {
        h_idx++;
    }
    
    //upper bound on the index (in case we receive overflown inputs, this can index out of bounds)
    h_idx = DAFX_MIN(h_idx, OD_LOOKUP_SIZE - 1);
    
    //setting the lower index
    l_idx = DAFX_MAX(h_idx - 1, 0);
    
    //corresponding slope
    slope = p_slopes[l_idx];
    
    //linear interpolation
    outGain_lin = p_outgains[l_idx] + slope * (inGain_lin - p_ingains[l_idx]);
    
    //gain has to be non-negative for small inputs
    outGain_lin = DAFX_MAX(outGain_lin, 0.0);
    
    return outGain_lin;
}

void RecalculateLookupTable(t_DAFXOverdrive *pOD)
{
    float *p_ingains = pOD->p_gain_lookup->ingains;
    float *p_outgains = pOD->p_gain_lookup->outgains;
    float *p_slopes = pOD->p_gain_lookup->slopes;
    
    float knee_width_db = pOD->knee_width_dB;
    float exp_thresh_db = 20*log10f(pOD->exp_threshold);
    float comp_thresh_db = 20*log10f(pOD->comp_threshold);
    float knee_lower_db = comp_thresh_db - (0.5 * knee_width_db);
    
    //TODO: get rid of divisions here
    float lookup_density_exp = (exp_thresh_db - OD_LOOKUP_LOWLIMIT_DB) / (OD_LOOKUP_POINTS_EXP - 1.0); //density of points for expander part
    float lookup_density_comp = (0 - knee_lower_db) / (OD_LOOKUP_POINTS_COMP - 1.0); //density of points for compressor part
    float temp_gain_lin;
    int i = 0;
    float k = OD_LOOKUP_LOWLIMIT_DB;
    
    //Filling in the input/output gain values of the lookup table
    //Expansion part
    for (; i < OD_LOOKUP_POINTS_EXP; i++)
    {
        temp_gain_lin = powf(10.0,k * 0.05);
        p_ingains[i] = temp_gain_lin;
        p_outgains[i] = CalculateOutputGainFromFormula(pOD, temp_gain_lin);
        k += lookup_density_exp;
    }
    
    //Compression part
    k = knee_lower_db;
    for (; i < OD_LOOKUP_SIZE; i++)
    {
        temp_gain_lin = powf(10.0,k * 0.05);
        p_ingains[i] = temp_gain_lin;
        p_outgains[i] = CalculateOutputGainFromFormula(pOD, temp_gain_lin);
        k += lookup_density_comp;
    }
    
    //Filling in the slope values
    for (i = 1; i < OD_LOOKUP_SIZE; i++)
    {
        p_slopes[i-1] = (p_outgains[i] - p_outgains[i-1]) / (p_ingains[i] - p_ingains[i-1]);
    }

}

//Re-draws a curve of the currently used gain function for display purposes in Max
//Calculates the output gain for all input gain values
bool ReDrawGainCurve(t_DAFXOverdrive *pOD)
{
    float lin_in_gain;
    float lin_out_gain;
    float increment = (float)OD_GAIN_CURVE_RANGE_DB / (float)pOD->block_size;
    float in_gain_db = -1.0 * OD_GAIN_CURVE_RANGE_DB;
    tf_gain_function pf_calculate_gain = (tf_gain_function) pOD->pf_gain_func; //Gain function currently pointed at
    
    for (int i = 0; i < pOD->block_size; i++)
    {
        lin_in_gain = powf(10.0,in_gain_db * 0.05);
        
        lin_out_gain = pf_calculate_gain(pOD,lin_in_gain);
        pOD->p_displayed_gain_curve[i] = 20*log10f(lin_out_gain) + in_gain_db;
        
        in_gain_db += increment;
    }
    
    return true;
}

void UpdateCompressorThreshold(t_DAFXOverdrive *pOD, float thresh_db)
{
    float temp;
    
    temp = DAFX_MAX(thresh_db, 20*log10f(pOD->exp_threshold) + 0.5 * pOD->knee_width_dB);
    temp = DAFX_MIN(temp, 0.0);  //threshold can't be higher than 0 dB
    pOD->comp_threshold = powf(10.0,temp * 0.05);
    
    //but if setting the new threshold caused overboosting, then recursively try with a lower one until it fits.
    if (1.0 < CalculateOutputGainFromFormula(pOD, 1.0))
        UpdateCompressorThreshold(pOD, thresh_db - 0.1);
    
    RecalculateLookupTable(pOD);
    ReDrawGainCurve(pOD);
}

void UpdateCompressorRatio(t_DAFXOverdrive *pOD, float ratio)
{
    pOD->comp_ratio = DAFX_MAX(ratio, 1.0);// //ratio can't be lower than 1
    
    //but if setting the new ratio caused overboosting, then recursively try with higher ones until it fits.
    if (1.0 < CalculateOutputGainFromFormula(pOD, 1.0))
        UpdateCompressorRatio(pOD, ratio + 0.25);
    
    RecalculateLookupTable(pOD);
    ReDrawGainCurve(pOD);
}

void UpdateExpanderThreshold(t_DAFXOverdrive *pOD, float thresh_db)
{
    float temp;
    
    temp = DAFX_MIN(thresh_db, 20*log10f(pOD->comp_threshold) - 0.5 * pOD->knee_width_dB);
    temp = DAFX_MIN(temp, 0.0); //threshold can't be higher than 0 dB
    
    pOD->exp_threshold = powf(10.0,temp * 0.05);
    
    RecalculateLookupTable(pOD);
    ReDrawGainCurve(pOD);
}

void UpdateExpanderRatio(t_DAFXOverdrive *pOD, float ratio)
{
    pOD->exp_ratio = DAFX_MAX(ratio, 0.0);////ratio can't be lower than 0
    
    RecalculateLookupTable(pOD);
    ReDrawGainCurve(pOD);
}

void UpdateKneeWidth(t_DAFXOverdrive *pOD, float knee_width_db)
{
    float temp;
    
    temp = DAFX_MIN(knee_width_db, 2 * (20*log10f(pOD->comp_threshold) - 20*log10f(pOD->exp_threshold)));
    temp = DAFX_MAX(temp, 0.0); //knee width can't be negative
    
    pOD->knee_width_dB = temp;
    
    RecalculateLookupTable(pOD);
    ReDrawGainCurve(pOD);
}

void UpdateMakeupGain(t_DAFXOverdrive *pOD, float new_makeup_gain_db)
{
    float maxmakeup, newmakeup;
    
    //maximum allowed makeup gain so that we don't overboost values --> output should not be more than 0dB even for fullscale inputs
    maxmakeup = pOD->makeup / CalculateOutputGainFromFormula(pOD, 1.0);
    newmakeup = powf(10.0, new_makeup_gain_db * 0.05);
    pOD->makeup = DAFX_MIN(newmakeup, maxmakeup);
    
    RecalculateLookupTable(pOD);
    ReDrawGainCurve(pOD);

}

void DeallocDAFXOverdrive(t_DAFXOverdrive *pOD)
{
    FREE(pOD->p_input_block);
    FREE(pOD->p_output_block);
    FREE(pOD->p_displayed_gain_curve);
}
