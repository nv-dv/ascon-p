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
	asm volatile("vpermq ymm3, ymm3, 0x93");  \

#define _ymm3_rotr() \
	/*         0x93='0b00111001' --> 0 3 2 1 */ \
	asm volatile("vpermq ymm3, ymm3, 0x39");  \

#define _ymm4_rotr() \
	asm volatile("vpermq ymm4, ymm4, 0x39");  \

#define _ymm13_rotl() \
	/*         0x93='0b10010011' --> 2 1 0 3 */ \
	asm volatile("vpermq ymm13, ymm13, 0x93");  \

#define _ymm13_rotr() \
	/*         0x93='0b00111001' --> 0 3 2 1 */ \
	asm volatile("vpermq ymm13, ymm13, 0x39");  \

#define _ymm14_rotr() \
	asm volatile("vpermq ymm14, ymm14, 0x39");  \

#define _4S_LOAD(p_share, x, rand) \
	asm volatile("movq xmm0, %0"::"m"(x):);  \
	for (size_t i = 0; i < 3; ++i) {  \
	asm volatile("movq xmm1, %0"::"m"(rand[i]));  \
	asm volatile("xorpd xmm0, xmm1");  \
	p_share.s[i] = rand[i];}  \
	asm volatile("movq %0, xmm0":"=m"(p_share.s[3])::);  \
	

// s0 ^ s1 --> result
#define _4S_GET(p_share, result) \
	asm volatile("movq xmm0, %0"::"m"(p_share.s[0]):);  \
	for (size_t i = 1; i < 4; ++i) {  \
	asm volatile("movq xmm1, %0"::"m"(p_share.s[i]));  \
	asm volatile("xorpd xmm0, xmm1");}  \
	asm volatile("movq %0, xmm0":"=m"(result)::);  \

// xor share1, share2 --> ymm0
#define _4S_XOR(p_share1, p_share2) \
	asm volatile("vmovdqa ymm0, %0"::"m"(p_share1):);  \
	asm volatile("vmovdqa ymm1, %0"::"m"(p_share2):);  \
	asm volatile("vxorpd ymm0, ymm0, ymm1":::);  \

// not with xor 0xff..ff --> ymm0
#define _4S_NOT(p_share) \
	asm volatile("vmovdqa ymm0, %0"::"m"(p_share):);  \
	D[3] = C_not;  \
	asm volatile("vmovdqa ymm1, %0"::"m"((((__m256*)D)[0])):);  \
	asm volatile("vxorpd ymm0, ymm0, ymm1":::);  \

// and of shares 1 and 2 --> res_share
#define _4S_AND1(p_share1, p_share2, res_share, i) \
	/* Inputs refresh */ \
	asm ("vmovdqa ymm4, %0"::"m"((((__m256*)R4)[i])):);  \
	asm ("vmovdqa ymm0, %0"::"m"(p_share1):);  \
	asm ("vmovdqa ymm3, %0"::"m"(p_share2):);  \
	asm ("vxorpd ymm1, ymm1, ymm1":::);  \
	asm ("vxorpd ymm0, ymm0, ymm4":::);  \
	_ymm4_rotr() \
	asm ("vxorpd ymm0, ymm0, ymm4":::);  \
	asm ("vmovdqa ymm4, %0"::"m"((((__m256*)R4)[i+1])):);  \
	/*body of AND gate*/   \
	/* Do 4 times */  \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	/* first refresh */  \
	asm ("vxorpd ymm1, ymm1, ymm4");  \
	_ymm4_rotr() \
	_ymm3_rotr() \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	_ymm3_rotr() \
	_ymm3_rotr() \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	/*Second refresh*/  \
	asm ("vxorpd ymm1, ymm1, ymm4");  \
	_ymm3_rotl() \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	/* Write results */  \
	asm ("vmovdqa %0, ymm1":"=m"(res_share)::);  \

#define _4S_AND2(p_share1, p_share2, res_share, i) \
	/* Inputs refresh */ \
	asm ("vmovdqa ymm14, %0"::"m"((((__m256*)R4)[i])):);  \
	asm ("vmovdqa ymm10, %0"::"m"(p_share1):);  \
	asm ("vmovdqa ymm13, %0"::"m"(p_share2):);  \
	asm ("vxorpd ymm11, ymm11, ymm11":::);  \
	asm ("vxorpd ymm10, ymm10, ymm14":::);  \
	_ymm14_rotr() \
	asm ("vxorpd ymm10, ymm10, ymm14":::);  \
	asm ("vmovdqa ymm14, %0"::"m"((((__m256*)R4)[i+1])):);  \
	/*body of AND gate*/   \
	/* Do 4 times */  \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	/* first refresh */  \
	asm ("vxorpd ymm11, ymm11, ymm14");  \
	_ymm14_rotr() \
	_ymm13_rotr() \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	_ymm13_rotr() \
	_ymm13_rotr() \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	/*Second refresh*/  \
	asm ("vxorpd ymm11, ymm11, ymm14");  \
	_ymm13_rotl() \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	/* Write results */  \
	asm ("vmovdqa %0, ymm11":"=m"(res_share)::);  \

