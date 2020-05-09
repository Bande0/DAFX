
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

#include "Vibrato.h"
#include "DAFX_Vibrato.h"
#include "DAFX_InitVibrato.h"
#include "DAFX_IntegerSampleDelayLine.h"

#include "DAFX_definitions.h"

#include <mach/mach_time.h>


// global class pointer variable
static t_class *Vibrato_class = NULL;


//***********************************************************************************************


// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("DAFXVibrato~", (method)Vibrato_new, (method)Vibrato_free, (long)sizeof(t_Vibrato), 0L, A_GIMME, 0);
    
    //adding methods to the object for handling different actions
    class_addmethod(c, (method)Vibrato_dsp64,		"dsp64",	A_CANT, 0); //action if input is a signal
    class_addmethod(c, (method)Vibrato_assist,	"assist",	A_CANT, 0); //action if mouse is hovered over an in/outlet
    class_addmethod(c, (method)Vibrato_int,	"int",      A_LONG, 0); //action if input is an int
    class_addmethod(c, (method)Vibrato_float,	"float",	A_FLOAT,0); //action if input is a float
    
    class_dspinit(c); //this is always needed for MSP objects
    class_register(CLASS_BOX, c);
    Vibrato_class = c;
    
}

//New instance creation function
//argument list has to be declared like this, because the class was created with class_new(...,A_GIMME,0).
//Had it been A_DEFFLOAT, then there would be a single float argument instead
void *Vibrato_new(t_symbol *s, long argc, t_atom *argv)
{
    //instantiate an object x of class Vibrato_class
    t_Vibrato *x = (t_Vibrato *)object_alloc(Vibrato_class);
    
    if (x) {
        
        // Creating inlets
        // dsp_setup sets up these inlets as proxies!
        dsp_setup((t_pxobject *)x, Vibrato_N_INLETS);	// MSP inlets: arg is # of inlets and is REQUIRED! use 0 if you don't need inlets
        
        //Creating outlets - note: no need to store pointers to them in the struct, as in Max object       
        for (int i = 0; i < Vibrato_N_OUTLETS; i++) {
            outlet_new(x, "signal"); 		// signal outlet (note "signal" rather than NULL)
        }        
        
        // allocate data structure for dsp API
        x->pVIB = (t_DAFXVibrato *) malloc(sizeof(t_DAFXVibrato));
        
        // Set the perform function pointer to Sample Based Compressor (and not bypass)
        x->pf_VIB_perform = &DAFXVibrato;   
        
        // TODO: can we alter this from Max in runtime or do we need to rebuild?
        //Initialize the structure
        x->pVIB->fs = FS_48k;
        x->pVIB->block_size = DAFX_BLOCK_SIZE;
        InitDAFXVibrato(x->pVIB);          
    }
    return (x);
}


// Detaches the object from the DSP chain and deallocates memory
void Vibrato_free(t_Vibrato *x)
{
    dsp_free((t_pxobject *)x);
    DeallocDAFXVibrato(x->pVIB);
}

//Action if mouse is hovered over the in/outlets
void Vibrato_assist(t_Vibrato *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        switch(a)
        {
            case VIB_INLET_INPUT_SIGNAL:
                sprintf(s, "(signal) Input signal");
                break;
            case VIB_INLET_RATE_BPM:
                sprintf(s, "(int) Vibrato Rate (BPM)");
                break;
            case VIB_INLET_DEPTH:
                sprintf(s, "(int) Vibrato depth");
                break;
            case VIB_INLET_DELAY_MS_DIRECT_CONTROL:
                sprintf(s, "(float) Delay ms direct control (debug)");
                break;
            case VIB_INLET_DELAYLINE_BUFFER_SIZE:
                sprintf(s, "(float) Delayline buffersize set (millisec) (debug)");
                break;
            case VIB_INLET_BYPASS_VIB:
                sprintf(s, "(int) Bypass / Enable Vibrato");
                break;
            default:
                sprintf(s, "Invalid inlet!");
                break;
        }

    }
    else {	// outlet
        switch(a)
        {
            case VIB_OUTLET_OUTPUT_SIGNAL:
                sprintf(s, "(signal) Output signal");
                break;
            case VIB_OUTLET_DELAY_MS:
                sprintf(s, "(signal) Delay control parameter");
                break;
            case VIB_OUTLET_DEBUG_LFO_AMP:
                sprintf(s, "(signal) LFO amplitude (debug)");
                break;
            case VIB_OUTLET_DEBUG_LFO_OFFSET:
                sprintf(s, "(signal) LFO offset (debug)");
                break;
            case VIB_OUTLET_DEBUG_LFO_SIGNAL:
                sprintf(s, "(signal) LFO signal (debug)");
                break;
            case VIB_OUTLET_DEBUG_BUFSIZE:
                sprintf(s, "(signal) delay line buffer size (debug)");
                break;
            default:
                sprintf(s, "Invalid outlet!");
                break;
        }

    }
}

