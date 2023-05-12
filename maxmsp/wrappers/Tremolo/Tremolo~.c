
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

#include "Tremolo.h"
#include "DAFX_Tremolo.h"
#include "DAFX_InitTremolo.h"

#include "DAFX_definitions.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#else
#include <mach/mach_time.h>
#endif


// global class pointer variable
static t_class *Tremolo_class = NULL;


//***********************************************************************************************


// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("DAFXTremolo~", (method)Tremolo_new, (method)Tremolo_free, (long)sizeof(t_Tremolo), 0L, A_GIMME, 0);
    
    //adding methods to the object for handling different actions
    class_addmethod(c, (method)Tremolo_dsp64,		"dsp64",	A_CANT, 0); //action if input is a signal
    class_addmethod(c, (method)Tremolo_assist,	"assist",	A_CANT, 0); //action if mouse is hovered over an in/outlet
    class_addmethod(c, (method)Tremolo_int,	"int",      A_LONG, 0); //action if input is an int
    class_addmethod(c, (method)Tremolo_float,	"float",	A_FLOAT,0); //action if input is a float
    
    class_dspinit(c); //this is always needed for MSP objects
    class_register(CLASS_BOX, c);
    Tremolo_class = c;
    
}

//New instance creation function
//argument list has to be declared like this, because the class was created with class_new(...,A_GIMME,0).
//Had it been A_DEFFLOAT, then there would be a single float argument instead
void *Tremolo_new(t_symbol *s, long argc, t_atom *argv)
{
    //instantiate an object x of class Tremolo_class
    t_Tremolo *x = (t_Tremolo *)object_alloc(Tremolo_class);
    
    if (x) {
        
        // Creating inlets
        // dsp_setup sets up these inlets as proxies!
        dsp_setup((t_pxobject *)x, Tremolo_N_INLETS);	// MSP inlets: arg is # of inlets and is REQUIRED! use 0 if you don't need inlets
        
        //Creating outlets - note: no need to store pointers to them in the struct, as in Max object       
        for (int i = 0; i < Tremolo_N_OUTLETS; i++) {
            outlet_new(x, "signal"); 		// signal outlet (note "signal" rather than NULL)
        }        
        
        // allocate data structure for dsp API
        x->pTREM = (t_DAFXTremolo *) malloc(sizeof(t_DAFXTremolo));
        
        // Set the perform function pointer to Sample Based Compressor (and not bypass)
        x->pf_TREM_perform = &DAFXTremolo;   
        
        // TODO: can we alter this from Max in runtime or do we need to rebuild?
        //Initialize the structure
        x->pTREM->fs = FS_48k;
        x->pTREM->block_size = DAFX_BLOCK_SIZE;
        InitDAFXTremolo(x->pTREM);          
    }
    return (x);
}


// Detaches the object from the DSP chain and deallocates memory
void Tremolo_free(t_Tremolo *x)
{
    dsp_free((t_pxobject *)x);
    DeallocDAFXTremolo(x->pTREM);
}

//Action if mouse is hovered over the in/outlets
void Tremolo_assist(t_Tremolo *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        switch(a)
        {
            case TREM_INLET_INPUT_SIGNAL:
                sprintf(s, "(signal) Input signal");
                break;
            case TREM_INLET_RATE_BPM:
                sprintf(s, "(int) Tremolo Rate (BPM)");
                break;
            case TREM_INLET_DEPTH_PERCENT:
                sprintf(s, "(int) tremolo depth (percent)");
                break;
            case TREM_INLET_SHARPNESS:
                sprintf(s, "(float) 'sharpness' (smooth transition between 0:sinewave, 1:squarewave)");
                break;
            case TREM_INLET_POSTGAIN:
                sprintf(s, "(float) post-gain");
                break;
            case TREM_INLET_BYPASS_TREM:
                sprintf(s, "(int) Bypass / Enable Tremolo");
                break;
            default:
                sprintf(s, "Invalid inlet!");
                break;
        }

    }
    else {	// outlet
        switch(a)
        {
            case TREM_OUTLET_OUTPUT_SIGNAL:
                sprintf(s, "(signal) Output signal");
                break;
            case TREM_OUTLET_LFO_SIGNAL:
                sprintf(s, "(signal) LFO signal");
                break;
            case TREM_OUTLET_LFO_OFFSET:
                sprintf(s, "(signal) LFO offset");
                break;
            default:
                sprintf(s, "Invalid outlet!");
                break;
        }

    }
}

//Action if input was a float
void Tremolo_float(t_Tremolo *x, double f)
{
    
    //Get inlet number where the data came from
    long in = proxy_getinlet((t_object *)x);
    
    switch(in)
    {
        //Rate
        case TREM_INLET_RATE_BPM:
            SetRate(x->pTREM, f);
            break;
        
        //Depth
        case TREM_INLET_DEPTH_PERCENT:
            SetDepth(x->pTREM, f);
            break;
            
        //Sharpness
        case TREM_INLET_SHARPNESS:
            SetSharpness(x->pTREM, f);
            break;
            
        //Sharpness
        case TREM_INLET_POSTGAIN:
            SetPostGain(x->pTREM, f);
            break;
            
        //Set the TREM_perform to Bypass / process
        case TREM_INLET_BYPASS_TREM:
            {
                if ((bool)f)
                {
                    x->pf_TREM_perform = &DAFXBypassTremolo;
                }
                else
                {
                    x->pf_TREM_perform = &DAFXTremolo;
                }
            }
            break;
            
        default:
            break;
    }
}

//Action if input was an int
void Tremolo_int(t_Tremolo *x, long n)
{
    Tremolo_float(x, (double)n);
}


// registers a function for the signal chain in Max
// This function is called if the input is a signal.
// It is possible to assign a different perform function with object_method() based on some condition
void Tremolo_dsp64(t_Tremolo *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    
    //This call adds the DSP operation of this MSP object to the signal chain
    //It is also possible to implement several perform functions, and assigning a different one to the DSP chain
    //chain based on some conditon
    object_method(dsp64, gensym("dsp_add64"), x, Tremolo_perform64, 0, NULL);
}


// this is the 64-bit perform method for audio vectors - note: it is possible to implement several perform functions and assign a different one with object_method() based on some condition
void Tremolo_perform64(t_Tremolo *x,
                      t_object *dsp64,
                      double **ins,
                      long numins,
                      double **outs,
                      long numouts,
                      long sampleframes,
                      long flags,
                      void *userparam)
{
    
    t_DAFXTremolo * pTREM = x->pTREM;
    
    t_double *InSignal = ins[TREM_INLET_INPUT_SIGNAL];
    t_double *OutSignal = outs[TREM_OUTLET_OUTPUT_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *LFOSignal = outs[TREM_OUTLET_LFO_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *LFOOffset = outs[TREM_OUTLET_LFO_OFFSET];	// we get audio for each outlet of the object from the **outs argument
    
    //Converting the incoming signal from double to float complex in a temporary array
    for (int i = 0; i < sampleframes; i++) {
        pTREM->p_input_block[i] = (float) InSignal[i];
    }
    
    // Call the Tremolo perform function
    performFunction TREM_perform = (performFunction) x->pf_TREM_perform;
    TREM_perform(pTREM);  
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutSignal[i] = (double) pTREM->p_output_block[i];
        LFOSignal[i] = (double) pTREM->p_LFO->p_output_block[i];
        LFOOffset[i] = (double) pTREM->p_LFO->offset;
    }
    
}