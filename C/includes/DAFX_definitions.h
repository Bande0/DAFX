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
    
#define     DAFX_BLOCK_SIZE     16
#define     DAFX_SAMPLE_RATE    FS_48k
    

// Math constant
#define     TWO_PI          6.283185307179586f
#define     INV_TWO_PI      0.159154943091895f
#define     ONE_PI          3.141592653589793f
#define     INV_ONE_PI      0.318309886183791f
#define     SQRT_TWO        1.414213562373095f
#define     INV_SQRT_TWO    0.7071067811865475
    
#define     DEG_TO_RAD	ONE_PI / 180.0f
#define     RAD_TO_DEG	180.0f * INV_ONE_PI

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define bool  int
    #define true  1
    #define false 0

    // MIN MAX definitions
    #define DAFX_MAX(x, y) (((x) > (y)) ? (x) : (y))
    #define DAFX_MIN(x, y) (((x) < (y)) ? (x) : (y))

    // FREE with null checking
    #define FREE(x) {if(x !=NULL) free(x);}
#else
    // MIN MAX definitions
    #define DAFX_MAX(x, y) ({x < y ? y: x;})
    #define DAFX_MIN(x, y) ({x > y ? y: x;})

    // FREE with null checking
    #define FREE(x) ({if(x !=NULL) free(x);})
#endif
    
    
#ifdef __cplusplus
}
#endif

#endif /* DAFX_definitions_h */
