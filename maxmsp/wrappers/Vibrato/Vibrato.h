//
//  Vibrato.h
//  Vibrato~
//


#ifndef Vibrato_h
#define Vibrato_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <libkern/OSAtomic.h>
#endif

#include "DAFX_Vibrato.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum
{
    VIB_INLET_INPUT_SIGNAL = 0,
    VIB_INLET_RATE_BPM,
    VIB_INLET_DEPTH,
    VIB_INLET_BYPASS_VIB,
    VIB_INLET_DELAYLINE_BUFFER_SIZE,
    Vibrato_N_INLETS,
};

enum
{
    VIB_OUTLET_OUTPUT_SIGNAL = 0,
    VIB_OUTLET_DELAY_MS,
    Vibrato_N_OUTLETS,
};
    
    
    // struct to represent the object's state
    typedef struct _Vibrato {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXVibrato * pVIB;
        void * pf_VIB_perform;        
       
    } t_Vibrato;
    
    
    //function pointer type to the VIB or BypassVIB function
    typedef bool (* performFunction)(t_DAFXVibrato *pVIB);
    
    
    // method prototypes
    
    // Creates a new object in MaxMSP
    void *Vibrato_new(t_symbol *s, long argc, t_atom *argv);
    
    // Detaches an object from the DSP chain and frees its allocated memory
    void Vibrato_free(t_Vibrato *x);

    //runs if input is a float
    void Vibrato_float(t_Vibrato *x, double f);
    
    //runs if input is an int
    void Vibrato_int(t_Vibrato *x, long n);
    
    // Runs if mouse is hovered over an in/outlet
    void Vibrato_assist(t_Vibrato *x, void *b, long m, long a, char *s);    
    
    // assigns a functionality to the object upon start
    void Vibrato_dsp64(t_Vibrato *x,
                                  t_object  *dsp64,
                                  short     *count,
                                  double    samplerate,
                                  long      maxvectorsize,
                                  long      flags);
    
    // The functionality of the object
    void Vibrato_perform64(t_Vibrato *x,
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



#endif /* Vibrato_h */
