//
//  DAFX_Crybaby.c
//  Crybaby~
//

#include "DAFX_Crybaby.h"
#include "DAFX_InitCrybaby.h"
#include "DAFX_BiquadFilter.h"
#include "DAFX_InitBiquadFilter.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>

bool Crybaby_ReinitLFOPhase(t_DAFXCrybaby *pCB)
{
    LFO_ReinitPhase(pCB->pLFO);
    return true;
}

bool Crybaby_SetLFOMode(t_DAFXCrybaby *pCB, t_lfo_algo_select mode)
{
    LFO_SetMode(pCB->pLFO, mode);
    return true;
}

bool Crybaby_SetLFORate(t_DAFXCrybaby *pCB, int rate_bpm)
{
    float f = DAFX_MAX((float) rate_bpm, 0.0) * 0.01666667; // *(1/60)
    LFO_SetFrequency(pCB->pLFO, f);
    return true;
}

bool Crybaby_SetLFOAmplitude(t_DAFXCrybaby *pCB, float amp)
{
    LFO_SetAmplitude(pCB->pLFO, amp);
    return true;
}

bool Crybaby_SetLFOBalance(t_DAFXCrybaby *pCB, float bal)
{
    LFO_SetBalance(pCB->pLFO, DAFX_MAX(DAFX_MIN(bal, CB_MAX_LFO_BALANCE), CB_MIN_LFO_BALANCE));
    return true;
}

bool Crybaby_SetLFOOffset(t_DAFXCrybaby *pCB, float offset)
{
    LFO_SetOffset(pCB->pLFO, offset);
    return true;
}

bool Crybaby_SetLFOClipHigh(t_DAFXCrybaby *pCB, float clip_h)
{
    LFO_SetClipHigh(pCB->pLFO, clip_h);
    return true;
}

bool Crybaby_SetLFOClipLow(t_DAFXCrybaby *pCB, float clip_l)
{
    LFO_SetClipLow(pCB->pLFO, clip_l);
    return true;
}

bool InitDAFXCrybaby(t_DAFXCrybaby *pCB)
{
    //Signal vector size
    int block_size = pCB->block_size;
    int fs = pCB->fs;
    
    // memory allocation
    pCB->p_input_block = (float *) calloc(block_size, sizeof(float));
    pCB->p_output_block = (float *) calloc(block_size, sizeof(float));
    pCB->p_biquad_coeffs = (float *) calloc(BIQUAD_DENOMINATOR_SIZE + BIQUAD_NUMERATOR_SIZE, sizeof(float));
    pCB->p_biquad = (t_DAFX_BiquadFilter *) calloc(1, sizeof(t_DAFX_BiquadFilter));
    
    //allocate and init LFO
    pCB->pLFO = (t_DAFXLowFrequencyOscillator *) calloc(1, sizeof(t_DAFXLowFrequencyOscillator));
    pCB->pLFO->fs = pCB->fs;
    pCB->pLFO->block_size = block_size;
    InitDAFXLowFrequencyOscillator(pCB->pLFO);
    LFO_SetMode(pCB->pLFO, CB_INIT_LFO_MODE);
    LFO_SetFrequency(pCB->pLFO, CB_INIT_LFO_FREQ_HZ);
    LFO_SetAmplitude(pCB->pLFO, CB_INIT_LFO_AMP);
    LFO_SetOffset(pCB->pLFO, CB_INIT_LFO_OFFSET);
    LFO_SetClipHigh(pCB->pLFO, CB_INIT_LFO_CLIP_H);
    LFO_SetClipLow(pCB->pLFO, CB_INIT_LFO_CLIP_L);
    LFO_SetBalance(pCB->pLFO, CB_INIT_LFO_BALANCE);
   
    //Useful params
    float w0 = 2.0 * ONE_PI * CB_INIT_F0 / (float)fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s / (2.0 * CB_INIT_Q);
    
    //init knob value
    pCB->gp = 0.0;
    
    //balance between clean and wah-ed signal
    pCB->wah_balance = CB_INIT_WAH_BALANCE;
    
    //high-pass filter coeffs
    float b0h = (1.0 + c) * 0.5;
    float b1h = -(1.0 + c);
    float b2h = (1.0 + c) * 0.5;
    float a0h = 1.0 + alpha;
    float a1h = -2.0 * c;
    float a2h = 1.0 - alpha;
    
    //bandpass filter coeffs
    float b0b = CB_INIT_Q * alpha;
    //float b1b = 0.0;
    float b2b = -1.0*CB_INIT_Q * alpha;
    pCB->a0b = 1.0 + alpha;
    pCB->a1b = -2.0 * c;
    pCB->a2b = 1.0 - alpha;
    
    //numerator coeffs
    pCB->b0 = CB_INIT_GBPF * b0b + CB_INIT_GI * pCB->a0b;
    pCB->b1 = CB_INIT_GI * pCB->a1b;
    pCB->b2 = CB_INIT_GBPF * b2b + CB_INIT_GI * pCB->a2b;
    
    // Constants to make denominator coefficients computation more efficient
    pCB->a0c = -1.0*CB_INIT_GF * b0h;
    pCB->a1c = -1.0*CB_INIT_GF * b1h;
    pCB->a2c = -1.0*CB_INIT_GF * b2h;
    
    //denominator coeffs (init)
    pCB->a0 = pCB->a0b + pCB->gp * pCB->a0c;
    float ax = 1.0 / pCB->a0;
    pCB->a1 = (pCB->a1b + pCB->gp * pCB->a1c) * ax;
    pCB->a2 = (pCB->a2b + pCB->gp * pCB->a2c) * ax;
    pCB->a0 = 1.0;
    
    //coeffs are being copied - not the nicest implementation
    pCB->p_biquad_coeffs[0] = pCB->b0;
    pCB->p_biquad_coeffs[1] = pCB->b1;
    pCB->p_biquad_coeffs[2] = pCB->b2;
    pCB->p_biquad_coeffs[3] = pCB->a0;
    pCB->p_biquad_coeffs[4] = pCB->a1;
    pCB->p_biquad_coeffs[5] = pCB->a2;

    //Init biquad filter
    pCB->p_biquad->buffer_len = block_size;
    InitBiquadFilter(pCB->p_biquad);
    SetBiquadFilterCoeffs(pCB->p_biquad, pCB->p_biquad_coeffs);
    
    return true;
}

