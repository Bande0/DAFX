
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

#include "Overdrive.h"
#include "DAFX_Overdrive.h"
#include "DAFX_InitOverdrive.h"

#include "DAFX_definitions.h"

#include <mach/mach_time.h>


// global class pointer variable
static t_class *Overdrive_class = NULL;


//***********************************************************************************************


// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("DAFXOverdrive~", (method)Overdrive_new, (method)Overdrive_free, (long)sizeof(t_Overdrive), 0L, A_GIMME, 0);
    
    //adding methods to the object for handling different actions
    class_addmethod(c, (method)Overdrive_dsp64,		"dsp64",	A_CANT, 0); //action if input is a signal
    class_addmethod(c, (method)Overdrive_assist,	"assist",	A_CANT, 0); //action if mouse is hovered over an in/outlet
    class_addmethod(c, (method)Overdrive_int,	"int",      A_LONG, 0); //action if input is an int
    class_addmethod(c, (method)Overdrive_float,	"float",	A_FLOAT,0); //action if input is a float
    
    class_dspinit(c); //this is always needed for MSP objects
    class_register(CLASS_BOX, c);
    Overdrive_class = c;
    
}

//New instance creation function
//argument list has to be declared like this, because the class was created with class_new(...,A_GIMME,0).
//Had it been A_DEFFLOAT, then there would be a single float argument instead
void *Overdrive_new(t_symbol *s, long argc, t_atom *argv)
{
    //instantiate an object x of class Overdrive_class
    t_Overdrive *x = (t_Overdrive *)object_alloc(Overdrive_class);
    
    if (x) {
        
        // Creating inlets
        // dsp_setup sets up these inlets as proxies!
        dsp_setup((t_pxobject *)x, Overdrive_N_INLETS);	// MSP inlets: arg is # of inlets and is REQUIRED! use 0 if you don't need inlets
        
        //Creating outlets - note: no need to store pointers to them in the struct, as in Max object       
        for (int i = 0; i < Overdrive_N_OUTLETS; i++) {
            outlet_new(x, "signal"); 		// signal outlet (note "signal" rather than NULL)
        }        
        
        // allocate data structure for dsp API
        x->pOD = (t_DAFXOverdrive *) malloc(sizeof(t_DAFXOverdrive));
        
        // Set the perform function pointer to Sample Based Compressor (and not bypass)
        x->pf_OD_perform = &DAFXOverdrive;   
        
        // TODO: can we alter this from Max in runtime or do we need to rebuild?
        //Initialize the structure
        x->pOD->fs = FS_48k;
        x->pOD->block_size = DAFX_BLOCK_SIZE;
        
        InitDAFXOverdrive(x->pOD);          
    }
    return (x);
}


// Detaches the object from the DSP chain and deallocates memory
void Overdrive_free(t_Overdrive *x)
{
    dsp_free((t_pxobject *)x);
    DeallocDAFXOverdrive(x->pOD);
}

//Action if mouse is hovered over the in/outlets
void Overdrive_assist(t_Overdrive *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        switch(a)
        {
            case OD_INLET_INPUT_SIGNAL:
                sprintf(s, "(signal) Input signal");
                break;
            case OD_INLET_ALGO_SELECT:
                sprintf(s, "(int) algo selector. 0 - tanh, 1 - sin, 2 - exp");
                break;
            case OD_INLET_IN_GAIN:
                sprintf(s, "(float) input gain (linear)");
                break;
            case OD_INLET_OUT_GAIN:
                sprintf(s, "(float) output gain (linear)");
                break;
            case OD_INLET_PARAM:
                sprintf(s, "(float) Overdrive parameter (for tanh and exp)");
                break;
            case OD_INLET_THRESH:
                sprintf(s, "(float) Overdrive threshold (for sin and exp)");
                break;
            case OD_INLET_BYPASS_OD:
                sprintf(s, "(int) Bypass / Enable OD (lookup) / Enable OD (formula) ");
                break;
            
            default:
                sprintf(s, "Invalid inlet!");
                break;
        }


    }
    else {	// outlet
        switch(a)
        {
            case OD_OUTLET_OUTPUT_SIGNAL:
                sprintf(s, "(signal) Output signal");
                break;
            case OD_OUTLET_GAIN_CURVE:
                sprintf(s, "(signal) Gain curve");
                break;
            default:
                sprintf(s, "Invalid outlet!");
                break;
        }

    }
}


