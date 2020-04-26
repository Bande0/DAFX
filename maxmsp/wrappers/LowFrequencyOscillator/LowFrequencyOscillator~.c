
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

#include "LowFrequencyOscillator.h"
#include "DAFX_LowFrequencyOscillator.h"
#include "DAFX_InitLowFrequencyOscillator.h"

#include "DAFX_definitions.h"

#include <mach/mach_time.h>


// global class pointer variable
static t_class *LowFrequencyOscillator_class = NULL;


//***********************************************************************************************


// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("DAFXLowFrequencyOscillator~", (method)LowFrequencyOscillator_new, (method)LowFrequencyOscillator_free, (long)sizeof(t_LowFrequencyOscillator), 0L, A_GIMME, 0);
    
    //adding methods to the object for handling different actions
    class_addmethod(c, (method)LowFrequencyOscillator_dsp64,		"dsp64",	A_CANT, 0); //action if input is a signal
    class_addmethod(c, (method)LowFrequencyOscillator_assist,	"assist",	A_CANT, 0); //action if mouse is hovered over an in/outlet
    class_addmethod(c, (method)LowFrequencyOscillator_int,	"int",      A_LONG, 0); //action if input is an int
    class_addmethod(c, (method)LowFrequencyOscillator_float,	"float",	A_FLOAT,0); //action if input is a float
    class_addmethod(c, (method)LowFrequencyOscillator_bang, "bang", 0);
    
    class_dspinit(c); //this is always needed for MSP objects
    class_register(CLASS_BOX, c);
    LowFrequencyOscillator_class = c;
    
}

//New instance creation function
//argument list has to be declared like this, because the class was created with class_new(...,A_GIMME,0).
//Had it been A_DEFFLOAT, then there would be a single float argument instead
void *LowFrequencyOscillator_new(t_symbol *s, long argc, t_atom *argv)
{
    //instantiate an object x of class LowFrequencyOscillator_class
    t_LowFrequencyOscillator *x = (t_LowFrequencyOscillator *)object_alloc(LowFrequencyOscillator_class);
    
    if (x) {
        
        // Creating inlets
        // dsp_setup sets up these inlets as proxies!
        dsp_setup((t_pxobject *)x, LowFrequencyOscillator_N_INLETS);	// MSP inlets: arg is # of inlets and is REQUIRED! use 0 if you don't need inlets
        
        //Creating outlets - note: no need to store pointers to them in the struct, as in Max object       
        for (int i = 0; i < LowFrequencyOscillator_N_OUTLETS; i++) {
            outlet_new(x, "signal"); 		// signal outlet (note "signal" rather than NULL)
        }        
        
        // allocate data structure for dsp API
        x->pLFO = (t_DAFXLowFrequencyOscillator *) malloc(sizeof(t_DAFXLowFrequencyOscillator));
        
        // Set the perform function pointer to Sample Based Compressor (and not bypass)
        x->pf_LFO_perform = &DAFXLowFrequencyOscillator;   
        
        // TODO: can we alter this from Max in runtime or do we need to rebuild?
        //Initialize the structure
        x->pLFO->fs = FS_48k;
        x->pLFO->block_size = DAFX_BLOCK_SIZE;
        
        InitDAFXLowFrequencyOscillator(x->pLFO);          
    }
    return (x);
}


// Detaches the object from the DSP chain and deallocates memory
void LowFrequencyOscillator_free(t_LowFrequencyOscillator *x)
{
    dsp_free((t_pxobject *)x);
    DeallocDAFXLowFrequencyOscillator(x->pLFO);
}

//Action if mouse is hovered over the in/outlets
void LowFrequencyOscillator_assist(t_LowFrequencyOscillator *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        switch(a)
        {
            case LFO_INLET_ALGO_SELECT:
                sprintf(s, "(int) algo selector. 0 - sinusoidal, 1 - sawtooth");
                break;
            case LFO_INLET_F:
                sprintf(s, "(float) oscillator frequency (Hz)");
                break;
            case LFO_INLET_AMP:
                sprintf(s, "(float) oscillator amplitude");
                break;
            case LFO_INLET_OFFSET:
                sprintf(s, "(float) waveform offset");
                break;
            case LFO_INLET_CLIP_H:
                sprintf(s, "(float) Upper amplitude limit ('clip high')");
                break;
            case LFO_INLET_CLIP_L:
                sprintf(s, "(float) Lower amplitude limit ('clip low')");
                break;
            case LFO_INLET_BALANCE:
                sprintf(s, "(float) (Sawtooth wave only) - Waveform shape balance");
                break;
            case LFO_INLET_REINIT_PHASE:
                sprintf(s, "(bang) Trigger oscillator phase restart");
                break;
            case LFO_INLET_BYPASS_LFO:
                sprintf(s, "(int) Bypass / Enable LFO");
                break;
            
            default:
                sprintf(s, "Invalid inlet!");
                break;
        }


    }
    else {	// outlet
        switch(a)
        {
            case LFO_OUTLET_OUTPUT_SIGNAL:
                sprintf(s, "(signal) Output signal");
                break;
            default:
                sprintf(s, "Invalid outlet!");
                break;
        }

    }
}