// rotate right share by i bits --> xmm0
#define _4S_ROTR(p_share, i) \
	asm volatile("vmovdqa ymm3, %0"::"m"(p_share):);  \
	asm volatile("vmovdqa ymm4, ymm3":::);  \
	asm volatile("vpsrlq ymm3, ymm3, %0"::"i"(i):);  \
	asm volatile("vpsllq ymm4, ymm4, %0"::"i"(64-i):);  \
	asm volatile("vorpd ymm3, ymm3, ymm4":::);  \

#define _4S_LDL1(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa ymm0, %0"::"m"(p_share):);  \
	asm ("vpsllq ymm1, ymm0, %0"::"i"(64-rot1):);  \
	asm ("vpsllq ymm4, ymm0, %0"::"i"(64-rot2):);  \
	asm ("vpsrlq ymm3, ymm0, %0"::"i"(rot2):);  \
	asm ("vpsrlq ymm0, ymm0, %0"::"i"(rot1):);  \
	asm ("vorpd ymm0, ymm0, ymm1":::);  \
	asm ("vorpd ymm3, ymm3, ymm4":::);  \
	asm ("vxorpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm2, ymm2, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, ymm2":"=m"(p_share)::);  \

#define _4S_LDL2(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa ymm5, %0"::"m"(p_share):);  \
	asm ("vpsllq ymm9, ymm5, %0"::"i"(64-rot2):);  \
	asm ("vpsllq ymm6, ymm5, %0"::"i"(64-rot1):);  \
	asm ("vpsrlq ymm8, ymm5, %0"::"i"(rot2):);  \
	asm ("vpsrlq ymm5, ymm5, %0"::"i"(rot1):);  \
	asm ("vorpd ymm5, ymm5, ymm6":::);  \
	asm ("vorpd ymm8, ymm8, ymm9":::);  \
	asm ("vxorpd ymm7, ymm5, ymm8":::);  \
	asm ("vxorpd ymm7, ymm7, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, ymm7":"=m"(p_share)::);  \

#define _4S_LDL3(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa ymm10, %0"::"m"(p_share):);  \
	asm ("vpsllq ymm14, ymm10, %0"::"i"(64-rot2):);  \
	asm ("vpsllq ymm11, ymm10, %0"::"i"(64-rot1):);  \
	asm ("vpsrlq ymm13, ymm10, %0"::"i"(rot2):);  \
	asm ("vpsrlq ymm10, ymm10, %0"::"i"(rot1):);  \
	asm ("vorpd ymm10, ymm10, ymm11":::);  \
	asm ("vorpd ymm13, ymm13, ymm14":::);  \
	asm ("vxorpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm12, ymm12, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, ymm12":"=m"(p_share)::);  \

