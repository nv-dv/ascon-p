#pragma once
#include<stdio.h>
#include<cstdint>

#include "random/RandomBuffer.h"
#include "globals.h"

typedef struct dshare_t
{
	uint64_t s[MASKING_ORDER];
} dshare;

extern uint64_t C_not;
extern uint64_t C;
extern RandomBuffer randbuf;

/* External Functions defintions */

extern void dS_ROUND(int C);
extern void InitDShares(uint64_t x[], uint64_t rand[]);
extern void GetDShares();

#define dS_P12 \
    dS_ROUND(0xf0);\
    dS_ROUND(0xe1);\
    dS_ROUND(0xd2);\
    dS_ROUND(0xc3);\
    dS_ROUND(0xb4);\
    dS_ROUND(0xa5);\
    dS_ROUND(0x96);\
    dS_ROUND(0x87);\
    dS_ROUND(0x78);\
    dS_ROUND(0x69);\
    dS_ROUND(0x5a);\
    dS_ROUND(0x4b);


#define dS_P6 \
    dS_ROUND(0x96);\
    dS_ROUND(0x87);\
    dS_ROUND(0x78);\
    dS_ROUND(0x69);\
    dS_ROUND(0x5a);\
    dS_ROUND(0x4b);


#define dS_P1 \
    dS_ROUND(0x4b);
