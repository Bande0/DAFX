
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

#include "Crybaby.h"
#include "DAFX_Crybaby.h"
#include "DAFX_InitCrybaby.h"
#include "DAFX_BiquadFilter.h"
#include "DAFX_InitBiquadFilter.h"

#include "DAFX_definitions.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <mach/mach_time.h>
#endif


// global class pointer variable
static t_class *Crybaby_class = NULL;


//***********************************************************************************************


// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("DAFXCrybaby~", (method)Crybaby_new, (method)Crybaby_free, (long)sizeof(t_Crybaby), 0L, A_GIMME, 0);
    
    //adding methods to the object for handling different actions
    class_addmethod(c, (method)Crybaby_dsp64,		"dsp64",	A_CANT, 0); //action if input is a signal
    class_addmethod(c, (method)Crybaby_assist,	"assist",	A_CANT, 0); //action if mouse is hovered over an in/outlet
    class_addmethod(c, (method)Crybaby_int,	"int",      A_LONG, 0); //action if input is an int
    class_addmethod(c, (method)Crybaby_float,	"float",	A_FLOAT,0); //action if input is a float
    class_addmethod(c, (method)Crybaby_bang, "bang", 0);
    
    class_dspinit(c); //this is always needed for MSP objects
    class_register(CLASS_BOX, c);
    Crybaby_class = c;
    
}

//New instance creation function
//argument list has to be declared like this, because the class was created with class_new(...,A_GIMME,0).
//Had it been A_DEFFLOAT, then there would be a single float argument instead
void *Crybaby_new(t_symbol *s, long argc, t_atom *argv)
{
    //instantiate an object x of class Crybaby_class
    t_Crybaby *x = (t_Crybaby *)object_alloc(Crybaby_class);
    
    if (x) {
        
        // Creating inlets
        // dsp_setup sets up these inlets as proxies!
        dsp_setup((t_pxobject *)x, Crybaby_N_INLETS);	// MSP inlets: arg is # of inlets and is REQUIRED! use 0 if you don't need inlets
        
        //Creating outlets - note: no need to store pointers to them in the struct, as in Max object       
        for (int i = 0; i < Crybaby_N_OUTLETS; i++) {
            outlet_new(x, "signal"); 		// signal outlet (note "signal" rather than NULL)
        }        
        
        // allocate data structure for dsp API
        x->pCB = (t_DAFXCrybaby *) malloc(sizeof(t_DAFXCrybaby));
        
        // Set the perform function pointer to Crybaby
        x->pf_CB_perform = &DAFXProcessCrybaby;
        
        // TODO: can we alter this from Max in runtime or do we need to rebuild?
        //Initialize the structure
        x->pCB->fs = FS_48k;
        x->pCB->block_size = DAFX_BLOCK_SIZE;        
        
        InitDAFXCrybaby(x->pCB);          
    }
    return (x);
}


// Detaches the object from the DSP chain and deallocates memory
void Crybaby_free(t_Crybaby *x)
{
    dsp_free((t_pxobject *)x);
    DeallocDAFXCrybaby(x->pCB);
}

//Action if mouse is hovered over the in/outlets
void Crybaby_assist(t_Crybaby *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        switch(a)
        {
            case CB_INLET_INPUT_SIGNAL:
                sprintf(s, "(signal) Input signal");
                break;
            case CB_INLET_PEDAL_POSITION:
                sprintf(s, "(float) pedal position");
                break;
            case CB_INLET_WAH_BALANCE:
                sprintf(s, "(float) Wah balance");
                break;
            case CB_INLET_LFO_MODE:
                sprintf(s, "(float) Auto-wah mode (0: Sin, 1: sawtooth)");
                break;
            case CB_INLET_LFO_RATE_BPM:
                sprintf(s, "(float) Auto-wah control rate (bpm)");
                break;
            case CB_INLET_LFO_AMP:
                sprintf(s, "(float) Auto-wah control amplitude");
                break;
            case CB_INLET_LFO_BALANCE:
                sprintf(s, "(float) Auto-wah control rise/fall balance (sawtooth only)");
                break;
            case CB_INLET_LFO_OFFSET:
                sprintf(s, "(float) Auto-wah control offset");
                break;
            case CB_INLET_LFO_CLIP_H:
                sprintf(s, "(float) Auto-wah control upper clipping limit (clip h)");
                break;
            case CB_INLET_LFO_CLIP_L:
                sprintf(s, "(float) Auto-wah control upper clipping limit (clip h)");
                break;
            case CB_INLET_REINIT_LFO_PHASE:
                sprintf(s, "(bang) Trigger reinit of LFO phasebb");
                break;
            case CB_INLET_BYPASS_CB:
                sprintf(s, "(int) Bypass / Manual / Auto");
                break;
            
            default:
                sprintf(s, "Invalid inlet!");
                break;
        }


    }
    else {	// outlet
        switch(a)
        {
            case CB_OUTLET_OUTPUT_SIGNAL:
                sprintf(s, "(signal) Output signal");
                break;
            case CB_OUTLET_FILTER_COEFFS:
                sprintf(s, "(signal) biquad filter coefficients");
                break;
            case CB_OUTLET_LFO_SIGNAL:
                sprintf(s, "(signal) Auto-wah control signal (LFO)");
                break;
            default:
                sprintf(s, "Invalid outlet!");
                break;
        }

    }
}