#define _4S_ROUND(c) \
	randbuf.GetBytes((uint8_t*)R4, sizeof(R4)); \
	/*load state*/ \
	D[3] = c;\
	asm volatile("vmovdqa ymm11, %0"::"m"((((__m256*)D)[0])):);  \
	asm volatile("vmovdqa ymm0, %0"::"m"(ssx0):);  \
	asm volatile("vmovdqa ymm4, %0"::"m"(ssx4):);  \
	asm volatile("vmovdqa ymm2, %0"::"m"(ssx2):);  \
	asm volatile("vmovdqa ymm3, %0"::"m"(ssx3):);  \
	asm volatile("vmovdqa ymm1, %0"::"m"(ssx1):);  \
	D[3] = C_not;  \
	asm volatile("vmovdqa ymm5, %0"::"m"((((__m256*)D)[0])):);  \
	asm volatile("vxorpd ymm2, ymm2, ymm11":::);  /*XOR(ssx2, c)*/  \
	asm volatile("vxorpd ymm0, ymm4, ymm0":::);  /*XOR(ssx0, ssx4)*/  \
	asm volatile("vxorpd ymm4, ymm4, ymm3":::);  /*XOR(ssx4, ssx3)*/  \
	asm volatile("vxorpd ymm2, ymm2, ymm1":::);  /*XOR(ssx2, ssx1)*/  \
	asm volatile("vxorpd ymm11, ymm1, ymm5":::);  /*sst1 = NOT(ssx1)*/  \
	asm volatile("vxorpd ymm13, ymm3, ymm5":::);  /*sst3 = NOT(ssx3)*/  \
	asm volatile("vxorpd ymm10, ymm0, ymm5":::);  /*sst0 = NOT(ssx0)*/  \
	asm volatile("vxorpd ymm14, ymm4, ymm5":::);  /*sst4 = NOT(ssx4)*/  \
	asm volatile("vxorpd ymm12, ymm2, ymm5":::);  /*sst2 = NOT(ssx2)*/ \
	/* write back state*/ \
	asm volatile("vmovdqa %0, ymm0":"=m"(ssx0)::);  \
	asm volatile("vmovdqa %0, ymm2":"=m"(ssx2)::);  \
	asm volatile("vmovdqa %0, ymm4":"=m"(ssx4)::);  \
	asm volatile("vmovdqa %0, ymm10":"=m"(sst0)::);  \
	asm volatile("vmovdqa %0, ymm11":"=m"(sst1)::);  \
	asm volatile("vmovdqa %0, ymm12":"=m"(sst2)::);  \
	asm volatile("vmovdqa %0, ymm13":"=m"(sst3)::);  \
	asm volatile("vmovdqa %0, ymm14":"=m"(sst4)::);  \
	/*LOAD Random*/\
	_4S_AND1(sst0, ssx1, sst0 ,0) \
	_4S_AND2(sst1, ssx2, sst1, 2) \
	_4S_AND1(sst2, ssx3, sst2, 4) \
	_4S_AND2(sst3, ssx4, sst3, 6) \
	_4S_AND1(sst4, ssx0, sst4, 8) \
	/*load state*/ \
	asm volatile("vmovdqa ymm0, %0"::"m"(ssx0):);  \
	asm volatile("vmovdqa ymm11, %0"::"m"(sst1):);  \
	asm volatile("vmovdqa ymm1, %0"::"m"(ssx1):);  \
	asm volatile("vmovdqa ymm12, %0"::"m"(sst2):);  \
	asm volatile("vmovdqa ymm2, %0"::"m"(ssx2):);  \
	asm volatile("vmovdqa ymm13, %0"::"m"(sst3):);  \
	asm volatile("vmovdqa ymm3, %0"::"m"(ssx3):);  \
	asm volatile("vmovdqa ymm14, %0"::"m"(sst4):);  \
	asm volatile("vmovdqa ymm4, %0"::"m"(ssx4):);  \
	asm volatile("vmovdqa ymm10, %0"::"m"(sst0):);  \
	asm volatile("vmovdqa ymm5, %0"::"m"((((__m256*)D)[0])):);  \
	asm volatile("vxorpd ymm0, ymm0, ymm11":::);  /*XOR(ssx0, sst1)*/  \
	asm volatile("vxorpd ymm1, ymm1, ymm12":::);  /*XOR(ssx1, sst2)*/  \
	asm volatile("vxorpd ymm2, ymm2, ymm13":::);  /*XOR(ssx2, sst3)*/  \
	asm volatile("vxorpd ymm3, ymm3, ymm14":::);  /*XOR(ssx3, sst4)*/  \
	asm volatile("vxorpd ymm4, ymm4, ymm10":::);  /*XOR(ssx4, sst0)*/  \
	asm volatile("vxorpd ymm1, ymm1, ymm0":::);  /*XOR(ssx1, ssx0)*/  \
	asm volatile("vxorpd ymm0, ymm0, ymm4":::);  /*XOR(ssx0, ssx4)*/  \
	asm volatile("vxorpd ymm3, ymm3, ymm2":::);  /*XOR(ssx3, ssx2)*/  \
	asm volatile("vxorpd ymm2, ymm2, ymm5":::);  /*NOT(ssx2)*/  \
	/*write back state*/ \
	asm volatile("vmovdqa %0, ymm4":"=m"(ssx4)::);  \
	asm volatile("vmovdqa %0, ymm0":"=m"(ssx0)::);  \
	asm volatile("vmovdqa %0, ymm1":"=m"(ssx1)::);  \
	asm volatile("vmovdqa %0, ymm3":"=m"(ssx3)::);  \
	asm volatile("vmovdqa %0, ymm2":"=m"(ssx2)::);  \
	_4S_LDL1(ssx0, 19, 28) \
	_4S_LDL2(ssx1, 39, 61) \
	_4S_LDL3(ssx2, 1, 6) \
	_4S_LDL2(ssx3, 10, 17) \
	_4S_LDL1(ssx4, 7, 41) \
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
