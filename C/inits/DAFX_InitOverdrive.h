//
//  DAFX_InitOverdrive.h
//  Overdrive~
//


#ifndef DAFX_InitOverdrive_h
#define DAFX_InitOverdrive_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include "DAFX_definitions.h"
    
#define OD_INIT_DEFAULT_IN_GAIN        1.0
#define OD_INIT_DEFAULT_OUT_GAIN       0.75
#define OD_INIT_DEFAULT_THRESHOLD      0.66
#define OD_INIT_DEFAULT_TAN_PARAM      5.0
#define OD_INIT_DEFAULT_EXP_PARAM      2.0
    
#define OD_INIT_THRESH_MIN             0.01
#define OD_INIT_THRESH_MAX             1.0
#define OD_INIT_TAN_MIN                3.0
#define OD_INIT_TAN_MAX                150.0
#define OD_INIT_EXP_MIN                2.0
#define OD_INIT_EXP_MAX                10.0
#define OD_INIT_IN_GAIN_MIN            0.0
#define OD_INIT_IN_GAIN_MAX            10.0
#define OD_INIT_OUT_GAIN_MIN           0.01
#define OD_INIT_OUT_GAIN_MAX           0.99   
    
  
    
#ifdef __cplusplus
}
#endif

#endif /* InitAdspOverdrive_h */
