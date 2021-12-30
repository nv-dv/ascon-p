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

typedef struct share8
{
	uint64_t s[8];
} share_8;

#ifndef CONSTS
	#define CONSTS
	uint64_t C_not = 0xffffffffffffffff;
	uint64_t C;
	RandomBuffer randbuf = RandomBuffer(500 * 2<<20);
	#define TYPE_UINT64 8
	#define TYPE_UINT128 16
	#define TYPE_UINT256 32
#endif

uint64_t D[8] = {0,0,0,0,0,0,0,0};

uint64_t E[8] = {0,0,0,0,0,0,0,0};

uint64_t R8[5*8*4];
uint64_t SR8[5*6];
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

// zmm3 >> 64 --> zmm3

#define _zmm0_rotr() \
	/*         0x1F58D1='0b000111110101100011010001' --> 0 7 6 5 4 3 2 1 */ \
	asm volatile("vpermq zmm0, zmm0, 0x1F58D1");  \

#define _zmm0_rotl() \
	/*         0xD63447='0b110101100011010001000111' --> 6 5 4 3 2 1 0 7 */ \
	asm volatile("vpermq zmm0, zmm0, 0xD63447");  \

#define _zmm3_rotr() \
	asm volatile("vpermq zmm3, zmm3, 0x1F58D1");  \

#define _zmm3_rotl() \
	asm volatile("vpermq zmm3, zmm3, 0xD63447");  \

#define _zmm4_rotr() \
	asm volatile("vpermq zmm4, zmm4, 0x1F58D1");  \

#define _zmm6_rotr() \
	asm volatile("vpermq zmm6, zmm6, 0x1F58D1");  \

#define _zmm7_rotr() \
	asm volatile("vpermq zmm6, zmm6, 0x1F58D1");  \

#define _zmm8_rotr() \
	asm volatile("vpermq zmm6, zmm6, 0x1F58D1");  \

#define _zmm13_rotl() \
	asm volatile("vpermq zmm13, zmm13, 0x93");  \

#define _zmm13_rotr() \
	asm volatile("vpermq zmm13, zmm13, 0x39");  \

#define _zmm14_rotr() \
	asm volatile("vpermq zmm14, zmm14, 0x39");  \

#define _8S_LOAD(p_share, x, rand) \
	asm volatile("movq xmm0, %0"::"m"(x):);  \
	for (size_t i = 0; i < 8-1; ++i) {  \
	asm volatile("movq xmm1, %0"::"m"(rand[i]));  \
	asm volatile("xorpd xmm0, xmm1");  \
	p_share.s[i] = rand[i];}  \
	asm volatile("movq %0, xmm0":"=m"(p_share.s[7])::);  \
	

// s0 ^ s1 --> result
#define _8S_GET(p_share, result) \
	asm volatile("movq xmm0, %0"::"m"(p_share.s[0]):);  \
	for (size_t i = 1; i < 8; ++i) {  \
	asm volatile("movq xmm1, %0"::"m"(p_share.s[i]));  \
	asm volatile("xorpd xmm0, xmm1");}  \
	asm volatile("movq %0, xmm0":"=m"(result)::);  \

// xor share1, share2 --> zmm0
#define _8S_XOR(p_share1, p_share2) \
	asm volatile("vmovdqa zmm0, %0"::"m"(p_share1):);  \
	asm volatile("vmovdqa zmm1, %0"::"m"(p_share2):);  \
	asm volatile("vxorpd zmm0, zmm0, zmm1":::);  \

// not with xor 0xff..ff --> zmm0
#define _8S_NOT(p_share) \
	asm volatile("vmovdqa zmm0, %0"::"m"(p_share):);  \
	D[7] = C_not;  \
	asm volatile("vmovdqa zmm1, %0"::"m"((((__m512*)D)[0])):);  \
	asm volatile("vxorpd zmm0, zmm0, zmm1":::);  \

