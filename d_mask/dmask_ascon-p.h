#pragma once
#include<stdio.h>
#include<cstdint>
#include "../consts.h"
#include <cstring>

#define CEILING(x,y) (((x) + (y) - 1) / (y))

#if defined(UMA_AND)
#define AND_RAND_COUNT (MASKING_ORDER)*CEILING(MASKING_ORDER-1, 4)
#define AND dS_UMA_AND
#else
#define AND_RAND_COUNT (MASKING_ORDER)*(MASKING_ORDER-1) / 2
#define AND dS_AND
#endif

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
	RandomBuffer randbuf = RandomBuffer(800 * 2<<20);
	#define TYPE_UINT64 8
	#define TYPE_UINT128 16
	#define TYPE_UINT256 32
#endif

dshare d_share_x[5];
dshare d_share_t[5];
uint64_t Rd[5*AND_RAND_COUNT];

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

inline dshare operator>>(dshare &share, int rot) {
	dshare ans;
	for (size_t i = 0; i<MASKING_ORDER; ++i) {
		ans.s[(i+rot)%MASKING_ORDER] = share.s[i];
	}
	return ans;
}
inline dshare& operator^=(dshare &self, dshare &other) {
	for (size_t i=0;i<MASKING_ORDER;++i) {
		self.s[i] ^= other.s[i];
	}
	return self;
}

inline dshare operator&(dshare &p1, dshare &p2) {
	dshare res;
	for (size_t i=0;i<MASKING_ORDER;++i) {
		res.s[i] = p1.s[i] & p2.s[i];
	}
	return res;
}

inline dshare dS_UMA_AND(dshare a, dshare b, size_t randindex) {
	#ifdef REFRESH
	refresh(a, b, &randbuf);
	#endif // REFRESH
	dshare ans;
	dshare tmp;
	dshare tmp2;
	dshare ri;
	ans = a & b;
	for (size_t i = 0; i<(MASKING_ORDER-1)>>2; ++i) {
		// load r_i
		for (size_t j=0;j<MASKING_ORDER;j++) {
			ri.s[j] = Rd[randindex++];
		}
		// XORing
		ans ^= ri;
		// ROR ri
		ri = ri>>1;
		tmp2 = b>>(1+2*i); tmp = a & tmp2; ans ^= tmp;
		tmp2 = a>>(1+2*i); tmp = tmp2 & b; ans ^= tmp;
		ans ^= ri;
		tmp2 = b>>(2+2*i); tmp = a & tmp2; ans ^= tmp;
		tmp2 = a>>(2+2*i); tmp = tmp2 & b; ans ^= tmp;
	}
	int l = (MASKING_ORDER-1)>>2;
	if ((MASKING_ORDER-1) % 4 == 3) {
		// load r_i
		for (size_t j=0;j<MASKING_ORDER;j++) {
			ri.s[j] = Rd[randindex++];
		}
		ans ^= ri;
		ri = ri>>1;
		tmp2 = b>>(1+2*l);
		tmp = a & tmp2;
		ans ^= tmp;
		tmp2 = a>>(1+2*l);
		tmp = tmp2 & b;
		ans ^= tmp;
		ans ^= ri;
		tmp2 = b>>(2+2*l);
		tmp = a & tmp2;
		ans ^= tmp;
	}
	else if ((MASKING_ORDER-1) % 4 == 3) {
		if (MASKING_ORDER == 3) {
			//load r_i
			ri.s[0] = Rd[randindex++];
			ri.s[1] = Rd[randindex++];
			ri.s[2] = ri.s[0] ^ ri.s[1];
			ans ^= ri;
			tmp2 = b>>(1+2*l);
			tmp = a & tmp2;
			ans ^= tmp;
			tmp2 = a>>(1+2*l);
			tmp = tmp2 & b;
			ans ^= tmp;
		}
		else {
			// load r_i
			for (size_t j=0;j<MASKING_ORDER;j++) {
				ri.s[j] = Rd[randindex++];
			}
		}
		ans ^= ri;
		tmp2 = b>>(1+2*l);
		tmp = a & tmp2;
		ans ^= tmp;
		ri = ri >> 1;
		tmp2 = b>>(2+2*l);
		tmp = a & tmp2;
		ans ^= tmp;
	}
	else if ((MASKING_ORDER-1)%4==1) {
		// load r_i
		for (size_t j=0;j<MASKING_ORDER/2;j++) {
			ri.s[j] = Rd[randindex++];
		}
		for (size_t j=MASKING_ORDER/2;j<MASKING_ORDER;j++) {
			ri.s[j] = ri.s[j-MASKING_ORDER/2];
		}
		ans ^= ri;
		tmp2 = b>>(1+2*l);
		tmp = a & tmp2;
		ans ^= tmp;
	}
	return ans;
}

