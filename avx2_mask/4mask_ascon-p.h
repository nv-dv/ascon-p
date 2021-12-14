#pragma once
#include<cstdint>
#include<stdio.h>
#include "../RandomBuffer/RandomBuffer.h"

// state words x0..x4 (uint64), temporary variables t0..t4 (uint64)
// 0
//x2 ^= c
//x0 ^= x4; //1 x4 ^= x3;    x2 ^= x1;
// 2
//t0 = x0;    t1 = x1;    t2 = x2;    t3 = x3;    t4 = x4;
// 3
//t0 = ~t0;    t1 = ~t1;    t2 = ~t2;    t3 = ~t3;    t4 = ~t4;
//t0 &= x1;    t1 &= x2;    t2 &= x3;    t3 &= x4;    t4 &= x0;
// 4
//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
// 5
//x1 ^= x0;    x0 ^= x4;    x3 ^= x2;    x2 = ~x2;
// linear layer
//t0 = x0 ^ ROTR(x0, 28);
//t1 = x1 ^ ROTR(x1, 39);
//t2 = x2 ^ ROTR(x2, 6);
//t3 = x3 ^ ROTR(x3, 17);
//t4 = x4 ^ ROTR(x4, 41);
//t0 ^= ROTR(x0, 19);
//t1 ^= ROTR(x1, 61);
//t2 ^= ROTR(x2, 1);
//t3 ^= ROTR(x3, 10);
//t4 ^= ROTR(x4, 7);
//for (int i = 0;i < 5;++i) x[i] = t[i];

typedef struct share4
{
	uint64_t s[4];
} share_4;

#ifndef CONSTS
	#define CONSTS
	uint64_t C_not = 0xffffffffffffffff;
	uint64_t C;
	RandomBuffer randbuf = RandomBuffer(500 * 2<<20);
	#define TYPE_UINT64 8
	#define TYPE_UINT128 16
	#define TYPE_UINT256 32
#endif

uint64_t D[4] = {0,0,0,0};

uint64_t R4[40];
share_4 ssx0;
share_4 ssx1;
share_4 ssx2;
share_4 ssx3;
share_4 ssx4;
share_4 sst0;
share_4 sst1;
share_4 sst2;
share_4 sst3;
share_4 sst4;

// ymm3 >> 64 --> ymm3
#define _ymm3_rotl() \
	/*         0x93='0b10010011' --> 2 1 0 3 */ \
	asm("vpermq ymm3, ymm3, 0x93");  \

#define _ymm3_rotr() \
	/*         0x93='0b00111001' --> 0 3 2 1 */ \
	asm("vpermq ymm3, ymm3, 0x39");  \

#define _ymm4_rotr() \
	asm("vpermq ymm4, ymm4, 0x39");  \

#define _4S_LOAD(p_share, x, rand) \
	asm("movq xmm0, %0"::"m"(x):);  \
	for (size_t i = 0; i < 3; ++i) {  \
	asm("movq xmm1, %0"::"m"(rand[i]));  \
	asm("xorpd xmm0, xmm1");  \
	p_share.s[i] = rand[i];}  \
	asm("movq %0, xmm0":"=m"(p_share.s[3])::);  \
	

// s0 ^ s1 --> result
#define _4S_GET(p_share, result) \
	asm("movq xmm0, %0"::"m"(p_share.s[0]):);  \
	for (size_t i = 1; i < 4; ++i) {  \
	asm("movq xmm1, %0"::"m"(p_share.s[i]));  \
	asm("xorpd xmm0, xmm1");}  \
	asm("movq %0, xmm0":"=m"(result)::);  \