//Action if input was a bang
void LowFrequencyOscillator_bang(t_LowFrequencyOscillator *x)
{
    switch (proxy_getinlet((t_object *)x)) {
            
        //Trigger phase reinit
        case LFO_INLET_REINIT_PHASE:
            _ReinitPhase(x->pLFO);
            break;
        default:
            break;
    }
}

//Action if input was a float
void LowFrequencyOscillator_float(t_LowFrequencyOscillator *x, double f)
{
    
    //Get inlet number where the data came from
    long in = proxy_getinlet((t_object *)x);
    
    switch(in)
    {
        //LowFrequencyOscillator algo selector
        case LFO_INLET_ALGO_SELECT:
            {
                switch((int)f) {
                    case LFO_ALGO_SELECT_SIN:
                        x->pLFO->algo = LFO_ALGO_SELECT_SIN;
                        x->pLFO->pf_process_func = &_GenerateSinusoidalLFO;
                        break;
                    case LFO_ALGO_SELECT_SAW:
                        x->pLFO->algo = LFO_ALGO_SELECT_SAW;
                        x->pLFO->pf_process_func = &_GenerateSawtoothLFO;
                        break;
                    default:
                        break;
                }
            }
            break;
            
        //Frequency
        case LFO_INLET_F:
            x->pLFO->f = DAFX_MAX(f, 0.0);
            _RecalculatePrivateVariables(x->pLFO);
            break;
        
        //Amplitude
        case LFO_INLET_AMP:
            x->pLFO->f = DAFX_MAX(f, 0.0);
            _RecalculatePrivateVariables(x->pLFO);
            break;
            
        //Balance
        case LFO_INLET_BALANCE:
            x->pLFO->balance = DAFX_MAX(DAFX_MIN(f, LFO_MAX_BALANCE), LFO_MIN_BALANCE);
            _RecalculatePrivateVariables(x->pLFO);
            break;
            
        //Offset
        case LFO_INLET_OFFSET:
            x->pLFO->offset = f;
            break;
            
        //Clip H
        case LFO_INLET_CLIP_H:
            x->pLFO->clip_h = f;
            break;
            
        //Clip L
        case LFO_INLET_CLIP_L:
            x->pLFO->clip_l = f;
            break;
            
        //Set the LFO_perform to Bypass / process
        case LFO_INLET_BYPASS_LFO:
            {
                if ((bool)f)
                {
                    x->pf_LFO_perform = &DAFXBypassLowFrequencyOscillator;
                }
                else
                {
                    x->pf_LFO_perform = &DAFXLowFrequencyOscillator;
                }
            }
            break;
            
        default:
            break;
    }
}

//Action if input was an int
void LowFrequencyOscillator_int(t_LowFrequencyOscillator *x, long n)
{
    LowFrequencyOscillator_float(x, (double)n);
}


// registers a function for the signal chain in Max
// This function is called if the input is a signal.
// It is possible to assign a different perform function with object_method() based on some condition
void LowFrequencyOscillator_dsp64(t_LowFrequencyOscillator *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    
    //This call adds the DSP operation of this MSP object to the signal chain
    //It is also possible to implement several perform functions, and assigning a different one to the DSP chain
    //chain based on some conditon
    object_method(dsp64, gensym("dsp_add64"), x, LowFrequencyOscillator_perform64, 0, NULL);
}


// this is the 64-bit perform method for audio vectors - note: it is possible to implement several perform functions and assign a different one with object_method() based on some condition
void LowFrequencyOscillator_perform64(t_LowFrequencyOscillator *x,
                      t_object *dsp64,
                      double **ins,
                      long numins,
                      double **outs,
                      long numouts,
                      long sampleframes,
                      long flags,
                      void *userparam)
{
    
    t_DAFXLowFrequencyOscillator * pLFO = x->pLFO;
    
    t_double *OutSignal = outs[LFO_OUTLET_OUTPUT_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    
    // Call the LowFrequencyOscillator perform function
    performFunction LFO_perform = (performFunction) x->pf_LFO_perform;
    LFO_perform(pLFO);  
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutSignal[i] = (double) pLFO->p_output_block[i];
    }
    
}