// general dS_AND with refresh
inline dshare dS_AND(dshare &p1, dshare &p2, size_t randindex) {
	#ifdef REFRESH
	if (randindex==0) refresh(p1, p2, &randbuf);
	#endif // REFRESH
	dshare ans;
	// matrix mult. with minimal randomness...
	#if !defined(OPTIMIZE_SSE2)
	// no SIMD optimizations...
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		ans.s[i] = 0;
	}
	for (size_t i = 0; i < MASKING_ORDER; i++)
	{
		ans.s[i] ^= p1.s[i] & p2.s[i];
		for (size_t j = i+1; j < MASKING_ORDER; j++)
		{
			ans.s[i] ^= Rd[randindex];
			ans.s[i] ^= p1.s[i] & p2.s[j];
			ans.s[j] ^= Rd[randindex];
			ans.s[j] ^= p1.s[j] & p2.s[i];
			randindex++;
		}
	}
	#else
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		asm("vxorpd xmm0, xmm0, xmm0");
		asm("vmovq %0, xmm0":"=m"(ans.s[i])::);
	}
	for (size_t i = 0; i < MASKING_ORDER; i++)
	{
		asm("vmovq xmm0, %0"::"m"(p1.s[i]):);
		asm("vmovq xmm1, %0"::"m"(p2.s[i]):);
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

			asm("vmovq xmm0, %0"::"m"(p1.s[i]):);
			asm("vmovq xmm1, %0"::"m"(p2.s[j]):);
			asm("andpd xmm0, xmm1");
			asm("vmovq xmm1, %0"::"m"(ans.s[i]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[i])::);

			asm("vmovq xmm0, %0"::"m"(tmp):);
			asm("vmovq xmm1, %0"::"m"(ans.s[j]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[j])::);
			
			asm("vmovq xmm0, %0"::"m"(p1.s[j]):);
			asm("vmovq xmm1, %0"::"m"(p2.s[i]):);
			asm("andpd xmm0, xmm1");
			asm("vmovq xmm1, %0"::"m"(ans.s[j]):);
			asm("xorpd xmm0, xmm1");
			asm("vmovq %0, xmm0":"=m"(ans.s[j])::);
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
	randbuf.GetBytes((uint8_t*)Rd, sizeof(Rd));
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
	t[0].s[0] = ~x[0].s[0];
	t[1].s[0] = ~x[1].s[0];
	t[2].s[0] = ~x[2].s[0];
	t[3].s[0] = ~x[3].s[0];
	t[4].s[0] = ~x[4].s[0];
	for (int i = 1; i < MASKING_ORDER; i++)
	{
		t[0].s[i] = x[0].s[i];
		t[1].s[i] = x[1].s[i];
		t[2].s[i] = x[2].s[i];
		t[3].s[i] = x[3].s[i];
		t[4].s[i] = x[4].s[i];
	}
	//t0 &= x1;    t1 &= x2;    t2 &= x3;    t3 &= x4;    t4 &= x0;
	t[0] = AND(t[0], x[1], 0);
	t[1] = AND(t[1], x[2], AND_RAND_COUNT);
	t[2] = AND(t[2], x[3], AND_RAND_COUNT*2);
	t[3] = AND(t[3], x[4], AND_RAND_COUNT*3);
	t[4] = AND(t[4], x[0], AND_RAND_COUNT*4);
	//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
	for (size_t i = 0; i<MASKING_ORDER; i++) {
		x[0].s[i] ^= t[1].s[i];
		x[1].s[i] ^= t[2].s[i];
		x[2].s[i] ^= t[3].s[i];
		x[3].s[i] ^= t[4].s[i];
		x[4].s[i] ^= t[0].s[i];
	}
	//x1 ^= x0;    x0 ^= x4;    x3 ^= x2;    x2 = ~x2;
	for (size_t i=0; i<MASKING_ORDER; i++) {
		x[1].s[i] ^= x[0].s[i];
		x[0].s[i] ^= x[4].s[i];
		x[3].s[i] ^= x[2].s[i];
	}
	x[2].s[0] = ~x[2].s[0];
	// linear layer
	for (size_t i = 0; i < MASKING_ORDER; i++) {
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