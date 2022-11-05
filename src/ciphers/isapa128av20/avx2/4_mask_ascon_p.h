#pragma once
#include<cstdint>
#include<stdio.h>
#include <immintrin.h>
#include "random/RandomBuffer.h"

typedef struct share4
{
	uint64_t s[4];
} share_4;

extern uint64_t C_not;
extern uint64_t C;
extern RandomBuffer randbuf;

/* Extenal functions definitions */
extern void _4S_ROUND(uint64_t c);

extern void Init4Shares(uint64_t state[], uint64_t rand[]);

extern void Get4Shares();

#define _4S_P12 \
    _4S_ROUND(0xf0);\
    _4S_ROUND(0xe1);\
    _4S_ROUND(0xd2);\
    _4S_ROUND(0xc3);\
    _4S_ROUND(0xb4);\
    _4S_ROUND(0xa5);\
    _4S_ROUND(0x96);\
    _4S_ROUND(0x87);\
    _4S_ROUND(0x78);\
    _4S_ROUND(0x69);\
    _4S_ROUND(0x5a);\
    _4S_ROUND(0x4b);


#define _4S_P6 \
    _4S_ROUND(0x96);\
    _4S_ROUND(0x87);\
    _4S_ROUND(0x78);\
    _4S_ROUND(0x69);\
    _4S_ROUND(0x5a);\
    _4S_ROUND(0x4b);


#define _4S_P1 \
    _4S_ROUND(0x4b);
