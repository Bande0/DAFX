//
//  DAFX_definitions.h
//


#ifndef DAFX_definitions_h
#define DAFX_definitions_h

#ifdef __cplusplus
extern "C" {
#endif
    

#define     FS_48k   48000
#define     FS_44_1k 44100
#define     FS_24k   24000
#define     FS_16k   16000
    
#define     DAFX_BLOCK_SIZE  256

// Math constant
#define     TWO_PI      6.283185307179586
#define     INV_TWO_PI  0.159154943091895
#define     ONE_PI      3.141592653589793
#define     INV_ONE_PI  0.318309886183791
    
#define     DEG_TO_RAD	ONE_PI / 180.0
#define     RAD_TO_DEG	180.0 * INV_ONE_PI

// MIN MAX definitions
#define DAFX_MAX(x, y) ({x < y ? y: x;})
#define DAFX_MIN(x, y) ({x > y ? y: x;})
    
// FREE with null checking
#define FREE(x) ({if(x !=NULL) free(x);})
    
    
#ifdef __cplusplus
}
#endif

#endif /* DAFX_definitions_h */
