#pragma once
#include<stdio.h>
#include<cstdint>
#include "../consts.h"

typedef struct dshare_t
{
	uint64_t s[MASKING_ORDER];
} dshare;
#include "dmask_AND_gate.h"

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

#ifndef CONSTS
	#define CONSTS
	uint64_t C_not = 0xffffffffffffffff;
	uint64_t C;
	RandomBuffer randbuf = RandomBuffer(500 * 2<<20);
	#define TYPE_UINT64 8
	#define TYPE_UINT128 16
	#define TYPE_UINT256 32
#endif

dshare d_share_x[5];
dshare d_share_t[5];

#define uint64_rotr(x,n) (((x)>>(n))|((x)<<(64-(n))))

inline void dS_LOAD(dshare* p_share, uint64_t x, uint64_t rand[]) {
	uint64_t tmp = x;
	for (int i = 0; i < MASKING_ORDER-1; i++)
	{
		tmp ^= rand[i];
		p_share->s[i] = rand[i];
	}
	p_share->s[MASKING_ORDER - 1] = tmp;
}

static uint64_t dS_GET(dshare* p_share) {
	uint64_t tmp = p_share->s[0];
	for (int i = 1; i < MASKING_ORDER; i++)
	{
		tmp ^= p_share->s[i];
	}
	return tmp;
}

// general dS_AND with refresh
inline dshare dS_AND(dshare* p1, dshare* p2) {
	#ifdef REFRESH
	refresh(p1, p2, &randbuf);
	#endif // REFRESH
	uint64_t tmp;
	dshare ans;
	// matrix mult. with minimal randomness...
	#if defined(OPTIMIZE_SSE2)
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		asm("vxorpd xmm0, xmm0, xmm0");
		asm("vmovq %0, xmm0":"=m"(ans.s[i])::);
	}
	for (size_t i = 0; i < MASKING_ORDER; i++)
	{
		asm("vmovq xmm0, %0"::"m"(p1->s[i]):);
		asm("vmovq xmm1, %0"::"m"(p2->s[i]):);
		asm("andpd xmm0, xmm1");
		asm("vmovq xmm1, %0"::"m"(ans.s[i]):);
		asm("xorpd xmm0, xmm1");
		asm("vmovq %0, xmm0":"=m"(ans.s[i])::);
		for (size_t j = i+1; j < MASKING_ORDER; j++)
		{
			tmp = randbuf.GetQWORD();
			asm("vmovq xmm0, %0"::"m"(tmp):);
			asm("vmovq xmm1, %0"::"m"(ans.s[i]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[i])::);

			asm("vmovq xmm0, %0"::"m"(p1->s[i]):);
			asm("vmovq xmm1, %0"::"m"(p2->s[j]):);
			asm("andpd xmm0, xmm1");
			asm("vmovq xmm1, %0"::"m"(ans.s[i]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[i])::);

			asm("vmovq xmm0, %0"::"m"(tmp):);
			asm("vmovq xmm1, %0"::"m"(ans.s[j]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[j])::);
			
			asm("vmovq xmm0, %0"::"m"(p1->s[j]):);
			asm("vmovq xmm1, %0"::"m"(p2->s[i]):);
			asm("andpd xmm0, xmm1");
			asm("vmovq xmm1, %0"::"m"(ans.s[j]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[j])::);
		}
	}
	#else
	// no SIMD optimizations...
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		ans.s[i] = 0;
	}
	for (size_t i = 0; i < MASKING_ORDER; i++)
	{
		ans.s[i] ^= p1->s[i] & p2->s[i];
		for (size_t j = i+1; j < MASKING_ORDER; j++)
		{
			tmp = randbuf.GetQWORD();
			ans.s[i] ^= tmp;
			ans.s[i] ^= p1->s[i] & p2->s[j];
			ans.s[j] ^= tmp;
			ans.s[j] ^= p1->s[j] & p2->s[i];
		}
	}
	#endif
	return ans;
}

