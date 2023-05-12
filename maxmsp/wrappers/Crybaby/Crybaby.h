//
//  Crybaby.h
//  Crybaby~
//


#ifndef Crybaby_h
#define Crybaby_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <libkern/OSAtomic.h>
#endif

#include "DAFX_Crybaby.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum
{
    CB_INLET_INPUT_SIGNAL = 0,
    CB_INLET_PEDAL_POSITION,
    CB_INLET_WAH_BALANCE,
    CB_INLET_LFO_MODE,
    CB_INLET_LFO_RATE_BPM,
    CB_INLET_LFO_AMP,
    CB_INLET_LFO_BALANCE,
    CB_INLET_LFO_OFFSET,
    CB_INLET_LFO_CLIP_H,
    CB_INLET_LFO_CLIP_L,
    CB_INLET_REINIT_LFO_PHASE,
    CB_INLET_BYPASS_CB,
    Crybaby_N_INLETS,
};

enum
{
    CB_OUTLET_OUTPUT_SIGNAL = 0,
    CB_OUTLET_FILTER_COEFFS,
    CB_OUTLET_LFO_SIGNAL,
    Crybaby_N_OUTLETS,
};
    
    
    
    // struct to represent the object's state
    typedef struct _Crybaby {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXCrybaby * pCB;
        void * pf_CB_perform;        
       
    } t_Crybaby;
    
    
    //function pointer type to the CB or BypassCB function
    typedef bool (* performFunction)(t_DAFXCrybaby *pCB);
    
    
    // method prototypes
    
    // Creates a new object in MaxMSP
    void *Crybaby_new(t_symbol *s, long argc, t_atom *argv);
    
    // Detaches an object from the DSP chain and frees its allocated memory
    void Crybaby_free(t_Crybaby *x);
    
    //runs if input is a float
    void Crybaby_float(t_Crybaby *x, double f);
    
    //runs if input is an int
    void Crybaby_int(t_Crybaby *x, long n);
    
    //runs if input is a bang
    void Crybaby_bang(t_Crybaby *x);
    
    // Runs if mouse is hovered over an in/outlet
    void Crybaby_assist(t_Crybaby *x, void *b, long m, long a, char *s);    
    
    // assigns a functionality to the object upon start
    void Crybaby_dsp64(t_Crybaby *x,
                                  t_object  *dsp64,
                                  short     *count,
                                  double    samplerate,
                                  long      maxvectorsize,
                                  long      flags);
    
    // The functionality of the object
    void Crybaby_perform64(t_Crybaby *x,
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



#endif /* Crybaby_h */
