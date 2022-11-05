#pragma once
#include<cstdint>
#include<stdio.h>
#include <immintrin.h>
#include "random/RandomBuffer.h"

typedef struct share2
{
	uint64_t s0;
	uint64_t s1;
} share_2;

extern uint64_t C_not;
extern uint64_t C;
extern RandomBuffer randbuf;

/* External Functions definitions */
extern void _2S_ROUND(uint64_t c);

#define _2S_P12 \
    _2S_ROUND(0xf0);\
    _2S_ROUND(0xe1);\
    _2S_ROUND(0xd2);\
    _2S_ROUND(0xc3);\
    _2S_ROUND(0xb4);\
    _2S_ROUND(0xa5);\
    _2S_ROUND(0x96);\
    _2S_ROUND(0x87);\
    _2S_ROUND(0x78);\
    _2S_ROUND(0x69);\
    _2S_ROUND(0x5a);\
    _2S_ROUND(0x4b);\


#define _2S_P6 \
    _2S_ROUND(0x96);\
    _2S_ROUND(0x87);\
    _2S_ROUND(0x78);\
    _2S_ROUND(0x69);\
    _2S_ROUND(0x5a);\
    _2S_ROUND(0x4b);\


#define _2S_P1 \
    _2S_ROUND(0x4b);\

extern void Init2Shares(uint64_t state[], uint64_t rand[]);

extern void Get2Shares();