//Action if input was a float
void Crybaby_float(t_Crybaby *x, double f)
{
    //Get inlet number where the data came from
    long in = proxy_getinlet((t_object *)x);
    
    switch(in)
    {
        //Pedal Position
        case CB_INLET_PEDAL_POSITION:
            UpdatePedalPos(x->pCB, f);
            break;
            
        //Wah balance
        case CB_INLET_WAH_BALANCE:
            x->pCB->wah_balance =  DAFX_MAX(DAFX_MIN(f, 1.0), 0.0);
            break;
            
        //LFO Mode
        case CB_INLET_LFO_MODE:
            Crybaby_SetLFOMode(x->pCB, (t_lfo_algo_select) f);
            break;
            
        //LFO Rate
        case CB_INLET_LFO_RATE_BPM:
            Crybaby_SetLFORate(x->pCB, (int)f);
            break;
            
        //LFO Amp
        case CB_INLET_LFO_AMP:
            Crybaby_SetLFOAmplitude(x->pCB, f);
            break;
            
        //LFO Balance
        case CB_INLET_LFO_BALANCE:
            Crybaby_SetLFOBalance(x->pCB, f);
            break;
            
        //LFO Offset
        case CB_INLET_LFO_OFFSET:
            Crybaby_SetLFOOffset(x->pCB, f);
            break;
            
        //LFO Clip H
        case CB_INLET_LFO_CLIP_H:
            Crybaby_SetLFOClipHigh(x->pCB, f);
            break;
            
        //LFO Clip L
        case CB_INLET_LFO_CLIP_L:
            Crybaby_SetLFOClipLow(x->pCB, f);
            break;
            
        //Set the CB_perform to Bypass / process
        case CB_INLET_BYPASS_CB:
            {
                switch((t_cb_algo_select)f) {
                    case CB_BYPASS_CRYBABY:
                        x->pf_CB_perform = &DAFXBypassCrybaby;
                        break;
                    case CB_MANUAL_CRYBABY:
                        x->pf_CB_perform = &DAFXProcessCrybaby;
                        break;
                    case CB_AUTO_CRYBABY:
                        x->pf_CB_perform = &DAFXProcessAutoCrybaby;
                        break;
                    default:
                        break;
                }
            }            
            break;
            
        default:
            break;
    }
}

//Action if input was an int
void Crybaby_int(t_Crybaby *x, long n)
{
    Crybaby_float(x, (double)n);
}

//Action if input was a bang
void Crybaby_bang(t_Crybaby *x)
{
    switch (proxy_getinlet((t_object *)x)) {
        //Reinit LFO Phase
        case CB_INLET_REINIT_LFO_PHASE:
            Crybaby_ReinitLFOPhase(x->pCB);
            break;
        default:
            break;
    }
}


// registers a function for the signal chain in Max
// This function is called if the input is a signal.
// It is possible to assign a different perform function with object_method() based on some condition
void Crybaby_dsp64(t_Crybaby *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    
    //This call adds the DSP operation of this MSP object to the signal chain
    //It is also possible to implement several perform functions, and assigning a different one to the DSP chain
    //chain based on some conditon
    object_method(dsp64, gensym("dsp_add64"), x, Crybaby_perform64, 0, NULL);
}


// this is the 64-bit perform method for audio vectors - note: it is possible to implement several perform functions and assign a different one with object_method() based on some condition
void Crybaby_perform64(t_Crybaby *x,
                      t_object *dsp64,
                      double **ins,
                      long numins,
                      double **outs,
                      long numouts,
                      long sampleframes,
                      long flags,
                      void *userparam)
{
    
    t_DAFXCrybaby * pCB = x->pCB;
    
    t_double *InSignal = ins[CB_INLET_INPUT_SIGNAL];		// we get audio for each inlet of the object from the **ins argument
    
    t_double *OutSignal = outs[CB_OUTLET_OUTPUT_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutFilterCoeffs = outs[CB_OUTLET_FILTER_COEFFS];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutLFO = outs[CB_OUTLET_LFO_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    
    //Converting the incoming signal from double to float complex in a temporary array
    for (int i = 0; i < sampleframes; i++) {
        pCB->p_input_block[i] = (float)InSignal[i];
    }
    
    // Call the Crybaby perform function
    performFunction CB_perform = (performFunction) x->pf_CB_perform;
    CB_perform(pCB);  
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutSignal[i] = (double) pCB->p_output_block[i];
        OutLFO[i] = (double) pCB->pLFO->p_output_block[i];
    }
    
    for(int i = 0; i < (BIQUAD_DENOMINATOR_SIZE + BIQUAD_NUMERATOR_SIZE); i++){
        OutFilterCoeffs[i] = (double) pCB->p_biquad_coeffs[i];
    }
    
}