bool UpdatePedalPos(t_DAFXCrybaby *pCB, float pedal_pos)
{
    // bound the pedal pos. between min and max values
    float gx = DAFX_MAX(DAFX_MIN(pedal_pos, CB_PEDAL_MAX), CB_PEDAL_MIN);
    
    // Ever so slight gain adjustment to match analog curves.
    // This is not necessary in a real implementation since it represents
    // a fraction of a degree tilt on the treadle (6 degrees pot rotation)
    float aa = -0.15;
    pCB->gp = gx * (1.0 + aa);
    
    // denominator coefficients need to be computed for every change in the pot position
    // This costs 3 multiply and add plus a single 1/x operation for computation of coefficients
    // Then add 2 more multiplications for denominator and one for numerator.
    // Sum is 6 multiply and one 1/x operation per sample.  It's approximately the same
    // as running 2 fixed biquad filters, so not too bad when all is refactored
    pCB->a0 = pCB->a0b + pCB->gp * pCB->a0c;
    float ax = 1.0 / pCB->a0;
    pCB->a1 = (pCB->a1b + pCB->gp * pCB->a1c) * ax;
    pCB->a2 = (pCB->a2b + pCB->gp * pCB->a2c) * ax;
    pCB->a0 = 1.0;
    
    //coeffs are being copied - not the nicest implementation
    pCB->p_biquad_coeffs[0] = pCB->b0;
    pCB->p_biquad_coeffs[1] = pCB->b1;
    pCB->p_biquad_coeffs[2] = pCB->b2;
    pCB->p_biquad_coeffs[3] = pCB->a0;
    pCB->p_biquad_coeffs[4] = pCB->a1;
    pCB->p_biquad_coeffs[5] = pCB->a2;
    
    SetBiquadFilterCoeffs(pCB->p_biquad, pCB->p_biquad_coeffs);
    
    return true;
}

bool DAFXProcessCrybaby(t_DAFXCrybaby *pCB)
{
    float *p_input_block = pCB->p_input_block;
    float *p_output_block = pCB->p_output_block;
    float *p_bq_inbuf = pCB->p_biquad->pInBuff;
    float *p_bq_outbuf = pCB->p_biquad->pOutBuff;
    float balance = pCB->wah_balance;
    float inv_balance = 1.0 - pCB->wah_balance;
    
    //copying samples to biquad input buffer (I know I know... I don't care now.)
    for (int i = 0; i < pCB->block_size; i++) {
        p_bq_inbuf[i] = p_input_block[i];
    }
    
    ProcessBlockBiquad(pCB->p_biquad);
    
    //summing the wah-ed and clean signals
    for (int i = 0; i < pCB->block_size; i++) {
        p_output_block[i] = balance * p_bq_outbuf[i] + inv_balance * p_input_block[i];
    }    
    
    return true;
}

bool DAFXProcessAutoCrybaby(t_DAFXCrybaby *pCB)
{
    float *p_input_block = pCB->p_input_block;
    float *p_output_block = pCB->p_output_block;
    float balance = pCB->wah_balance;
    float inv_balance = 1.0 - pCB->wah_balance;
    float *p_lfo_buff = pCB->pLFO->p_output_block;
    
    float pedal_pos, out;
    
    //First, generate the LFO signal with a single call to its sample generator function
    DAFXLowFrequencyOscillator(pCB->pLFO);
    
    //Loop through the input buffers (signal buff and LFO control buff are of same length)
    // --> update pedal position, re-generate coeffs, process filter
    for (int i = 0; i < pCB->block_size; i++)
    {
        pedal_pos = DAFX_MAX(DAFX_MIN(p_lfo_buff[i], CB_PEDAL_MAX), CB_PEDAL_MIN);
        pedal_pos = 1.0 - pedal_pos;
        
        UpdatePedalPos(pCB, pedal_pos);
        out = ProcessSingleSampleBiquad(pCB->p_biquad, p_input_block[i]);
        
        p_output_block[i] = balance * out + inv_balance * p_input_block[i];
    }
    
    return true;
}

bool DAFXBypassCrybaby(t_DAFXCrybaby *pCB)
{
    memcpy(pCB->p_output_block, pCB->p_input_block, sizeof(float) * pCB->block_size);
    return true;
}

void DeallocDAFXCrybaby(t_DAFXCrybaby *pCB)
{
    FREE(pCB->p_input_block);
    FREE(pCB->p_output_block);
    FREE(pCB->p_biquad_coeffs);
    DeallocBiquadFilter(pCB->p_biquad);
}