// and of shares 1 and 2 --> res_share
#define _8S_AND1(p_share1, p_share2, res_share, i, j) \
	/* Inputs refresh */ \
	asm ("vmovdqa zmm0, %0"::"m"(p_share1):);  \
	asm ("vmovdqa zmm4, %0"::"m"((((__m512*)R8)[i])):);  \
	asm ("vmovdqa zmm6, %0"::"m"((((__m512*)R8)[i+1])):);  \
	asm ("vmovdqa zmm7, %0"::"m"((((__m512*)R8)[i+2])):);  \
	asm ("vmovdqa zmm8, %0"::"m"((((__m512*)R8)[i+3])):);  \
	asm ("vmovdqa zmm3, %0"::"m"(p_share2):);  \
	E[0] = SR8[j]; E[1] = SR8[j+1]; E[2] = SR8[j+2]; E[4] = SR8[j+3]; E[5] = SR8[j+4]; E[1] = SR8[j+5]; \
	asm ("vxorpd zmm1, zmm1, zmm1":::);  \
	asm ("vxorpd zmm0, zmm0, zmm4":::);  \
	_zmm4_rotr() \
	asm ("vxorpd zmm0, zmm0, zmm4":::);  \
	/* 2nd round of refresh */ \
	asm ("vmovdqa zmm5, %0"::"m"((__m512*)E));
	asm ("vxorpd zmm0, zmm0, zmm5":::);  \
	/*body of AND gate*/   \
	/* Do 8 times */  \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	/* first refresh r*/  \
	asm ("vxorpd zmm1, zmm1, zmm6");  \
	_zmm3_rotr() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	_zmm3_rotl() \
	_zmm6_rotr() \
	_zmm0_rotr() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	asm ("vxorpd zmm1, zmm1, zmm6":::);  \
	_zmm0_rotl() \
	_zmm3_rotr() \
	_zmm3_rotr() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	_zmm0_rotr() \
	_zmm0_rotr() \
	_zmm3_rotl() \
	_zmm3_rotl() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	/*Second refresh*/  \
	asm ("vxorpd zmm1, zmm1, zmm7");  \
	_zmm7_rotr() \
	_zmm0_rotl() \
	_zmm0_rotl() \
	_zmm3_rotr() \
	_zmm3_rotr() \
	_zmm3_rotr() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	_zmm0_rotr() \
	_zmm0_rotr() \
	_zmm0_rotr() \
	_zmm3_rotl() \
	_zmm3_rotl() \
	_zmm3_rotl() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	asm ("vxorpd zmm1, zmm1, zmm7":::);  \
	asm ("vmovdqa zmm0, %0"::"m"(p_share1):);  \
	_zmm3_rotr() \
	_zmm3_rotr() \
	_zmm3_rotr() \
	_zmm3_rotr() \
	asm ("vandpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm1, zmm1, zmm2":::);  \
	/*Third refresh*/  \
	asm ("vxorpd zmm1, zmm1, zmm8":::);  \
	_zmm8_rotr() \
	asm ("vxorpd zmm1, zmm1, zmm8":::);  \
	/* Write results */  \
	asm ("vmovdqa %0, zmm1":"=m"(res_share)::);  \

// rotate right share by i bits --> xmm0
#define _8S_ROTR(p_share, i) \
	asm volatile("vmovdqa zmm3, %0"::"m"(p_share):);  \
	asm volatile("vmovdqa zmm4, zmm3":::);  \
	asm volatile("vpsrlq zmm3, zmm3, %0"::"i"(i):);  \
	asm volatile("vpsllq zmm4, zmm4, %0"::"i"(64-i):);  \
	asm volatile("vorpd zmm3, zmm3, zmm4":::);  \

#define _8S_LDL1(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa zmm0, %0"::"m"(p_share):);  \
	asm ("vpsllq zmm1, zmm0, %0"::"i"(64-rot1):);  \
	asm ("vpsllq zmm4, zmm0, %0"::"i"(64-rot2):);  \
	asm ("vpsrlq zmm3, zmm0, %0"::"i"(rot2):);  \
	asm ("vpsrlq zmm0, zmm0, %0"::"i"(rot1):);  \
	asm ("vorpd zmm0, zmm0, zmm1":::);  \
	asm ("vorpd zmm3, zmm3, zmm4":::);  \
	asm ("vxorpd zmm2, zmm0, zmm3":::);  \
	asm ("vxorpd zmm2, zmm2, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, zmm2":"=m"(p_share)::);  \

