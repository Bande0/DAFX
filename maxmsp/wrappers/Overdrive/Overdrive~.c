
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

//helper function for displaying a marker in MaxMSP
static void UpdateMarkerOnGainCurve(t_Overdrive *x);

// Entry point - no arguments here
void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.
    
    t_class *c = class_new("Overdrive~", (method)Overdrive_new, (method)Overdrive_free, (long)sizeof(t_Overdrive), 0L, A_GIMME, 0);
    
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
        
        //set elapsed time to 0
        x->elapsed_NS_smooth = 0.0;
        
        //initialize 'gain marker' to out-of-bounds values (i.e. no marker displayed)
  //      x->p_current_in_out_gain_marker = (double *) malloc(HubbleNumOfSubbands * sizeof(double));
  //      for (int i = 0; i < HubbleNumOfSubbands; i++) {
  //          x->p_current_in_out_gain_marker[i] = OUT_OF_RANGE_NUMBER_ON_PLOT;
  //      }
        
        //Initialize the structure
  //      x->pOD->fs = HubbleSamplingFrequency;
  //      x->pOD->block_size = HubbleNumOfSubbands;
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
            case OD_INLET_COMP_THRESHOLD_DB:
                sprintf(s, "(float) Compressor Threshold dB");
                break;
            case OD_INLET_COMP_RATIO:
                sprintf(s, "(float) Compressor Ratio");
                break;
            case OD_INLET_EXP_THRESHOLD_DB:
                sprintf(s, "(float) Expander Threshold dB");
                break;
            case OD_INLET_EXP_RATIO:
                sprintf(s, "(float) Expander Ratio");
                break;
            case OD_INLET_KNEE_WIDTH_DB:
                sprintf(s, "(float) Knee Width dB");
                break;
            case OD_INLET_MAKEUP_GAIN_DB:
                sprintf(s, "(float) Make-up gain dB");
                break;
            case OD_INLET_COMP_ATTACK_TIME_MS:
                sprintf(s, "(float) Compressor Attack time (s)");
                break;
            case OD_INLET_COMP_RELEASE_TIME_MS:
                sprintf(s, "(float) Compressor Release time (s)");
                break;
            case OD_INLET_EXP_ATTACK_TIME_MS:
                sprintf(s, "(float) Expander Attack time (s)");
                break;
            case OD_INLET_EXP_RELEASE_TIME_MS:
                sprintf(s, "(float) Expander Release time (s)");
                break;
            case OD_INLET_BYPASS_OD:
                sprintf(s, "(int) Bypass / Enable OD (lookup) / Enable OD (formula) ");
                break;
            case OD_INLET_XVAD_OR_VAD_SWITCH:
                sprintf(s, "(int) XVAD (0) or VAD (1) mode switch");
                break;
            case OD_INLET_VAD_MEASURE:
                sprintf(s, "(float) VAD measure (xcorr, flatness, etc)");
                break;
            case OD_INLET_VAD_THRESHOLD:
                sprintf(s, "(float) VAD threshold for speech");
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
                sprintf(s, "(signal) Gain Function curve (dB)");
                break;
            case OD_OUTLET_APPLIED_GAIN_DB:
                sprintf(s, "(float) Applied Gain (dB)");
                break;
            case OD_OUTLET_RMS_POWER_LINEAR:
                sprintf(s, "(float) RMS power (linear)");
                break;
            case OD_OUTLET_RMS_POWER_SMOOTH:
                sprintf(s, "(float) RMS power smooth (linear)");
                break;
            case OD_OUTLET_ELAPSED_TIME_MS:
                sprintf(s, "(float) elapsed time (ms)");
                break;
            case OD_OUTLET_VAD_LABEL:
                sprintf(s, "(int) VAD label");
                break;
            case OD_OUTLET_CURRENT_MARKER_ON_GAIN_CURVE:
                sprintf(s, "(signal) displays the current marker on the gain curve, as a plottable vector");
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
        //Compressor Threshold dB
        case OD_INLET_COMP_THRESHOLD_DB:
            UpdateCompressorThreshold(x->pOD, (float) f);
            break;
            
        //Compressor Ratio
        case OD_INLET_COMP_RATIO:
            UpdateCompressorRatio(x->pOD, (float) f);
            break;
            
        //Expander Threshold dB
        case OD_INLET_EXP_THRESHOLD_DB:
            UpdateExpanderThreshold(x->pOD, (float) f);
            break;
            
        //Expander Ratio
        case OD_INLET_EXP_RATIO:
            UpdateExpanderRatio(x->pOD, (float) f);
            break;
            
        //Knee Width dB
        case OD_INLET_KNEE_WIDTH_DB:
            UpdateKneeWidth(x->pOD, (float) f);
            break;
            
        //Make-up gain dB
        case OD_INLET_MAKEUP_GAIN_DB:
            UpdateMakeupGain(x->pOD, (float) f);
            break;
            
        //Compressor Attack Time
        case OD_INLET_COMP_ATTACK_TIME_MS:
            {
                x->pOD->comp_attack_time = (float)f;
                RecalculateAttackAndReleaseTimes(x->pOD);
            }
            break;
            
        //Compressor Release Time
        case OD_INLET_COMP_RELEASE_TIME_MS:
            {
                x->pOD->comp_release_time = (float)f;
                RecalculateAttackAndReleaseTimes(x->pOD);
            }
            break;
            
        //Expander Attack Time
        case OD_INLET_EXP_ATTACK_TIME_MS:
        {
            x->pOD->exp_attack_time = (float)f;
            RecalculateAttackAndReleaseTimes(x->pOD);
        }
            break;
            
        //Expander Release Time
        case OD_INLET_EXP_RELEASE_TIME_MS:
        {
            x->pOD->exp_release_time = (float)f;
            RecalculateAttackAndReleaseTimes(x->pOD);
        }
            break;
            
        case OD_INLET_BYPASS_OD:
            { //Set the OD_perform to Bypass / ideal / optimized OD
                switch((int)f) {
                    case OD_BYPASS:
                        x->pf_OD_perform = &DAFXBypassOverdrive;
                        break;
                    case OD_IDEAL_GAIN_FUNCTION:
                        x->pf_OD_perform = &DAFXOverdrive;
                        x->pOD->pf_gain_func = &CalculateOutputGainFromFormula;
                        ReDrawGainCurve(x->pOD);
                        break;
                    case OD_OPTIMIZED_GAIN_FUNCTION:
                        x->pf_OD_perform = &DAFXOverdrive;
                        x->pOD->pf_gain_func = &CalculateOutputGainFromLookupTable;
                        ReDrawGainCurve(x->pOD);
                        break;
                }
            }
            break;
        
        //use XVAD or VAD
        case OD_INLET_XVAD_OR_VAD_SWITCH:
        {
            if ((bool)f)
            {
                x->pOD->use_xvad_or_vad = OD_USE_VAD;
                break;
            }
            
            else
            {
                x->pOD->use_xvad_or_vad = OD_USE_XVAD;
                break;
            }
        }
            
        case OD_INLET_VAD_MEASURE:
            x->pOD->VAD_measure = (float)f;
            break;
            
        case OD_INLET_VAD_THRESHOLD:
            x->pOD->VAD_measure_threshold = (float)f;
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
    
    //Variables for processing time measurement
    uint64_t startTime, endTime, elapsedMTU;
    double elapsedNS;
    double alpha = 0.95;
    mach_timebase_info_data_t info;
    
    t_double *InSignal = ins[OD_INLET_INPUT_SIGNAL];		// we get audio for each inlet of the object from the **ins argument
    
    t_double *OutSignal = outs[OD_OUTLET_OUTPUT_SIGNAL];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutGainCurve= outs[OD_OUTLET_GAIN_CURVE];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutAppliedGain_dB = outs[OD_OUTLET_APPLIED_GAIN_DB];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutRMS = outs[OD_OUTLET_RMS_POWER_LINEAR];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutRMS_Smooth = outs[OD_OUTLET_RMS_POWER_SMOOTH];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutElapsedMilliseconds = outs[OD_OUTLET_ELAPSED_TIME_MS];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutVADlabel = outs[OD_OUTLET_VAD_LABEL];	// we get audio for each outlet of the object from the **outs argument
    t_double *OutMarkerOnGainCurve = outs[OD_OUTLET_CURRENT_MARKER_ON_GAIN_CURVE];	// we get audio for each outlet of the object from the **outs argument
    
    
    //Converting the incoming signal from double to float complex in a temporary array
    for (int i = 0; i < sampleframes; i++) {
        pOD->p_input_block[i] = (float)InSignal[i];
    }
    
    startTime = mach_absolute_time();
    
    // Call the Compressor perform function
    performFunction OD_perform = (performFunction) x->pf_OD_perform;
    OD_perform(pOD);
    
    endTime = mach_absolute_time();
    elapsedMTU = endTime - startTime;
    
    mach_timebase_info(&info);
    elapsedNS = (double)elapsedMTU * (double)info.numer / (double)info.denom; //nanosec
    elapsedNS *= 0.001;
    x->elapsed_NS_smooth = alpha * x->elapsed_NS_smooth + (1.0 - alpha) * elapsedNS;
    
    UpdateMarkerOnGainCurve(x);
    
    for(int i = 0; i < sampleframes; i++){
        //Converting results from float back to double, which Max expects
        OutSignal[i] = (double) pOD->p_output_block[i];
        
        OutAppliedGain_dB[i] = (double) pOD->AppliedGain_db;
        OutRMS[i] = (double) pOD->rms_power;
        OutRMS_Smooth[i] = (double) pOD->rms_smoothed;
        OutGainCurve[i] = (double) pOD->p_displayed_gain_curve[i];
        
        OutElapsedMilliseconds[i] = x->elapsed_NS_smooth;
        OutVADlabel[i] = (double)pOD->VAD_label;
        OutMarkerOnGainCurve[i] = (double) x->p_current_in_out_gain_marker[i];
    }
    
}

