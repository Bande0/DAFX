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
    OD_INLET_COMP_THRESHOLD_DB,
    OD_INLET_COMP_RATIO,
    OD_INLET_EXP_THRESHOLD_DB,
    OD_INLET_EXP_RATIO,
    OD_INLET_KNEE_WIDTH_DB,
    OD_INLET_MAKEUP_GAIN_DB,
    OD_INLET_COMP_ATTACK_TIME_MS,
    OD_INLET_COMP_RELEASE_TIME_MS,
    OD_INLET_EXP_ATTACK_TIME_MS,
    OD_INLET_EXP_RELEASE_TIME_MS,
    OD_INLET_BYPASS_OD,
    OD_INLET_XVAD_OR_VAD_SWITCH,
    OD_INLET_VAD_MEASURE,
    OD_INLET_VAD_THRESHOLD,
    Overdrive_N_INLETS,
};

enum
{
    OD_OUTLET_OUTPUT_SIGNAL = 0,
    OD_OUTLET_GAIN_CURVE,    
    OD_OUTLET_APPLIED_GAIN_DB,
    OD_OUTLET_RMS_POWER_LINEAR,
    OD_OUTLET_RMS_POWER_SMOOTH,
    OD_OUTLET_ELAPSED_TIME_MS,
    OD_OUTLET_VAD_LABEL,
    OD_OUTLET_CURRENT_MARKER_ON_GAIN_CURVE,
    Overdrive_N_OUTLETS,
};
    
enum{
    OD_BYPASS = 0,
    OD_IDEAL_GAIN_FUNCTION,
    OD_OPTIMIZED_GAIN_FUNCTION
};
    
#define OUT_OF_RANGE_NUMBER_ON_PLOT 10.0
    
    // struct to represent the object's state
    typedef struct _Overdrive {
        t_pxobject		ob;			// the object itself (t_pxobject in MSP instead of t_object)
        
        t_DAFXOverdrive * pOD;
        void * pf_OD_perform;        
        
        double elapsed_NS_smooth;
        double *p_current_in_out_gain_marker;
       
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