#define _8S_LDL2(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa zmm5, %0"::"m"(p_share):);  \
	asm ("vpsllq zmm9, zmm5, %0"::"i"(64-rot2):);  \
	asm ("vpsllq zmm6, zmm5, %0"::"i"(64-rot1):);  \
	asm ("vpsrlq zmm8, zmm5, %0"::"i"(rot2):);  \
	asm ("vpsrlq zmm5, zmm5, %0"::"i"(rot1):);  \
	asm ("vorpd zmm5, zmm5, zmm6":::);  \
	asm ("vorpd zmm8, zmm8, zmm9":::);  \
	asm ("vxorpd zmm7, zmm5, zmm8":::);  \
	asm ("vxorpd zmm7, zmm7, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, zmm7":"=m"(p_share)::);  \

#define _8S_LDL3(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa zmm10, %0"::"m"(p_share):);  \
	asm ("vpsllq zmm14, zmm10, %0"::"i"(64-rot2):);  \
	asm ("vpsllq zmm11, zmm10, %0"::"i"(64-rot1):);  \
	asm ("vpsrlq zmm13, zmm10, %0"::"i"(rot2):);  \
	asm ("vpsrlq zmm10, zmm10, %0"::"i"(rot1):);  \
	asm ("vorpd zmm10, zmm10, zmm11":::);  \
	asm ("vorpd zmm13, zmm13, zmm14":::);  \
	asm ("vxorpd zmm12, zmm10, zmm13":::);  \
	asm ("vxorpd zmm12, zmm12, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, zmm12":"=m"(p_share)::);  \

