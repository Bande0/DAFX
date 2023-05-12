//
//  Crossover.h
//  Crossover~
//


#ifndef Crossover_h
#define Crossover_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <libkern/OSAtomic.h>
#endif

#include "DAFX_Crossover.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum
{
    XOVER_INLET_INPUT_SIGNAL = 0,
    XOVER_INLET_CUTOFF_FREQUENCY,
    XOVER_INLET_CASCADE_ORDER,
    XOVER_INLET_BYPASS_XOVER,
    Crossover_N_INLETS,
};

enum
{
    XOVER_OUTLET_OUTPUT_LP_SIGNAL = 0,
    XOVER_OUTLET_OUTPUT_HP_SIGNAL,
    Crossover_N_OUTLETS,
};
    
    
    
    // struct to represent the object's state
    typedef struct _Crossover {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXCrossover * pXOVER;
        void * pf_XOVER_perform;        
       
    } t_Crossover;
    
    
    //function pointer type to the XOVER or BypassXOVER function
    typedef bool (* performFunction)(t_DAFXCrossover *pXOVER);
    
    
    // method prototypes
    
    // Creates a new object in MaxMSP
    void *Crossover_new(t_symbol *s, long argc, t_atom *argv);
    
    // Detaches an object from the DSP chain and frees its allocated memory
    void Crossover_free(t_Crossover *x);
    
    //runs if input is a float
    void Crossover_float(t_Crossover *x, double f);
    
    //runs if input is an int
    void Crossover_int(t_Crossover *x, long n);
    
    //runs if input is a bang
    void Crossover_bang(t_Crossover *x);
    
    // Runs if mouse is hovered over an in/outlet
    void Crossover_assist(t_Crossover *x, void *b, long m, long a, char *s);    
    
    // assigns a functionality to the object upon start
    void Crossover_dsp64(t_Crossover *x,
                                  t_object  *dsp64,
                                  short     *count,
                                  double    samplerate,
                                  long      maxvectorsize,
                                  long      flags);
    
    // The functionality of the object
    void Crossover_perform64(t_Crossover *x,
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



#endif /* Crossover_h */
