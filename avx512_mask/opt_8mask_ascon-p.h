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

__m512i DD = {0,0,0,0,0,0,0,0};
__m512i E = {0,0,0,0,0,0,0,0};
// normal mask is 0 1 2 3 4 5 6 7
__m512i vec_ror = {7, 0, 1, 2, 3, 4, 5, 6};
__m512i vec_rol = {1, 2, 3, 4, 5, 6, 7, 0};

share_8 sssx0;
share_8 sssx1;
share_8 sssx2;
share_8 sssx3;
share_8 sssx4;
share_8 ssst0;
share_8 ssst1;
share_8 ssst2;
share_8 ssst3;
share_8 ssst4;

__m512i RandIn[5*3];
uint64_t SR8[5*3];

#define _zmm_rotr(a) _mm512_permutexvar_epi64(vec_ror, a)
#define _zmm_rotl(a) _mm512_permutexvar_epi64(vec_rol, a)

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

inline void _8S_AND(share8* share0, share8* share1, share8* res_share, int i, int j) {
	__m512i x = _mm512_loadu_si512(share0);
	__m512i y = _mm512_loadu_si512(share1);
	__m512i z2, res;
	__m512i r = _mm512_loadu_si512(&RandIn[i]);
	__m512i rtag = _mm512_loadu_si512(&RandIn[i+1]);
	// input refresh
	__m512i refr = _mm512_loadu_si512(&RandIn[i+2]);
	E[0] = SR8[j]; E[1] = SR8[j+1]; E[2] = SR8[j+2]; E[4] = SR8[j]; E[5] = SR8[j+1]; E[6] = SR8[j+2];
	x = _mm512_xor_epi64(x, refr);
	refr = _zmm_rotr(refr);
	x = _mm512_xor_epi64(x, refr);
	x = _mm512_xor_epi64(x, E);
	// x.y
	z2 = _mm512_and_epi64(x, y);
	z2 =  _mm512_xor_epi64(z2, r);
	r = _zmm_rotr(r);
	res = _mm512_xor_epi64(res, z2);
	y = _zmm_rotr(y);
	// x.rot(y,1)
	z2 = _mm512_and_epi64(x, y);
	res = _mm512_xor_epi64(res, z2);
	y = _zmm_rotr(y);
	x = _zmm_rotr(x);
	// rot(x,1).y
	z2 = _mm512_and_epi64(x, _mm512_loadu_si512(share1));
	z2 =  _mm512_xor_epi64(z2, r);
	res = _mm512_xor_epi64(res, z2);
	x = _zmm_rotl(x);
	// x.rot(y,2)
	z2 = _mm512_and_epi64(x, y);
	res = _mm512_xor_epi64(res, z2);
	y = _zmm_rotr(y);
	x = _zmm_rotr(x);
	x = _zmm_rotr(x);
	// rot(x,2).y
	z2 = _mm512_and_epi64(x, _mm512_loadu_si512(share1));
	z2 =  _mm512_xor_epi64(z2, rtag);
	rtag = _zmm_rotr(rtag);
	res = _mm512_xor_epi64(res, z2);
	x = _zmm_rotl(x);
	x = _zmm_rotl(x);
	// x.rot(y,3)
	z2 = _mm512_and_epi64(x, y);
	res = _mm512_xor_epi64(res, z2);
	y = _zmm_rotr(y);
	x = _zmm_rotr(x);
	x = _zmm_rotr(x);
	x = _zmm_rotr(x);
	// rot(x,3).y
	z2 = _mm512_and_epi64(x, _mm512_loadu_si512(share1));
	z2 = _mm512_xor_epi64(z2, rtag);
	res = _mm512_xor_epi64(res, z2);
	x = _zmm_rotl(x);
	x = _zmm_rotl(x);
	x = _zmm_rotl(x);
	//x.rot(y,4)
	z2 = _mm512_and_epi64(x, y);
	res = _mm512_xor_epi64(res, z2);
	// write out
	_mm512_storeu_si512(res_share, res);
}

inline void _8S_LDL(share8* share, int rot1, int rot2) {
	__m512i z0 = _mm512_loadu_si512(share);
	__m512i z1 = _mm512_ror_epi64(z0, rot1);
	__m512i z2 = _mm512_ror_epi64(z0, rot2);
	__m512i z3 = _mm512_xor_epi64(z0, z1);
	__m512i z4 = _mm512_xor_epi64(z2, z3);
	_mm512_storeu_si512(share, z4);
}

inline void Get8Shares() {
	uint64_t res;
	_8S_GET(sssx0, res);
	printf("%.16llx ", res);
	_8S_GET(sssx1, res);
	printf("%.16llx ", res);
	_8S_GET(sssx2, res);
	printf("%.16llx ", res);
	_8S_GET(sssx3, res);
	printf("%.16llx ", res);
	_8S_GET(sssx4, res);
	printf("%.16llx\n", res);
}