#define _8S_ROUND(c) \
	randbuf.GetBytes((uint8_t*)R8, sizeof(R8)); \
	randbuf.GetBytes((uint8_t*)SR8, sizeof(SR8)); \
	/*load state*/ \
	D[7] = c;\
	asm volatile("vmovdqa zmm11, %0"::"m"((((__m512*)D)[0])):);  \
	asm volatile("vmovdqa zmm0, %0"::"m"(ssx0):);  \
	asm volatile("vmovdqa zmm4, %0"::"m"(ssx4):);  \
	asm volatile("vmovdqa zmm2, %0"::"m"(ssx2):);  \
	asm volatile("vmovdqa zmm3, %0"::"m"(ssx3):);  \
	asm volatile("vmovdqa zmm1, %0"::"m"(ssx1):);  \
	D[7] = C_not;  \
	asm volatile("vmovdqa zmm5, %0"::"m"((((__m512*)D)[0])):);  \
	asm volatile("vxorpd zmm2, zmm2, zmm11":::);  /*XOR(ssx2, c)*/  \
	asm volatile("vxorpd zmm0, zmm4, zmm0":::);  /*XOR(ssx0, ssx4)*/  \
	asm volatile("vxorpd zmm4, zmm4, zmm3":::);  /*XOR(ssx4, ssx3)*/  \
	asm volatile("vxorpd zmm2, zmm2, zmm1":::);  /*XOR(ssx2, ssx1)*/  \
	asm volatile("vxorpd zmm11, zmm1, zmm5":::);  /*sst1 = NOT(ssx1)*/  \
	asm volatile("vxorpd zmm13, zmm3, zmm5":::);  /*sst3 = NOT(ssx3)*/  \
	asm volatile("vxorpd zmm10, zmm0, zmm5":::);  /*sst0 = NOT(ssx0)*/  \
	asm volatile("vxorpd zmm14, zmm4, zmm5":::);  /*sst4 = NOT(ssx4)*/  \
	asm volatile("vxorpd zmm12, zmm2, zmm5":::);  /*sst2 = NOT(ssx2)*/ \
	/* write back state*/ \
	asm volatile("vmovdqa %0, zmm0":"=m"(ssx0)::);  \
	asm volatile("vmovdqa %0, zmm2":"=m"(ssx2)::);  \
	asm volatile("vmovdqa %0, zmm4":"=m"(ssx4)::);  \
	asm volatile("vmovdqa %0, zmm10":"=m"(sst0)::);  \
	asm volatile("vmovdqa %0, zmm11":"=m"(sst1)::);  \
	asm volatile("vmovdqa %0, zmm12":"=m"(sst2)::);  \
	asm volatile("vmovdqa %0, zmm13":"=m"(sst3)::);  \
	asm volatile("vmovdqa %0, zmm14":"=m"(sst4)::);  \
	/*LOAD Random*/\
	_8S_AND1(sst0, ssx1, sst0 ,0) \
	_8S_AND1(sst1, ssx2, sst1, 4) \
	_8S_AND1(sst2, ssx3, sst2, 8) \
	_8S_AND1(sst3, ssx4, sst3, 12) \
	_8S_AND1(sst4, ssx0, sst4, 16) \
	/*load state*/ \
	D[7] = C_not; \
	asm volatile("vmovdqa zmm0, %0"::"m"(ssx0):);  \
	asm volatile("vmovdqa zmm11, %0"::"m"(sst1):);  \
	asm volatile("vmovdqa zmm1, %0"::"m"(ssx1):);  \
	asm volatile("vmovdqa zmm12, %0"::"m"(sst2):);  \
	asm volatile("vmovdqa zmm2, %0"::"m"(ssx2):);  \
	asm volatile("vmovdqa zmm13, %0"::"m"(sst3):);  \
	asm volatile("vmovdqa zmm3, %0"::"m"(ssx3):);  \
	asm volatile("vmovdqa zmm14, %0"::"m"(sst4):);  \
	asm volatile("vmovdqa zmm4, %0"::"m"(ssx4):);  \
	asm volatile("vmovdqa zmm10, %0"::"m"(sst0):);  \
	asm volatile("vmovdqa zmm5, %0"::"m"((((__m512*)D)[0])):);  \
	asm volatile("vxorpd zmm0, zmm0, zmm11":::);  /*XOR(ssx0, sst1)*/  \
	asm volatile("vxorpd zmm1, zmm1, zmm12":::);  /*XOR(ssx1, sst2)*/  \
	asm volatile("vxorpd zmm2, zmm2, zmm13":::);  /*XOR(ssx2, sst3)*/  \
	asm volatile("vxorpd zmm3, zmm3, zmm14":::);  /*XOR(ssx3, sst4)*/  \
	asm volatile("vxorpd zmm4, zmm4, zmm10":::);  /*XOR(ssx4, sst0)*/  \
	asm volatile("vxorpd zmm1, zmm1, zmm0":::);  /*XOR(ssx1, ssx0)*/  \
	asm volatile("vxorpd zmm0, zmm0, zmm4":::);  /*XOR(ssx0, ssx4)*/  \
	asm volatile("vxorpd zmm3, zmm3, zmm2":::);  /*XOR(ssx3, ssx2)*/  \
	asm volatile("vxorpd zmm2, zmm2, zmm5":::);  /*NOT(ssx2)*/  \
	/*write back state*/ \
	asm volatile("vmovdqa %0, zmm4":"=m"(ssx4)::);  \
	asm volatile("vmovdqa %0, zmm0":"=m"(ssx0)::);  \
	asm volatile("vmovdqa %0, zmm1":"=m"(ssx1)::);  \
	asm volatile("vmovdqa %0, zmm3":"=m"(ssx3)::);  \
	asm volatile("vmovdqa %0, zmm2":"=m"(ssx2)::);  \
	_8S_LDL1(ssx0, 19, 28) \
	_8S_LDL2(ssx1, 39, 61) \
	_8S_LDL3(ssx2, 1, 6) \
	_8S_LDL2(ssx3, 10, 17) \
	_8S_LDL1(ssx4, 7, 41) \
	/*end*/ \

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
    _8S_ROUND(0x4b);\


#define _8S_P6 \
    _8S_ROUND(0x96);\
    _8S_ROUND(0x87);\
    _8S_ROUND(0x78);\
    _8S_ROUND(0x69);\
    _8S_ROUND(0x5a);\
    _8S_ROUND(0x4b);\


#define _8S_P1 \
    _8S_ROUND(0x4b);\

inline void Init8Shares(uint64_t state[], uint64_t rand[]) {
	uint64_t tmp = state[0];
	_8S_LOAD(ssx0, tmp, rand);
	tmp = state[1];
	rand += 7;
	_8S_LOAD(ssx1, tmp, rand);
	tmp = state[2];
	rand += 7;
	_8S_LOAD(ssx2, tmp, rand);
	tmp = state[3];
	rand += 7;
	_8S_LOAD(ssx3, tmp, rand);
	tmp = state[4];
	rand += 7;
	_8S_LOAD(ssx4, tmp, rand);
}

inline void Get8Shares() {
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
