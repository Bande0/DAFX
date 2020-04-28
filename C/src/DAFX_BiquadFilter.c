//
//  DAFX_BiquadFilter.c
//  BiquadFilter~
//
//  Created by Sebastian Curdy on 13/09/16.
//
//

#include "DAFX_BiquadFilter.h"
#include "DAFX_InitBiquadFilter.h"

#include "DAFX_definitions.h"

#include <Accelerate/Accelerate.h>


bool InitBiquadFilter(t_DAFX_BiquadFilter *pBQF)
{
    //Allocating I/O buffers
    pBQF->pInBuff = (float *) calloc(sizeof(float), pBQF->buffer_len);
    pBQF->pOutBuff = (float *) calloc(sizeof(float), pBQF->buffer_len);
    
    //Allocating internal memory
    pBQF->pInternalMemory = (float *) calloc(sizeof(float), BIQUAD_FILTER_ORDER);
    
    //Allocating space for filter coeffs
    pBQF->b = (float *) calloc(sizeof(float), BIQUAD_NUMERATOR_SIZE);
    pBQF->a = (float *) calloc(sizeof(float), BIQUAD_DENOMINATOR_SIZE);
    
    //Initializing to a pass-through filter
    pBQF->b[0] = 1.0;
    pBQF->a[0] = 1.0; //not necessary to set
    
    return true;
}

bool SetBiquadFilterCoeffs(t_DAFX_BiquadFilter *pBQF, float *p_coeffs)
{
    //expected coeff order: b0, b1, b2, a0, a1, a2
    float a0 = p_coeffs[3];
    float ax = 1.0 / a0;
    
    pBQF->b[0] = p_coeffs[0] * ax;
    pBQF->b[1] = p_coeffs[1] * ax;
    pBQF->b[2] = p_coeffs[2] * ax;
    
    pBQF->a[0] = 1.0;
    pBQF->a[1] = p_coeffs[4] * ax;
    pBQF->a[2] = p_coeffs[5] * ax;
    
    return true;
}


bool ProcessBlockBiquad(t_DAFX_BiquadFilter *pBQF)
{
    float * p_in = pBQF->pInBuff;
    float * p_out = pBQF->pOutBuff;
    float * b = pBQF->b;
    float * a = pBQF->a;
    float * w = pBQF->pInternalMemory;
    
    int buffer_len = pBQF->buffer_len;
    
    for (int i=0; i < buffer_len; i++)// loop through all samples in inBuff
    {        
        p_out[i] = ProcessSingleSampleBiquad(pBQF, p_in[i]);
    }
    
    return true;
}

float ProcessSingleSampleBiquad(t_DAFX_BiquadFilter *pBQF, float x)
{
    float * b = pBQF->b;
    float * a = pBQF->a;
    float * w = pBQF->pInternalMemory;
    
    float y;
    //@matlab: y(n)= b0*x(n) + w1(n-1);
    y = b[0] * x + w[0];
    //@matlab: w1(n)=b1*x(n) + a1*y(n) + w2(n)
    w[0] = b[1] * x - a[1] * y + w[1];
    //@matlab: w2(n)= b2*x(n) - a2*y(n)
    w[1]= b[2] * x - a[2] * y;
    
    return y;
}

void DeallocBiquadFilter(t_DAFX_BiquadFilter *pBQF)
{
    if(pBQF != NULL) {
        
           FREE(pBQF->pInBuff);
           FREE(pBQF->pOutBuff);
           FREE(pBQF->pInternalMemory);
           FREE(pBQF->b);
           FREE(pBQF->a);
    }
    
}



