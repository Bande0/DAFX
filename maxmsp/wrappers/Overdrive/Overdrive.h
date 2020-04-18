//
//  Overdrive.h
//  Overdrive~
//


#ifndef Overdrive_h
#define Overdrive_h

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libkern/OSAtomic.h>

#include "DAFX_Overdrive.h"

#ifdef __cplusplus
extern "C" {
#endif
    
enum
{
    OD_INLET_INPUT_SIGNAL = 0,
    OD_INLET_ALGO_SELECT,
    OD_INLET_IN_GAIN,
    OD_INLET_OUT_GAIN,
    OD_INLET_PARAM,
    OD_INLET_THRESH,
    OD_INLET_BYPASS_OD,
    Overdrive_N_INLETS,
};

enum
{
    OD_OUTLET_OUTPUT_SIGNAL = 0,
    OD_OUTLET_GAIN_CURVE,
    Overdrive_N_OUTLETS,
};
    
    
    
    // struct to represent the object's state
    typedef struct _Overdrive {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXOverdrive * pOD;
        void * pf_OD_perform;        
       
    } t_Overdrive;
    
    
    //function pointer type to the OD or BypassOD function
    typedef bool (* performFunction)(t_DAFXOverdrive *pOD);
    
    
    // method prototypes
    
    // Creates a new object in MaxMSP
    void *Overdrive_new(t_symbol *s, long argc, t_atom *argv);
    
    // Detaches an object from the DSP chain and frees its allocated memory
    void Overdrive_free(t_Overdrive *x);
    
    //runs if input is a float
    void Overdrive_float(t_Overdrive *x, double f);
    
    //runs if input is an int
    void Overdrive_int(t_Overdrive *x, long n);
    
    // Runs if mouse is hovered over an in/outlet
    void Overdrive_assist(t_Overdrive *x, void *b, long m, long a, char *s);    
    
    // assigns a functionality to the object upon start
    void Overdrive_dsp64(t_Overdrive *x,
                                  t_object  *dsp64,
                                  short     *count,
                                  double    samplerate,
                                  long      maxvectorsize,
                                  long      flags);
    
    // The functionality of the object
    void Overdrive_perform64(t_Overdrive *x,
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



#endif /* Overdrive_h */
