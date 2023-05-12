//
//  DAFX_Crossover.c
//  Crossover~
//

#include "DAFX_Crossover.h"
#include "DAFX_InitCrossover.h"
#include "DAFX_BiquadFilter.h"
#include "DAFX_InitBiquadFilter.h"
#include "DAFX_definitions.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <math.h>
#else
#include <Accelerate/Accelerate.h>
#endif


bool XOVER_SetCutoffFrequency(t_DAFXCrossover *pXOVER, int fc)
{
    pXOVER->fc = fc;
    
    //Useful params
    float w0 = 2.0 * ONE_PI * (float)pXOVER->fc / (float)pXOVER->fs;
    float wc = cosf(w0);
    float ws = sinf(w0);
    //alpha specifically for butterworth: ws/(2*Q) where Q == 1/sqrt(2)
    float alpha = ws * INV_SQRT_TWO;
    
    // --- Butter LP coeffs
    float a0_lp = 1.0 + alpha;
    float a1_lp = -2.0 * wc;
    float a2_lp = 1.0 - alpha;
    float b0_lp = 0.5 * (1.0 - wc);
    float b1_lp = 1.0 - wc;
    float b2_lp = 0.5 * (1.0 - wc);
    
    float ax = 1.0 / a0_lp;
    a1_lp = a1_lp * ax;
    a2_lp = a2_lp * ax;
    b0_lp = b0_lp * ax;
    b1_lp = b1_lp * ax;
    b2_lp = b2_lp * ax;
    a0_lp = 1.0;
    
    //coeffs are being copied - not the nicest implementation
    pXOVER->p_lp_butter_coeffs[0] = b0_lp;
    pXOVER->p_lp_butter_coeffs[1] = b1_lp;
    pXOVER->p_lp_butter_coeffs[2] = b2_lp;
    pXOVER->p_lp_butter_coeffs[3] = a0_lp;
    pXOVER->p_lp_butter_coeffs[4] = a1_lp;
    pXOVER->p_lp_butter_coeffs[5] = a2_lp;
    
    // --- Butter HP coeffs
    float a0_hp = 1.0 + alpha;
    float a1_hp = -2.0 * wc;
    float a2_hp = 1.0 - alpha;
    float b0_hp = -0.5 * (1.0 + wc);
    float b1_hp = 1.0 + wc;
    float b2_hp = -0.5 * (1.0 + wc);
    
    ax = 1.0 / a0_hp;
    a1_hp = a1_hp * ax;
    a2_hp = a2_hp * ax;
    b0_hp = b0_hp * ax;
    b1_hp = b1_hp * ax;
    b2_hp = b2_hp * ax;
    a0_hp = 1.0;
    
    //coeffs are being copied - not the nicest implementation
    pXOVER->p_hp_butter_coeffs[0] = b0_hp;
    pXOVER->p_hp_butter_coeffs[1] = b1_hp;
    pXOVER->p_hp_butter_coeffs[2] = b2_hp;
    pXOVER->p_hp_butter_coeffs[3] = a0_hp;
    pXOVER->p_hp_butter_coeffs[4] = a1_hp;
    pXOVER->p_hp_butter_coeffs[5] = a2_hp;
    
    //Set up LP and HP biquad filter coeffs
    for (int i = 0; i < pXOVER->num_cascades; i++) {
        //Yet another copy operation inside this set function - complete bullshit implementation
        SetBiquadFilterCoeffs(pXOVER->pp_lp_biquads[i], pXOVER->p_lp_butter_coeffs);
        SetBiquadFilterCoeffs(pXOVER->pp_hp_biquads[i], pXOVER->p_hp_butter_coeffs);
    }
    
    return true;
}

bool XOVER_SetCascadeOrder(t_DAFXCrossover *pXOVER, int order)
{
    //Not implemented yet
    return true;
}

