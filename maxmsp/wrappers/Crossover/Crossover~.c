
#include "ext.h"			// standard Max include, always required (except in Jitter)
#include "ext_obex.h"		// required for "new" style objects
#include "z_dsp.h"			// required for MSP objects

#include "Crossover.h"
#include "DAFX_Crossover.h"
#include "DAFX_InitCrossover.h"
#include "DAFX_BiquadFilter.h"
#include "DAFX_InitBiquadFilter.h"

#include "DAFX_definitions.h"

#include <mach/mach_time.h>


// global class pointer variable
static t_class *Crossover_class = NULL;


//***********************************************************************************************


// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("DAFXCrossover~", (method)Crossover_new, (method)Crossover_free, (long)sizeof(t_Crossover), 0L, A_GIMME, 0);
    
    //adding methods to the object for handling different actions
    class_addmethod(c, (method)Crossover_dsp64,		"dsp64",	A_CANT, 0); //action if input is a signal
    class_addmethod(c, (method)Crossover_assist,	"assist",	A_CANT, 0); //action if mouse is hovered over an in/outlet
    class_addmethod(c, (method)Crossover_int,	"int",      A_LONG, 0); //action if input is an int
    class_addmethod(c, (method)Crossover_float,	"float",	A_FLOAT,0); //action if input is a float
    class_addmethod(c, (method)Crossover_bang, "bang", 0);
    
    class_dspinit(c); //this is always needed for MSP objects
    class_register(CLASS_BOX, c);
    Crossover_class = c;
    
}

//New instance creation function
//argument list has to be declared like this, because the class was created with class_new(...,A_GIMME,0).
//Had it been A_DEFFLOAT, then there would be a single float argument instead
void *Crossover_new(t_symbol *s, long argc, t_atom *argv)
{
    //instantiate an object x of class Crossover_class
    t_Crossover *x = (t_Crossover *)object_alloc(Crossover_class);
    
    if (x) {
        
        // Creating inlets
        // dsp_setup sets up these inlets as proxies!
        dsp_setup((t_pxobject *)x, Crossover_N_INLETS);	// MSP inlets: arg is # of inlets and is REQUIRED! use 0 if you don't need inlets
        
        //Creating outlets - note: no need to store pointers to them in the struct, as in Max object       
        for (int i = 0; i < Crossover_N_OUTLETS; i++) {
            outlet_new(x, "signal"); 		// signal outlet (note "signal" rather than NULL)
        }        
        
        // allocate data structure for dsp API
        x->pXOVER = (t_DAFXCrossover *) malloc(sizeof(t_DAFXCrossover));
        
        // Set the perform function pointer to Crossover
        x->pf_XOVER_perform = &DAFXProcessCrossover;
        
        // TODO: can we alter this from Max in runtime or do we need to rebuild?
        //Initialize the structure
        x->pXOVER->fs = FS_48k;
        x->pXOVER->block_size = DAFX_BLOCK_SIZE;        
        
        InitDAFXCrossover(x->pXOVER);          
    }
    return (x);
}


// Detaches the object from the DSP chain and deallocates memory
void Crossover_free(t_Crossover *x)
{
    dsp_free((t_pxobject *)x);
    DeallocDAFXCrossover(x->pXOVER);
}

//Action if mouse is hovered over the in/outlets
void Crossover_assist(t_Crossover *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { //inlet
        switch(a)
        {
            case XOVER_INLET_INPUT_SIGNAL:
                sprintf(s, "(signal) Input signal");
                break;
            case XOVER_INLET_CUTOFF_FREQUENCY:
                sprintf(s, "(int) Cutoff Frequency (Hz)");
                break;
            case XOVER_INLET_CASCADE_ORDER:
                sprintf(s, "(int) Number of cascaded biquads");
                break;
            case XOVER_INLET_BYPASS_XOVER:
                sprintf(s, "(int) Bypass / Enable Crossover");
                break;
            
            default:
                sprintf(s, "Invalid inlet!");
                break;
        }

    }
    else {	// outlet
        switch(a)
        {
            case XOVER_OUTLET_OUTPUT_LP_SIGNAL:
                sprintf(s, "(signal) LP output signal");
                break;
            case XOVER_OUTLET_OUTPUT_HP_SIGNAL:
                sprintf(s, "(signal) HP output signal");
                break;
            default:
                sprintf(s, "Invalid outlet!");
                break;
        }

    }
}

//Action if input was a float
void Crossover_float(t_Crossover *x, double f)
{
    //Get inlet number where the data came from
    long in = proxy_getinlet((t_object *)x);
    
    switch(in)
    {
        //Cutoff Frequency
        case XOVER_INLET_CUTOFF_FREQUENCY:
            XOVER_SetCutoffFrequency(x->pXOVER, (int)f);
            break;
            
        case XOVER_INLET_CASCADE_ORDER:
            XOVER_SetCascadeOrder(x->pXOVER, (int)f);
            break;
            
        //Set the XOVER_perform to Bypass / process
        case XOVER_INLET_BYPASS_XOVER:
            {
                switch((int)f) {
                    case 0:
                        x->pf_XOVER_perform = &DAFXProcessCrossover;
                        break;
                    case 1:
                        x->pf_XOVER_perform = &DAFXBypassCrossover;
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
void Crossover_int(t_Crossover *x, long n)
{
    Crossover_float(x, (double)n);
}

//Action if input was a bang
void Crossover_bang(t_Crossover *x)
{
    switch (proxy_getinlet((t_object *)x)) {
        
        default:
            break;
    }
}

// registers a function for the signal chain in Max
// This function is called if the input is a signal.
// It is possible to assign a different perform function with object_method() based on some condition
void Crossover_dsp64(t_Crossover *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    
    //This call adds the DSP operation of this MSP object to the signal chain
    //It is also possible to implement several perform functions, and assigning a different one to the DSP chain
    //chain based on some conditon
    object_method(dsp64, gensym("dsp_add64"), x, Crossover_perform64, 0, NULL);
}


// this is the 64-bit perform method for audio vectors - note: it is possible to implement several perform functions and assign a different one with object_method() based on some condition
void Crossover_perform64(t_Crossover *x,
                      t_object *dsp64,
                      double **ins,
                      long numins,
                      double **outs,
                      long numouts,
                      long sampleframes,
                      long flags,
                      void *userparam)
{
    
    t_DAFXCrossover * pXOVER = x->pXOVER;
    
    t_double *InSignal = ins[XOVER_INLET_INPUT_SIGNAL];		// we get audio for each inlet of the object from the **ins argument
    
    t_double *OutLPSignal = outs[XOVER_OUTLET_OUTPUT_LP_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutHPSignal = outs[XOVER_OUTLET_OUTPUT_HP_SIGNAL];	// we get audio for each outlet of the object from the **outs argumen
    
    //Converting the incoming signal from double to float complex in a temporary array
    for (int i = 0; i < sampleframes; i++) {
        pXOVER->p_input_block[i] = (float)InSignal[i];
    }
    
    // Call the Crossover perform function
    performFunction XOVER_perform = (performFunction) x->pf_XOVER_perform;
    XOVER_perform(pXOVER);  
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutLPSignal[i] = (double) pXOVER->pp_output_blocks[0][i];
        OutHPSignal[i] = (double) pXOVER->pp_output_blocks[1][i];
    }
    
}