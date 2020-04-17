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
    
#define OD_USE_XVAD      0
#define OD_USE_VAD       1
    
#define OD_INIT_USE_XVAD_OR_VAD        OD_USE_VAD
#define	OD_INIT_COMP_THRESHOLD         0.017782794100389f //-35.0 dB
#define	OD_INIT_COMP_RATIO             5.0f
#define	OD_INIT_EXP_THRESHOLD          0.005623413251903f //-45.0 dB
#define	OD_INIT_EXP_RATIO              3.0f
#define	OD_INIT_KNEE_WIDTH_DB          5.0f
#define	OD_INIT_MAKEUP                 10.0f //20.0 dB
    
#define	OD_INIT_COMP_ATTACK_TIME       0.0015f
#define	OD_INIT_COMP_RELEASE_TIME      0.75f
#define	OD_INIT_EXP_ATTACK_TIME        0.1f
#define	OD_INIT_EXP_RELEASE_TIME       0.9f
    
#define	OD_LOOKUP_POINTS_EXP           8   //number of points in the lookup table for the expansion part
#define	OD_LOOKUP_POINTS_COMP          11  //number of points in the lookup table for the compression part
#define	OD_LOOKUP_SIZE                 OD_LOOKUP_POINTS_EXP + OD_LOOKUP_POINTS_COMP  //total size of the lookup table
#define	OD_LOOKUP_LOWLIMIT_DB          -80.0f //lowest dB value represented by the lookup table
    
#define	OD_GAIN_CURVE_RANGE_DB         45.0f //minimum dB value displayed on the drawn gain curve x-axis
    
#define SMOOTHER_INIT_DELAY_VALUE       0.00001f; //-50.0f
#define SMOOTHER_COMP_MODE              0
#define SMOOTHER_EXP_MODE               1
#define SMOOTHER_INIT_MODE              SMOOTHER_COMP_MODE
    
#define SMOOTHER_LAMBDA_RISE_CORRECTION 0.476190476190476 // 1.0 / 2.1
#define SMOOTHER_LAMBDA_FALL_CORRECTION 2.1
    
#define OD_INIT_VAD_THRESHOLD   -0.5f
#define OD_INIT_XVAD_THRESHOLD   0.25f
    
    
    
#ifdef __cplusplus
}
#endif

#endif /* InitAdspOverdrive_h */
