#pragma once
#include<cstdint>
#include<stdio.h>
#include <immintrin.h>
#include "random/RandomBuffer.h"

typedef struct share8
{
	uint64_t s[8];
} share_8;

extern uint64_t C_not;
extern uint64_t C;
extern RandomBuffer randbuf;

/* External functions definitions */
extern void _8S_ROUND(uint64_t c);
extern void Init8Shares(uint64_t state[], uint64_t rand[]);
extern void Get8Shares();

#define _8S_P12 \
    _8S_ROUND(0xf0);\
    _8S_ROUND(0xe1);\
    _8S_ROUND(0xd2);\
    _8S_ROUND(0xc3);\
    _8S_ROUND(0xb4);\
    _8S_ROUND(0xa5);\
    _8S_ROUND(0x96);\
    _8S_ROUND(0x87);\
    _8S_ROUND(0x78);\
    _8S_ROUND(0x69);\
    _8S_ROUND(0x5a);\
    _8S_ROUND(0x4b);


#define _8S_P6 \
    _8S_ROUND(0x96);\
    _8S_ROUND(0x87);\
    _8S_ROUND(0x78);\
    _8S_ROUND(0x69);\
    _8S_ROUND(0x5a);\
    _8S_ROUND(0x4b);


#define _8S_P1 \
    _8S_ROUND(0x4b);
