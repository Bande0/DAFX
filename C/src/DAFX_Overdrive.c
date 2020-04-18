//
//  DAFX_Overdrive.c
//  Overdrive~
//

#include "DAFX_Overdrive.h"
#include "DAFX_InitOverdrive.h"
#include "DAFX_definitions.h"
#include <Accelerate/Accelerate.h>

bool InitDAFXOverdrive(t_DAFXOverdrive *pOD)
{
    //Signal vector size
    int block_size = pOD->block_size;
    
    // I/O buffers
    pOD->p_input_block = (float *) calloc(block_size, sizeof(float));
    pOD->p_output_block = (float *) calloc(block_size, sizeof(float));
    
    //degault overdrive method
    pOD->algo = OD_ALGO_SELECT_TANH;
    
    //OD params
    pOD->in_gain = OD_INIT_DEFAULT_IN_GAIN;
    pOD->out_gain = OD_INIT_DEFAULT_OUT_GAIN;
    pOD->thresh = OD_INIT_DEFAULT_THRESHOLD;
    pOD->inv_thresh = 1.0 / OD_INIT_DEFAULT_THRESHOLD;
    pOD->tan_param = OD_INIT_DEFAULT_TAN_PARAM;
    pOD->exp_param = OD_INIT_DEFAULT_EXP_PARAM;
    
    //Allocate memory for the displayed gain curve
    // Dependency - the displayed gain curve is the same size as the signal block length
    pOD->p_gain_curve = (float *) calloc(block_size, sizeof(float));
    ReDrawGainCurve(pOD);
    
    return true;
}

bool DAFXOverdrive(t_DAFXOverdrive *pOD)
{
    int block_size = pOD->block_size;
    float *pInput = pOD->p_input_block;
    float *pOutput = pOD->p_output_block;
    
    float in_gain = pOD->in_gain;
    float out_gain = pOD->out_gain;
    float tan_param = pOD->tan_param;
    float exp_param = pOD->exp_param;
    float thresh = pOD->thresh;
    float saturation = 0.5 * pOD->inv_thresh; //for sinus only
    
    //TODO: TURN THE THREE METHODS INTO 3 DIFFERENT PROCESS FUNCTIONS SO THAT THE GAIN PLOTTER CAN CALL THEM TOO
    for (int i = 0; i < block_size; i++)
    {
        //apply input gain
        float x = in_gain * pInput[i];
        
        // tanh
        pOutput[i] = out_gain * tanh(tan_param * x);
        
        //sin
    //    if (x > thresh)
    //    {
    //        pOutput[i] = out_gain;
    //    }
    //    else if (x < -1.0*thresh)
    //    {
    //        pOutput[i] = -1.0 * out_gain;
    //    }
    //    else
    //    {
    //        pOutput[i] = out_gain * sinf(saturation * ONE_PI * x);
    //    }
    }
    
    return true;
}

bool DAFXBypassOverdrive(t_DAFXOverdrive *pOD)
{
    memcpy(pOD->p_output_block, pOD->p_input_block, sizeof(float) * pOD->block_size);
    return true;
}


//Re-draws a curve of the currently used gain function for display purposes in Max
//Calculates the output gain for all input gain values
bool ReDrawGainCurve(t_DAFXOverdrive *pOD)
{
    // The displayed gain curve is the same length as teh signal block size
    float increment = 1.0f / (float)pOD->block_size;
    float x = 0.0;
    
    //TODO delete these when the function pointers will be implemented properly
    float in_gain = pOD->in_gain;
    float out_gain = pOD->out_gain;
    float tan_param = pOD->tan_param;
    float exp_param = pOD->exp_param;
    float thresh = pOD->thresh;
    float saturation = 0.5 * pOD->inv_thresh; //for sinus only
    
  //  tf_gain_function pf_calculate_gain = (tf_gain_function) pOD->pf_gain_func; //Overdrive function currently pointed at
    
    for (int i = 0; i < pOD->block_size; i++)
    {
        //TODO: CALL THE CURRENT OVERDRIVE PROCESS FUNCTION HERE
        
        //tanh
        pOD->p_gain_curve[i] = pOD->out_gain * tanh(pOD->tan_param * x);
        
        //sin
 //       if (x > thresh)
 //       {
 //           pOD->p_gain_curve[i] = out_gain;
 //       }
 //       else if (x < -1.0*thresh)
 //       {
 //           pOD->p_gain_curve[i] = -1.0 * out_gain;
 //       }
 //       else
 //       {
 //           pOD->p_gain_curve[i] = out_gain * sinf(saturation * ONE_PI * x);
 //       }
               
        x += increment;
    }
    
    return true;
}

void DeallocDAFXOverdrive(t_DAFXOverdrive *pOD)
{
    FREE(pOD->p_input_block);
    FREE(pOD->p_output_block);
    FREE(pOD->p_gain_curve);
}
