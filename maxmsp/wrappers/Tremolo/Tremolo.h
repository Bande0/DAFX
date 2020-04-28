//
//  Tremolo.h
//  Tremolo~
//


#ifndef Tremolo_h
#define Tremolo_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

#include "DAFX_Tremolo.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum
{
    TREM_INLET_INPUT_SIGNAL = 0,
    TREM_INLET_RATE_BPM,
    TREM_INLET_DEPTH_PERCENT,
    TREM_INLET_SHARPNESS,
    TREM_INLET_BYPASS_TREM,
    Tremolo_N_INLETS,
};

enum
{
    TREM_OUTLET_OUTPUT_SIGNAL = 0,
    TREM_OUTLET_LFO_SIGNAL,
    TREM_OUTLET_LFO_OFFSET,
    Tremolo_N_OUTLETS,
};
    
    
    // struct to represent the object's state
    typedef struct _Tremolo {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXTremolo * pTREM;
        void * pf_TREM_perform;        
       
    } t_Tremolo;
    
    
    //function pointer type to the TREM or BypassTREM function
    typedef bool (* performFunction)(t_DAFXTremolo *pTREM);
    
    
    // method prototypes
    
    // Creates a new object in MaxMSP
    void *Tremolo_new(t_symbol *s, long argc, t_atom *argv);
    
    // Detaches an object from the DSP chain and frees its allocated memory
    void Tremolo_free(t_Tremolo *x);

    //runs if input is a float
    void Tremolo_float(t_Tremolo *x, double f);
    
    //runs if input is an int
    void Tremolo_int(t_Tremolo *x, long n);
    
    // Runs if mouse is hovered over an in/outlet
    void Tremolo_assist(t_Tremolo *x, void *b, long m, long a, char *s);    
    
    // assigns a functionality to the object upon start
    void Tremolo_dsp64(t_Tremolo *x,
                                  t_object  *dsp64,
                                  short     *count,
                                  double    samplerate,
                                  long      maxvectorsize,
                                  long      flags);
    
    // The functionality of the object
    void Tremolo_perform64(t_Tremolo *x,
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



#endif /* Tremolo_h */