#define dS_LDL(i, j, rot1, rot2) \
		t[j].s[i] = x[j].s[i]; \
		rot[j].s[i] = uint64_rotr(x[j].s[i], rot1); \
		t[j].s[i] ^= rot[j].s[i]; \
		rot[j].s[i] = uint64_rotr(x[j].s[i], rot2); \
		x[j].s[i] = t[j].s[i] ^ rot[j].s[i]; \

inline void dS_ROUND(int C) {
	dshare* x = d_share_x;
	dshare* t = d_share_t;
	dshare rot[5];
	//x2 ^= c
	x[2].s[0] ^= C;
	//x0 ^= x4;     x4 ^= x3;    x2 ^= x1;
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		x[0].s[i] ^= x[4].s[i];
		x[4].s[i] ^= x[3].s[i];
		x[2].s[i] ^= x[1].s[i];
	}
	//t0 = ~x0;    t1 = ~x1;    t2 = ~x2;    t3 = ~x3;    t4 = ~x4;
	t[0].s[0] = x[0].s[0] ^ C_not;
	t[1].s[0] = x[1].s[0] ^ C_not;
	t[2].s[0] = x[2].s[0] ^ C_not;
	t[3].s[0] = x[3].s[0] ^ C_not;
	t[4].s[0] = x[4].s[0] ^ C_not;
	for (int j = 1; j < MASKING_ORDER; j++)
	{
		t[0].s[j] = x[0].s[j];
		t[1].s[j] = x[1].s[j];
		t[2].s[j] = x[2].s[j];
		t[3].s[j] = x[3].s[j];
		t[4].s[j] = x[4].s[j];
	}
		
	//t0 &= x1;    t1 &= x2;    t2 &= x3;    t3 &= x4;    t4 &= x0;
	int tmp;
	for (int i = 0; i < 5; i++)
	{
		tmp = ((i + 1) % 5);
		t[i] = dS_AND(t + i, x + tmp);
	}
	//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
	for (int i = 0; i < 5; i++)
	{
		tmp = ((i + 1) % 5);
		for (size_t j = 0; j<MASKING_ORDER; j++) {
			x[i].s[j] ^= t[tmp].s[j];
		}
	}
	//x1 ^= x0;    x0 ^= x4;    x3 ^= x2;    x2 = ~x2;
	for (size_t i=0; i<MASKING_ORDER; i++) {
		x[1].s[i] ^= x[0].s[i];
		x[0].s[i] ^= x[4].s[i];
		x[3].s[i] ^= x[2].s[i];
	}
	x[2].s[0] ^= C_not;
	// linear layer
	for (int i = 0; i < MASKING_ORDER; i++) {
		dS_LDL(i, 0, 19, 28);
		dS_LDL(i, 1, 39, 61);
		dS_LDL(i, 2, 1, 6);
		dS_LDL(i, 3, 10, 17);
		dS_LDL(i, 4, 7, 41);
	}
	// end
}
	
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
    dS_ROUND(0x4b);\


#define dS_P6 \
    dS_ROUND(0x96);\
    dS_ROUND(0x87);\
    dS_ROUND(0x78);\
    dS_ROUND(0x69);\
    dS_ROUND(0x5a);\
    dS_ROUND(0x4b);\


#define dS_P1 \
    dS_ROUND(0x4b);\

inline void InitDShares(uint64_t x[], uint64_t rand[]) {
	for (int i = 0; i < 5; i++)
	{
		dS_LOAD(d_share_x + i, x[i], rand);
		rand += MASKING_ORDER-1;
	}
}

inline void GetDShares() {
	uint64_t ans[5];
	for (int i = 0; i < 5; i++)
	{
		ans[i] = dS_GET(d_share_x + i);
		printf("%.16llx ", ans[i]);
	}
	printf("\n");
}