//Calculates the position of a marker, showing where the current signal is on the gain curve
//Only for display in MaxMSP
void UpdateMarkerOnGainCurve(t_Overdrive *x)
{
    tf_gain_function pf_calculate_gain = (tf_gain_function) x->pOD->pf_gain_func; //Gain function currently pointed at
    int idx = 0;
    float min = 1000.0;
    float increment = (float)OD_GAIN_CURVE_RANGE_DB / (float)x->pOD->block_size;
    float y = -1.0 * OD_GAIN_CURVE_RANGE_DB;
    float temp;
    float lin_gain = x->pOD->makeup;
    float gain_offset = 0.0;
    
    //The 'marker' is a full curve, with out-of-bounds numbers throughout,
    //except for one value, which is the marker position
    for (int i = 0; i < x->pOD->block_size; i++)
    {
        x->p_current_in_out_gain_marker[i] = OUT_OF_RANGE_NUMBER_ON_PLOT;
    }
    
    //looking for position index of the marker in the gain curve
    for (int i = 0; i < x->pOD->block_size; i++)
    {
        temp = fabsf(20.0f * log10f(x->pOD->rms_smoothed) - y);
        
        if (temp < min)
        {
            min = temp;
            idx = i;
            gain_offset = y;
        }
        
        y += increment;
    }
    
    //if VAD == 1 and we are in expander range: don't apply the expander (i.e. gain == 1.0)
    //in all other cases, apply the OD.
    if (!((x->pOD->VAD_label) && (x->pOD->rms_smoothed < x->pOD->exp_threshold))){
        //Call the method for calculating the output gain
        lin_gain = pf_calculate_gain(x->pOD, x->pOD->rms_smoothed);
    }
    
    //setting the marker value at the right position, to the right value
    x->p_current_in_out_gain_marker[idx] = 20*log10f(lin_gain) + gain_offset;
}

