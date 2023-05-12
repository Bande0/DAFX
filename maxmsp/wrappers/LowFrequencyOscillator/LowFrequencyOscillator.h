//
//  LowFrequencyOscillator.h
//  LowFrequencyOscillator~
//


#ifndef LowFrequencyOscillator_h
#define LowFrequencyOscillator_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <libkern/OSAtomic.h>
#endif

#include "DAFX_LowFrequencyOscillator.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum
{
    LFO_INLET_ALGO_SELECT = 0,
    LFO_INLET_F,
    LFO_INLET_AMP,
    LFO_INLET_OFFSET,
    LFO_INLET_CLIP_H,
    LFO_INLET_CLIP_L,
    LFO_INLET_BALANCE,
    LFO_INLET_REINIT_PHASE,
    LFO_INLET_BYPASS_LFO,
    LowFrequencyOscillator_N_INLETS,
};

enum
{
    LFO_OUTLET_OUTPUT_SIGNAL = 0,
    LowFrequencyOscillator_N_OUTLETS,
};
    
    
    // struct to represent the object's state
    typedef struct _LowFrequencyOscillator {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXLowFrequencyOscillator * pLFO;
        void * pf_LFO_perform;        
       
    } t_LowFrequencyOscillator;
    
    
    //function pointer type to the LFO or BypassLFO function
    typedef bool (* performFunction)(t_DAFXLowFrequencyOscillator *pLFO);
    
    
    // method prototypes
    
    // Creates a new object in MaxMSP
    void *LowFrequencyOscillator_new(t_symbol *s, long argc, t_atom *argv);
    
    // Detaches an object from the DSP chain and frees its allocated memory
    void LowFrequencyOscillator_free(t_LowFrequencyOscillator *x);
    
    //runs if input is a bang
    void LowFrequencyOscillator_bang(t_LowFrequencyOscillator *x);
    
    //runs if input is a float
    void LowFrequencyOscillator_float(t_LowFrequencyOscillator *x, double f);
    
    //runs if input is an int
    void LowFrequencyOscillator_int(t_LowFrequencyOscillator *x, long n);
    
    // Runs if mouse is hovered over an in/outlet
    void LowFrequencyOscillator_assist(t_LowFrequencyOscillator *x, void *b, long m, long a, char *s);    
    
    // assigns a functionality to the object upon start
    void LowFrequencyOscillator_dsp64(t_LowFrequencyOscillator *x,
                                  t_object  *dsp64,
                                  short     *count,
                                  double    samplerate,
                                  long      maxvectorsize,
                                  long      flags);
    
    // The functionality of the object
    void LowFrequencyOscillator_perform64(t_LowFrequencyOscillator *x,
                                      t_object  *dsp64,
                                      double    **ins,
                                      long      numins,
                                      double    **outs,
                                      long      numouts,
                                      long      sampleframes,
                                      long      flags,
                                      void      *userparam);
    
    
    
#ifdef __cplusplus
}
#endif



#endif /* LowFrequencyOscillator_h */
