//
//  DAFX_BiquadFilter.h
//  BiquadFilter~
//
//  Created by Sebastian Curdy on 13/09/16.
//
//

#ifndef DAFX_BiquadFilter_h
#define DAFX_BiquadFilter_h


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
    
    typedef struct{
        
        int buffer_len;
        
        float * pInBuff;
        float * pOutBuff;
        float * pInternalMemory;
        
        float * b;
        float * a;
        
    }t_DAFX_BiquadFilter;
    
    
    /*!
     * Init BiquadFilter struct and allocate memory
     *
     * @param pointer on a BiquadFilter structure
     * @return process status
     */
    bool InitBiquadFilter( t_DAFX_BiquadFilter *pBQF);
    
    /*!
     * Update and apply Biquad filter
     *
     * The flow structure is the transposed direct form II
     * Coefficient must be set as a SOS form :
     * [b0 b1 b2 a0 a1 a2]
     * the filter normalizes the coefficients with respect to a0 and
     * computes the following
     *
     *     b0+ b1*z^-1 + b2*z^-2
     * H(z)=------------------------
     *      1 + a1*z^-1 + a2*z^-2
     *
     * not an efficient implementation but allows for several ways of inputting
     * coefficients (6-array or 5-array are both supported)
     *
     * @param pointer on BiquadFilter structure
     * @return process status
     */
    bool ProcessBlockBiquad(t_DAFX_BiquadFilter *pBQF);
    
    /* same as above, but sample-based*/
    float ProcessSingleSampleBiquad(t_DAFX_BiquadFilter *pBQF, float x);
    
    /*!
     * Set biquad coefficients
     *
     * Coefficients are expected as an array of 6 floats, in the following format:
     * (b0, b1, b2, a0, a1, a2) - b0-b2: feedforward, a0-a2: feedback
     * The coefficients don't have to be normalized to a0, but also not forbidden
     * not an efficient implementation but allows for several ways of inputting
     * coefficients (6-array or 5-array are both supported)
     *
     * @param pointer on BiquadFilter structure
     * @param pointer on array of coefficients (b0, b1, b2, a0, a1, a2)
     * @return process status
     */
    bool SetBiquadFilterCoeffs(t_DAFX_BiquadFilter *pBQF, float *p_coeffs);
    
    /*!
     * Deallocates allocated memory for the object
     *
     * @param pointer on BiquadFilter structure
     * @return void
     */
    void DeallocBiquadFilter(t_DAFX_BiquadFilter *pBQF);
    
    
    
#ifdef __cplusplus
}
#endif


#endif /* DAFX_BiquadFilter_h */