//Action if input was a float
void Overdrive_float(t_Overdrive *x, double f)
{
    
    //Get inlet number where the data came from
    long in = proxy_getinlet((t_object *)x);
    
    switch(in)
    {
        //Overdrive algo selector
        case OD_INLET_ALGO_SELECT:
            {
                switch((int)f) {
                    case OD_ALGO_SELECT_TANH:
                        x->pOD->algo = OD_ALGO_SELECT_TANH;
                        break;
                    case OD_ALGO_SELECT_SIN:
                        x->pOD->algo = OD_ALGO_SELECT_SIN;
                        break;
                    case OD_ALGO_SELECT_EXP:
                        x->pOD->algo = OD_ALGO_SELECT_EXP;
                        break;
                    default:
                        break;
                }
            }
            break;
            
        //Input gain
        case OD_INLET_IN_GAIN:
            x->pOD->in_gain = DAFX_MAX(DAFX_MIN(f, OD_INIT_IN_GAIN_MAX), OD_INIT_IN_GAIN_MIN);
            ReDrawGainCurve(x->pOD);
            break;
        
        //Output gain
        case OD_INLET_OUT_GAIN:
            x->pOD->out_gain = DAFX_MAX(DAFX_MIN(f, OD_INIT_OUT_GAIN_MAX), OD_INIT_OUT_GAIN_MIN);
            ReDrawGainCurve(x->pOD);
            break;
            
        //Overdrive parameter
        case OD_INLET_PARAM:
            {
                switch(x->pOD->algo) {
                    case OD_ALGO_SELECT_TANH:
                        x->pOD->tan_param = DAFX_MAX(DAFX_MIN(f, OD_INIT_TAN_MAX), OD_INIT_TAN_MIN);
                        break;
                    case OD_ALGO_SELECT_SIN:
                        break;
                    case OD_ALGO_SELECT_EXP:
                        x->pOD->exp_param = DAFX_MAX(DAFX_MIN(f, OD_INIT_EXP_MAX), OD_INIT_EXP_MIN);
                        break;
                    default:
                        break;
                }
            }
            ReDrawGainCurve(x->pOD);
            break;
            
            
        //Overdrive threshold
        case OD_INLET_THRESH:
            {
                switch(x->pOD->algo) {
                    case OD_ALGO_SELECT_TANH:
                        break;
                    case OD_ALGO_SELECT_SIN:
                    case OD_ALGO_SELECT_EXP:
                        x->pOD->thresh = DAFX_MAX(DAFX_MIN(f, OD_INIT_THRESH_MAX), OD_INIT_THRESH_MIN);
                        x->pOD->inv_thresh = 1.0f / x->pOD->thresh;
                        break;
                    default:
                        break;
                }
            }
            ReDrawGainCurve(x->pOD);
            break;
            
        //Set the OD_perform to Bypass / process
        case OD_INLET_BYPASS_OD:
            {
                if ((bool)f)
                {
                    x->pf_OD_perform = &DAFXBypassOverdrive;
                }
                else
                {
                    x->pf_OD_perform = &DAFXOverdrive;
                }
            }
            break;
            
        default:
            break;
    }
}

//Action if input was an int
void Overdrive_int(t_Overdrive *x, long n)
{
    Overdrive_float(x, (double)n);
}


// registers a function for the signal chain in Max
// This function is called if the input is a signal.
// It is possible to assign a different perform function with object_method() based on some condition
void Overdrive_dsp64(t_Overdrive *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    
    //This call adds the DSP operation of this MSP object to the signal chain
    //It is also possible to implement several perform functions, and assigning a different one to the DSP chain
    //chain based on some conditon
    object_method(dsp64, gensym("dsp_add64"), x, Overdrive_perform64, 0, NULL);
}


// this is the 64-bit perform method for audio vectors - note: it is possible to implement several perform functions and assign a different one with object_method() based on some condition
void Overdrive_perform64(t_Overdrive *x,
                      t_object *dsp64,
                      double **ins,
                      long numins,
                      double **outs,
                      long numouts,
                      long sampleframes,
                      long flags,
                      void *userparam)
{
    
    t_DAFXOverdrive * pOD = x->pOD;
    
    t_double *InSignal = ins[OD_INLET_INPUT_SIGNAL];		// we get audio for each inlet of the object from the **ins argument
    
    t_double *OutSignal = outs[OD_OUTLET_OUTPUT_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutGainCurve= outs[OD_OUTLET_GAIN_CURVE];	// we get audio for each outlet of the object from the **outs argument
    
    //Converting the incoming signal from double to float complex in a temporary array
    for (int i = 0; i < sampleframes; i++) {
        pOD->p_input_block[i] = (float)InSignal[i];
    }
    
    // Call the Overdrive perform function
    performFunction OD_perform = (performFunction) x->pf_OD_perform;
    OD_perform(pOD);  
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutSignal[i] = (double) pOD->p_output_block[i];
        OutGainCurve[i] = (double) pOD->p_gain_curve[i];
    }
    
}