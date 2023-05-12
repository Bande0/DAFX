//
//  DAFX_LowFrequencyOscillator.c
//  LowFrequencyOscillator~
//

#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_InitLowFrequencyOscillator.h"
#include "DAFX_definitions.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <math.h>
#else
#include <Accelerate/Accelerate.h>
#endif

bool LFO_SetMode(t_DAFXLowFrequencyOscillator *pLFO, t_lfo_algo_select alg)
{
    switch(alg) {
        case LFO_ALGO_SELECT_SIN:
            pLFO->algo = LFO_ALGO_SELECT_SIN;
            pLFO->pf_process_func = &_GenerateSinusoidalLFO;
            break;
        case LFO_ALGO_SELECT_SAW:
            pLFO->algo = LFO_ALGO_SELECT_SAW;
            pLFO->pf_process_func = &_GenerateSawtoothLFO;
            break;
        default:
            break;
    }

    return true;
}

bool LFO_SetFrequency(t_DAFXLowFrequencyOscillator *pLFO, float f)
{
    pLFO->f = DAFX_MAX(f, 0.0);
    _RecalculatePrivateVariables(pLFO);
    
    return true;
}

bool LFO_SetAmplitude(t_DAFXLowFrequencyOscillator *pLFO, float a)
{
    pLFO->amp = DAFX_MAX(a, 0.0);
    _RecalculatePrivateVariables(pLFO);
    
    return true;
}

bool LFO_SetBalance(t_DAFXLowFrequencyOscillator *pLFO, float bal)
{
    pLFO->balance = DAFX_MAX(DAFX_MIN(bal, LFO_MAX_BALANCE), LFO_MIN_BALANCE);
    _RecalculatePrivateVariables(pLFO);
    
    return true;
}

bool LFO_SetOffset(t_DAFXLowFrequencyOscillator *pLFO, float off)
{
    pLFO->offset = off;
    return true;
}

bool LFO_SetClipHigh(t_DAFXLowFrequencyOscillator *pLFO, float clip_h)
{
    pLFO->clip_h = clip_h;
    return true;
}

bool LFO_SetClipLow(t_DAFXLowFrequencyOscillator *pLFO, float clip_l)
{
    pLFO->clip_l = clip_l;
    return true;
}


bool LFO_ReinitPhase(t_DAFXLowFrequencyOscillator *pLFO)
{
    if(pLFO->algo == LFO_ALGO_SELECT_SIN)
    {
        pLFO->u = 1.0;
        pLFO->v = 0.0;
    }
    else
    {
        pLFO->y = 0.0;
        pLFO->d = pLFO->d_rise;
        pLFO->d_state = LFO_STATE_RISING;
    }
    return true;
}

bool _RecalculatePrivateVariables(t_DAFXLowFrequencyOscillator *pLFO)
{
    //For now, this re-calculates both the sine and sawtooth parameters, regardless of current mode
    //this is to ensure that the frequency, amp, etc. stays the same if the mode is changed on the fly
    //sure there is a better way to implement this
    
    // --- sine specific parameters
    pLFO->k1 = tanf(0.5 * TWO_PI * pLFO->f / (float) pLFO->fs);
    pLFO->k2 = 2.0 * pLFO->k1 / (1.0 + pLFO->k1 * pLFO->k1);
    
    // --- sawtooth specific parameters
    pLFO->T = roundf((float)pLFO->fs / pLFO->f);  //duration of a full period in samples (rounded to the nearest sample)
    //duration of rise and fall periods
    pLFO->t1 = pLFO->T * pLFO->balance;
    pLFO->t2 = pLFO->T - pLFO->t1;
    //rise and fall differentials within one sample
    pLFO->d_rise = 2.0 * pLFO->amp / pLFO->t1;
    pLFO->d_fall = -2.0 * pLFO->amp / pLFO->t2;

    //update with new differential
    if (pLFO->d_state == LFO_STATE_RISING)
    {
        pLFO->d = pLFO->d_rise;
    }
    else
    {
        pLFO->d = pLFO->d_fall;
    }
    
    return true;
}

float _GenerateSinusoidalLFO(t_DAFXLowFrequencyOscillator *pLFO)
{
    //The recursive algorithm
    float vv = pLFO->u - pLFO->k1 * pLFO->v;
    pLFO->v = pLFO->v + pLFO->k2 * vv;  //sin(w)
    pLFO->u = vv - pLFO->k1 * pLFO->v;  //cos(w)
    
    //offset and clip output
    return (DAFX_MAX(DAFX_MIN(pLFO->amp * pLFO->v + pLFO->offset, pLFO->clip_h), pLFO->clip_l));
}

float _GenerateSawtoothLFO(t_DAFXLowFrequencyOscillator *pLFO)
{
    // recursive modified sawtooth waveform generation
    
    //Change direction if amplitude limit exceeded
    if (pLFO->y >= pLFO->amp)
    {
        pLFO->d = pLFO->d_fall;
        pLFO->d_state = LFO_STATE_FALLING;
    }
    else if (pLFO->y <= -1.0 * pLFO->amp)
    {
        pLFO->d = pLFO->d_rise;
        pLFO->d_state = LFO_STATE_RISING;
    }
    
    //update new sample with differential
    pLFO->y += pLFO->d;
    
    //offset and clip output
    return (DAFX_MAX(DAFX_MIN(pLFO->y + pLFO->offset, pLFO->clip_h), pLFO->clip_l));
}


bool InitDAFXLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO)
{
    // ---- general, wrapper ---- //
    //Signal vector size
    int block_size = pLFO->block_size;
    // I/O buffers
    pLFO->p_output_block = (float *) calloc(block_size, sizeof(float));
    
    //default mode
    pLFO->algo = LFO_ALGO_SELECT_SIN;
    //set sample generation function accordingly
    pLFO->pf_process_func = &_GenerateSinusoidalLFO;
    
    //common params
    pLFO->f = LFO_INIT_DEFAULT_FREQ_HZ;
    pLFO->amp = LFO_INIT_DEFAULT_AMP;
    pLFO->offset = LFO_INIT_DEFAULT_OFFSET;
    pLFO->clip_h = LFO_INIT_DEFAULT_CLIP_H;
    pLFO->clip_l = LFO_INIT_DEFAULT_CLIP_L;
    
    //sine specific params
    pLFO->u = 1.0;
    pLFO->v = 0.0;
    _RecalculatePrivateVariables(pLFO);
    
    //sawtooth specific params
    pLFO->balance = LFO_INIT_DEFAULT_BALANCE;
    pLFO->d_state = LFO_STATE_RISING; //init rise/fall state flag
    _RecalculatePrivateVariables(pLFO);
    pLFO->y = 0.0; // output sample
    
    return true;
}

bool DAFXLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO)
{
    int block_size = pLFO->block_size;
    float *pOutput = pLFO->p_output_block;
    tf_process_function pf_generate = (tf_process_function) pLFO->pf_process_func; //Sample generation function currently pointed at
    
    for (int i = 0; i < block_size; i++) {
        pOutput[i] = pf_generate(pLFO);
    }
    
    return true;
}

bool DAFXBypassLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO)
{
    memset(pLFO->p_output_block, 0, sizeof(float) * pLFO->block_size);
    return true;
}

void DeallocDAFXLowFrequencyOscillator(t_DAFXLowFrequencyOscillator *pLFO)
{
    FREE(pLFO->p_output_block);
}