// xor share1, share2 --> ymm0
#define _4S_XOR(p_share1, p_share2) \
	asm("vmovdqu ymm0, %0"::"m"(p_share1):);  \
	asm("vmovdqu ymm1, %0"::"m"(p_share2):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \

// not with xor 0xff..ff --> ymm0
#define _4S_NOT(p_share) \
	asm("vmovdqu ymm0, %0"::"m"(p_share):);  \
	D[3] = C_not;  \
	asm("vmovdqu ymm1, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \

// and of shares 1 and 2 --> res_share
#define _4S_AND(p_share1, p_share2, res_share, i) \
	/* Inputs refresh */ \
	asm("vmovdqu ymm4, %0"::"m"((((__m256*)R4)[i])):);  \
	asm("vmovdqu ymm0, %0"::"m"(p_share1):);  \
	asm("vxorpd ymm0, ymm0, ymm4":::);  \
	_ymm4_rotr() \
	asm("vxorpd ymm0, ymm0, ymm4":::);  \
	asm("vxorpd ymm1, ymm1, ymm1":::);  \
	/*body of AND gate*/   \
	asm("vmovdqu ymm3, %0"::"m"(p_share2):);  \
	/* Do 4 times */  \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	/* first refresh */  \
	asm("vmovdqu ymm4, %0"::"m"((((__m256*)R4)[i+1])):);  \
	asm("vxorpd ymm1, ymm1, ymm4");  \
	_ymm4_rotr() \
	_ymm3_rotr() \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	_ymm3_rotr() \
	_ymm3_rotr() \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	/*Second refresh*/  \
	asm("vxorpd ymm1, ymm1, ymm4");  \
	_ymm3_rotl() \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	/* Write results */  \
	asm("vmovdqu %0, ymm1":"=m"(res_share)::);  \

// rotate right share by i bits --> xmm0
#define _4S_ROTR(p_share, i) \
	asm("vxorpd ymm0, ymm0, ymm0":::);  \
	asm("vmovdqu ymm0, %0"::"m"(p_share):);  \
	asm("vmovdqu ymm1, ymm0":::);  \
	asm("vpsrlq ymm0, ymm0, %0"::"i"(i):);  \
	asm("vpsllq ymm1, ymm1, %0"::"i"(64-i):);  \
	asm("vorpd ymm0, ymm0, ymm1":::);  \

#define _4S_LDL(p_share, rot1, rot2) \
	_4S_ROTR(p_share, rot1) \
	asm("vmovdqa ymm2, ymm0":::);  \
	/* TODO: add refresh */ \
	_4S_ROTR(p_share, rot2) \
	asm("vmovdqa ymm3, ymm0":::);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu ymm3, %0"::"m"(p_share):);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu %0, ymm2":"=m"(p_share)::);  \

#define _4S_ROUND(c) \
	D[3] = c;\
	/*0*/\
	asm("vmovdqu ymm0, %0"::"m"(ssx2):);  \
	asm("vxorpd ymm1, ymm1, ymm1":::);  \
	asm("vmovdqu ymm1, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \
	asm("vmovdqu %0, ymm0":"=m"(ssx2)::);  \
	/*_4S_XOR(ssx0, ssx4)*/ \
	asm("vmovdqu ymm0, %0"::"m"(ssx0):);  \
	asm("vmovdqu ymm1, %0"::"m"(ssx4):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \
	asm("vmovdqu %0, ymm0":"=m"(ssx0)::);  \
	/*_4S_XOR(ssx4, ssx3)*/ \
	asm("vmovdqu ymm2, %0"::"m"(ssx4):);  \
	asm("vmovdqu ymm3, %0"::"m"(ssx3):);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu %0, ymm2":"=m"(ssx4)::);  \
	/*_4S_XOR(ssx2, ssx1)*/ \
	asm("vmovdqu ymm4, %0"::"m"(ssx2):);  \
	asm("vmovdqu ymm5, %0"::"m"(ssx1):);  \
	asm("vxorpd ymm4, ymm4, ymm5":::);  \
	asm("vmovdqu %0, ymm4":"=m"(ssx2)::);  \
	/*2*/\
	/*_4S_NOT(ssx0)*/ \
	D[3] = C_not;  \
	asm("vmovdqu ymm6, %0"::"m"(ssx0):);  \
	asm("vmovdqu ymm7, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm6, ymm6, ymm7":::);  \
	asm("vmovdqu %0, ymm6":"=m"(sst0)::);  \
	/*_4S_NOT(ssx1)*/ \
	asm("vmovdqu ymm0, %0"::"m"(ssx1):);  \
	asm("vmovdqu ymm1, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \
	asm("vmovdqu %0, ymm0":"=m"(sst1)::);  \
	/*_4S_NOT(ssx2)*/ \
	asm("vmovdqu ymm2, %0"::"m"(ssx2):);  \
	asm("vmovdqu ymm3, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu %0, ymm2":"=m"(sst2)::);  \
	/*_4S_NOT(ssx3)*/ \
	asm("vmovdqu ymm4, %0"::"m"(ssx3):);  \
	asm("vmovdqu ymm5, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm4, ymm4, ymm5":::);  \
	asm("vmovdqu %0, ymm4":"=m"(sst3)::);  \
	/*_4S_NOT(ssx4)*/ \
	asm("vmovdqu ymm6, %0"::"m"(ssx4):);  \
	asm("vmovdqu ymm7, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm6, ymm6, ymm7":::);  \
	asm("vmovdqu %0, ymm6":"=m"(sst4)::);  \
	/*3*/\
	/*LOAD R*/\
	randbuf.GetBytes((uint8_t*)R4, sizeof(R4)); \
	_4S_AND(sst0, ssx1, sst0 ,0) \
	_4S_AND(sst1, ssx2, sst1, 2) \
	_4S_AND(sst2, ssx3, sst2, 4) \
	_4S_AND(sst3, ssx4, sst3, 6) \
	_4S_AND(sst4, ssx0, sst4, 8) \
	/*4*/\
	/*_4S_XOR(ssx0, sst1)*/ \
	asm("vmovdqu ymm0, %0"::"m"(ssx0):);  \
	asm("vmovdqu ymm1, %0"::"m"(sst1):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \
	asm("vmovdqu %0, ymm0":"=m"(ssx0)::);  \
	/*_4S_XOR(ssx1, sst2)*/ \
	asm("vmovdqu ymm2, %0"::"m"(ssx1):);  \
	asm("vmovdqu ymm3, %0"::"m"(sst2):);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu %0, ymm2":"=m"(ssx1)::);  \
	/*_4S_XOR(ssx2, sst3)*/ \
	asm("vmovdqu ymm4, %0"::"m"(ssx2):);  \
	asm("vmovdqu ymm5, %0"::"m"(sst3):);  \
	asm("vxorpd ymm4, ymm4, ymm5":::);  \
	asm("vmovdqu %0, ymm4":"=m"(ssx2)::);  \
	/*_4S_XOR(ssx3, sst4)*/ \
	asm("vmovdqu ymm6, %0"::"m"(ssx3):);  \
	asm("vmovdqu ymm7, %0"::"m"(sst4):);  \
	asm("vxorpd ymm6, ymm6, ymm7":::);  \
	asm("vmovdqu %0, ymm6":"=m"(ssx3)::);  \
	/*_4S_XOR(ssx4, sst0)*/ \
	asm("vmovdqu ymm0, %0"::"m"(ssx4):);  \
	asm("vmovdqu ymm1, %0"::"m"(sst0):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \
	asm("vmovdqu %0, ymm0":"=m"(ssx4)::);  \
	/*5*/\
	/*_4S_XOR(ssx1, ssx0)*/ \
	asm("vmovdqu ymm2, %0"::"m"(ssx1):);  \
	asm("vmovdqu ymm3, %0"::"m"(ssx0):);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu %0, ymm2":"=m"(ssx1)::);  \
	/*_4S_XOR(ssx0, ssx4)*/ \
	asm("vmovdqu ymm4, %0"::"m"(ssx0):);  \
	asm("vmovdqu ymm5, %0"::"m"(ssx4):);  \
	asm("vxorpd ymm4, ymm4, ymm5":::);  \
	asm("vmovdqu %0, ymm4":"=m"(ssx0)::);  \
	/*_4S_XOR(ssx3, ssx2)*/ \
	asm("vmovdqu ymm6, %0"::"m"(ssx3):);  \
	asm("vmovdqu ymm7, %0"::"m"(ssx2):);  \
	asm("vxorpd ymm6, ymm6, ymm7":::);  \
	asm("vmovdqu %0, ymm6":"=m"(ssx3)::);  \
	/*_4S_NOT(ssx2)*/ \
	asm("vmovdqu ymm0, %0"::"m"(ssx2));  \
	asm("vmovdqu ymm1, %0"::"m"((((__m256*)D)[0])):);  \
	asm("vxorpd ymm0, ymm0, ymm1":::);  \
	asm("vmovdqu %0, ymm0":"=m"(ssx2)::);  \
	/*6*/\
	_4S_LDL(ssx0, 19, 28) \
	_4S_LDL(ssx1, 39, 61) \
	_4S_LDL(ssx2, 1, 6) \
	_4S_LDL(ssx3, 10, 17) \
	_4S_LDL(ssx4, 7, 41) \
	/*end*/ \

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
    _4S_ROUND(0x4b);\


#define _4S_P6 \
    _4S_ROUND(0x96);\
    _4S_ROUND(0x87);\
    _4S_ROUND(0x78);\
    _4S_ROUND(0x69);\
    _4S_ROUND(0x5a);\
    _4S_ROUND(0x4b);\


#define _4S_P1 \
    _4S_ROUND(0x4b);\

inline void Init4Shares(uint64_t state[], uint64_t rand[]) {
	uint64_t tmp = state[0];
	_4S_LOAD(ssx0, tmp, rand);
	tmp = state[1];
	rand += 3;
	_4S_LOAD(ssx1, tmp, rand);
	tmp = state[2];
	rand += 3;
	_4S_LOAD(ssx2, tmp, rand);
	tmp = state[3];
	rand += 3;
	_4S_LOAD(ssx3, tmp, rand);
	tmp = state[4];
	rand += 3;
	_4S_LOAD(ssx4, tmp, rand);
}

inline void Get4Shares() {
	uint64_t res;
	_4S_GET(ssx0, res);
	printf("%.16llx ", res);
	_4S_GET(ssx1, res);
	printf("%.16llx ", res);
	_4S_GET(ssx2, res);
	printf("%.16llx ", res);
	_4S_GET(ssx3, res);
	printf("%.16llx ", res);
	_4S_GET(ssx4, res);
	printf("%.16llx\n", res);
}
