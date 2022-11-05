/* global definitions for the configuration of the program */

#define MASKING_ORDER 12
#define OPTIMIZE_SSE
// #define REFRESH_HPC
#define POOLING
#define COUNT 200000 / MASKING_ORDER
#define BITS 320
// #define UMA_AND

#if defined(REFRESH_GENERIC)
    #define REFRESH_ISW
#endif

#if defined(REFRESH_ISW) or defined(REFRESH_HPC)
    #define REFRESH
#endif