bool InitDAFXCrossover(t_DAFXCrossover *pXOVER)
{
    //Signal vector size
    int block_size = pXOVER->block_size;
    
    //number of cascaded biquads in each channel
    pXOVER->num_cascades = XOVER_INIT_NUMOF_BIQUADS;
    
    // memory allocation
    pXOVER->p_input_block = (float *) calloc(block_size, sizeof(float));
    pXOVER->pp_output_blocks = (float **) malloc(XOVER_INIT_NUMOF_CHANNELS * sizeof(float *));
    for (int i = 0; i < XOVER_INIT_NUMOF_CHANNELS; i++) {
        pXOVER->pp_output_blocks[i] = (float *) calloc(block_size, sizeof(float));
    }
    pXOVER->p_lp_butter_coeffs = (float *) calloc(BIQUAD_DENOMINATOR_SIZE + BIQUAD_NUMERATOR_SIZE, sizeof(float));
    pXOVER->p_hp_butter_coeffs = (float *) calloc(BIQUAD_DENOMINATOR_SIZE + BIQUAD_NUMERATOR_SIZE, sizeof(float));
    
    pXOVER->pp_lp_biquads = (t_DAFX_BiquadFilter **) malloc(pXOVER->num_cascades * sizeof(t_DAFX_BiquadFilter *));
    pXOVER->pp_hp_biquads = (t_DAFX_BiquadFilter **) malloc(pXOVER->num_cascades * sizeof(t_DAFX_BiquadFilter *));
    for (int i = 0; i < pXOVER->num_cascades; i++) {
        pXOVER->pp_lp_biquads[i] = (t_DAFX_BiquadFilter *) calloc(1, sizeof(t_DAFX_BiquadFilter));
        pXOVER->pp_hp_biquads[i] = (t_DAFX_BiquadFilter *) calloc(1, sizeof(t_DAFX_BiquadFilter));
    }
    
    //Init and set up LP and HP biquad filters
    for (int i = 0; i < pXOVER->num_cascades; i++) {
        pXOVER->pp_lp_biquads[i]->buffer_len = block_size;
        pXOVER->pp_hp_biquads[i]->buffer_len = block_size;
        InitBiquadFilter(pXOVER->pp_lp_biquads[i]);
        InitBiquadFilter(pXOVER->pp_hp_biquads[i]);
    }
    
    //Connecting biquad filters - again, stupid implementation because now there are
    //some output sample buffers that are allocated but never used. I DONT CARE
    for (int i = 1; i < pXOVER->num_cascades; i++) {
        pXOVER->pp_lp_biquads[i-1]->pOutBuff = pXOVER->pp_lp_biquads[i]->pInBuff;
        pXOVER->pp_hp_biquads[i-1]->pOutBuff = pXOVER->pp_hp_biquads[i]->pInBuff;
    }
    
    //connect the input buffer to the first biquad of each channel
    pXOVER->pp_lp_biquads[0]->pInBuff = pXOVER->p_input_block;
    pXOVER->pp_hp_biquads[0]->pInBuff = pXOVER->p_input_block;
    
    //connect the output buffers of the last biquads to the main outputs
    pXOVER->pp_lp_biquads[pXOVER->num_cascades-1]->pOutBuff = pXOVER->pp_output_blocks[0];
    pXOVER->pp_hp_biquads[pXOVER->num_cascades-1]->pOutBuff = pXOVER->pp_output_blocks[1];
    
    //calculating and setting up coeffs
    XOVER_SetCutoffFrequency(pXOVER, XOVER_INIT_FC_HZ);
    
    return true;
}

bool DAFXProcessCrossover(t_DAFXCrossover *pXOVER)
{
    for (int i = 0; i < pXOVER->num_cascades; i++) {
        ProcessBlockBiquad(pXOVER->pp_lp_biquads[i]);
        ProcessBlockBiquad(pXOVER->pp_hp_biquads[i]);
    }
    
    return true;
}

//in bypass, we output the unaltered signal to both sides
bool DAFXBypassCrossover(t_DAFXCrossover *pXOVER)
{
    memcpy(pXOVER->pp_output_blocks[0], pXOVER->p_input_block, sizeof(float) * pXOVER->block_size);
    memcpy(pXOVER->pp_output_blocks[1], pXOVER->p_input_block, sizeof(float) * pXOVER->block_size);
    return true;
}

void DeallocDAFXCrossover(t_DAFXCrossover *pXOVER)
{
    FREE(pXOVER->p_input_block);
    for (int i = 0; i < XOVER_INIT_NUMOF_CHANNELS; i++) {
        FREE(pXOVER->pp_output_blocks[i]);
    }
    FREE(pXOVER->pp_output_blocks);
    FREE(pXOVER->p_lp_butter_coeffs);
    FREE(pXOVER->p_hp_butter_coeffs);
    for (int i = 0; i < pXOVER->num_cascades; i++) {
        //DeallocBiquadFilter(pXOVER->pp_lp_biquads[i]);
        FREE(pXOVER->pp_lp_biquads[i]);
        //DeallocBiquadFilter(pXOVER->pp_hp_biquads[i]);
        FREE(pXOVER->pp_hp_biquads[i]);
    }
    FREE(pXOVER->pp_lp_biquads);
    FREE(pXOVER->pp_hp_biquads);
}