void _8S_ROUND(uint64_t c) {
	/*LOAD Random*/
	randbuf.GetBytes((uint8_t*)RandIn, sizeof(RandIn));
	randbuf.GetBytes((uint8_t*)SR8, sizeof(SR8));
	/*load state*/
	DD[7] = c;
	__m512i z11 = (DD);
	__m512i z0 = _mm512_loadu_si512(&sssx0);
	__m512i z1 = _mm512_loadu_si512(&sssx1);
	__m512i z2 = _mm512_loadu_si512(&sssx2);
	__m512i z3 = _mm512_loadu_si512(&sssx3);
	__m512i z4 = _mm512_loadu_si512(&sssx4);
	DD[7] = C_not;
	//x0 ^= x4; //1 x4 ^= x3;    x2 ^= x1;
	z2 = _mm512_xor_epi64(z2, z11);
	z0 = _mm512_xor_epi64(z0, z4);
	z4 = _mm512_xor_epi64(z4, z3);
	z2 = _mm512_xor_epi64(z2, z1);
	//t0 = ~x0;    t1 = ~x1;    t2 = ~x2;    t3 = ~x3;    t4 = ~x4;
	z11 = _mm512_xor_epi64(z1, DD);
	__m512i z13 = _mm512_xor_epi64(z3, DD);
	__m512i z10 = _mm512_xor_epi64(z0, DD);
	__m512i z14 = _mm512_xor_epi64(z4, DD);
	__m512i z12 = _mm512_xor_epi64(z2, DD);
	/* write back state*/
	_mm512_storeu_si512(&sssx0, z0);
	_mm512_storeu_si512(&sssx2, z2);
	_mm512_storeu_si512(&sssx4, z4);
	_mm512_storeu_si512(&ssst0, z10);
	_mm512_storeu_si512(&ssst1, z11);
	_mm512_storeu_si512(&ssst2, z12);
	_mm512_storeu_si512(&ssst3, z13);
	_mm512_storeu_si512(&ssst4, z14);
	_8S_AND(&ssst0, &sssx1, &ssst0, 0,0);
	_8S_AND(&ssst1, &sssx2, &ssst1, 3,3);
	_8S_AND(&ssst2, &sssx3, &ssst2, 6,6);
	_8S_AND(&ssst3, &sssx4, &ssst3, 9,9);
	_8S_AND(&ssst4, &sssx0, &ssst4, 12,12);
	/*load state*/
	DD[7] = C_not;
	z0 = _mm512_loadu_si512(&sssx0);
	z1 = _mm512_loadu_si512(&sssx1);
	z2 = _mm512_loadu_si512(&sssx2);
	z3 = _mm512_loadu_si512(&sssx3);
	z4 = _mm512_loadu_si512(&sssx4);
	z10 = _mm512_loadu_si512(&ssst0);
	z11 = _mm512_loadu_si512(&ssst1);
	z12 = _mm512_loadu_si512(&ssst2);
	z13 = _mm512_loadu_si512(&ssst3);
	z14 = _mm512_loadu_si512(&ssst4);
	//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
	z0 = _mm512_xor_epi64(z0, z11);
	z1 = _mm512_xor_epi64(z1, z12);
	z2 = _mm512_xor_epi64(z2, z13);
	z3 = _mm512_xor_epi64(z3, z14);
	z4 = _mm512_xor_epi64(z4, z10);
	//x1 ^= x0;    x0 ^= x4;    x3 ^= x2;    x2 = ~x2;
	z1 = _mm512_xor_epi64(z1, z0);
	z0 = _mm512_xor_epi64(z4, z0);
	z3 = _mm512_xor_epi64(z3, z2);
	z2 = _mm512_xor_epi64(z2, DD);
	/*write back state*/
	_mm512_storeu_si512(&sssx4, z4);
	_mm512_storeu_si512(&sssx0, z0);
	_mm512_storeu_si512(&sssx1, z1);
	_mm512_storeu_si512(&sssx3, z3);
	_mm512_storeu_si512(&sssx2, z2);
	_8S_LDL(&sssx0, 19, 28);
	_8S_LDL(&sssx1, 39, 61);
	_8S_LDL(&sssx2, 1, 6);
	_8S_LDL(&sssx3, 10, 17);
	_8S_LDL(&sssx4, 7, 41);
	/*end*/
	}

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
	_8S_LOAD(sssx0, tmp, rand);
	tmp = state[1];
	rand += 7;
	_8S_LOAD(sssx1, tmp, rand);
	tmp = state[2];
	rand += 7;
	_8S_LOAD(sssx2, tmp, rand);
	tmp = state[3];
	rand += 7;
	_8S_LOAD(sssx3, tmp, rand);
	tmp = state[4];
	rand += 7;
	_8S_LOAD(sssx4, tmp, rand);
}