//Action if input was a float
void Vibrato_float(t_Vibrato *x, double f)
{
    
    //Get inlet number where the data came from
    long in = proxy_getinlet((t_object *)x);
    
    switch(in)
    {
        //Rate
        case VIB_INLET_RATE_BPM:
            VIB_SetRate(x->pVIB, f);
            break;
        
        //Depth
        case VIB_INLET_DEPTH:
            VIB_SetDepth(x->pVIB, f);
            break;
            
        //Delay direct control
        case VIB_INLET_DELAY_MS_DIRECT_CONTROL:
            DEL_SetDelayMs(x->pVIB->pDEL, f);
            break;
            
        //Delay direct control
        case VIB_INLET_DELAYLINE_BUFFER_SIZE:
            DEL_SetMaxDelayMs(x->pVIB->pDEL, f);
            break;
            
        //Set the VIB_perform to Bypass / process
        case VIB_INLET_BYPASS_VIB:
            {
                if ((bool)f)
                {
                    x->pf_VIB_perform = &DAFXBypassVibrato;
                }
                else
                {
                    x->pf_VIB_perform = &DAFXVibrato;
                }
            }
            break;
            
        default:
            break;
    }
}

//Action if input was an int
void Vibrato_int(t_Vibrato *x, long n)
{
    Vibrato_float(x, (double)n);
}


// registers a function for the signal chain in Max
// This function is called if the input is a signal.
// It is possible to assign a different perform function with object_method() based on some condition
void Vibrato_dsp64(t_Vibrato *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    
    //This call adds the DSP operation of this MSP object to the signal chain
    //It is also possible to implement several perform functions, and assigning a different one to the DSP chain
    //chain based on some conditon
    object_method(dsp64, gensym("dsp_add64"), x, Vibrato_perform64, 0, NULL);
}


// this is the 64-bit perform method for audio vectors - note: it is possible to implement several perform functions and assign a different one with object_method() based on some condition
void Vibrato_perform64(t_Vibrato *x,
                      t_object *dsp64,
                      double **ins,
                      long numins,
                      double **outs,
                      long numouts,
                      long sampleframes,
                      long flags,
                      void *userparam)
{
    
    t_DAFXVibrato * pVIB = x->pVIB;
    
    t_double *InSignal = ins[VIB_INLET_INPUT_SIGNAL];
    
    t_double *OutSignal = outs[VIB_OUTLET_OUTPUT_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *DelayMSControl = outs[VIB_OUTLET_DELAY_MS];	// we get audio for each outlet of the object from the **outs argument
    t_double *LFOAmp = outs[VIB_OUTLET_DEBUG_LFO_AMP];	// we get audio for each outlet of the object from the **outs argument
    t_double *LFOOffset = outs[VIB_OUTLET_DEBUG_LFO_OFFSET];	// we get audio for each outlet of the object from the **outs argument
    t_double *LFOSignal = outs[VIB_OUTLET_DEBUG_LFO_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *DELBufsize = outs[VIB_OUTLET_DEBUG_BUFSIZE];	// we get audio for each outlet of the object from the **outs argument
    
    //Converting the incoming signal from double to float complex in a temporary array
    for (int i = 0; i < sampleframes; i++) {
        pVIB->p_input_buffer[i] = (float) InSignal[i];
    }
    
    // Call the Vibrato perform function
    performFunction VIB_perform = (performFunction) x->pf_VIB_perform;
    VIB_perform(pVIB);  
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutSignal[i] = (double) pVIB->p_output_buffer[i];
        
        DelayMSControl[i] = (double) pVIB->pDEL->delay_samples;
        LFOAmp[i] = (double) pVIB->pLFO->amp;
        LFOOffset[i] = (double) pVIB->pLFO->offset;
        LFOSignal[i] = (double) pVIB->pLFO->p_output_block[i];
        DELBufsize[i] = (double) pVIB->pDEL->buf_size;
          
    }
    
}