#pragma once
#include<stdio.h>
#include<cstdint>
#include "dmask_AND_gate.h"

#define d MASKING_ORDER
typedef struct dshare_t
{
	uint64_t s[d];
} dshare;



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
	#define TYPE_UINT64 8
	#define TYPE_UINT128 16
#endif

dshare d_share_x[5];
dshare d_share_t[5];
RandomBuffer randbuf = RandomBuffer(100 * 2<<20);

#define uint64_rotr(x,n) (((x)>>(n))|((x)<<(64-(n))))
#define uint64_cpy(dst, src) dst[0]=src[0]; dst[1]=src[1];

inline void dS_LOAD(dshare* p_share, uint64_t x, uint64_t rand[]) {
	uint64_t tmp = x;
	for (int i = 0; i < d-1; i++)
	{
		tmp ^= rand[i];
		p_share->s[i] = rand[i];
	}
	p_share->s[d - 1] = tmp;
}

inline uint64_t dS_GET(dshare* p_share) {
	uint64_t tmp = p_share->s[0];
	for (int i = 1; i < d; i++)
	{
		tmp ^= p_share->s[i];
	}
	return tmp;
}

inline dshare dS_XOR(dshare* p1, dshare* p2) {
	dshare ans;
	#ifdef OPTIMIZE_SEE2
	uint64_t *ptr1 = (uint64_t*)p1->s,  *ptr2 = (uint64_t*)p2->s;
	for (size_t i = 0; i < (d & 0xfe); i+=2)
	{
		asm("movdqu xmm0, %0;\t\nmovdqu xmm1, %1;\t\nxorpd xmm0, xmm1;\t\nmovdqu %0, xmm0;\t\n":"=m"(ptr1):"m"(ptr2):);
		ans.s[i] = local1[0];
		ans.s[i + 1] = local1[1];
		ptr1+=2;
		ptr2+=2;
	}
	if (d & 1) {
		ans.s[d - 1] = p1->s[d - 1] ^ p2->s[d - 1];
	}
	#elif OPTIMIZE_AVX2
	#else
	for (size_t i = 0; i < d; i++)
	{
		ans.s[i] = p1->s[i] ^ p2->s[i];
	}
	#endif
	return ans;
}

inline dshare dS_NOT(dshare* p_share) {
	dshare ans;
	for (int i = 0; i < d; i++)
	{
		ans.s[i] = p_share->s[i];
	}
	ans.s[0] ^= C_not;
	return ans;
}

// general dS_AND with no refresh
inline dshare dS_AND(dshare* p1, dshare* p2) {
	#ifdef REFRESH
	refresh(p1, p2, &randbuf);
	#endif // REFRESH
	uint64_t m[d][d];
	uint64_t tmp;
	dshare ans;
	// adding randomness (minimal)...
	for (size_t i = 0; i < d; i++)
	{
		m[i][i] = 0;
		for (size_t j = i+1; j < d; j++)
		{
			tmp = randbuf.GetQWORD();
			m[i][j] = tmp;
			m[j][i] = tmp;
		}
	}
	// matrix mult. with minimal randomness...
	#ifdef OPTIMIZE_SEE2
	uint64_t *ptr1 = p1->s, *ptr2 = p2->s;
	for (size_t i = 0; i < d; i++)
	{
		for (size_t j = 0; j < (d & 0xfe); j+=2)
		{
			asm("
				movhpd xmm0, %1;\n
                movlpd xmm0, %1;\n
				movdqu xmm1, %2;\n
				andpd xmm0, xmm1;\n
				movdqu local0, xmm0;\n
			":"=m"(ptr1):"m"(ptr1[i]), "m"(ptr2):)
			m[i][j] ^= local0[0];
			m[i][j + 1] ^= local0[1];
			ptr2 += 2;
		}
		if (d & 1) {
			m[i][d - 1] ^= p1->s[i] & p2->s[d - 1];
		}
		ptr2 = p2->s;
	}
	#elif OPTIMIZE_AVX2

	#else
	for (size_t i = 0; i < d; i++)
	{
		for (size_t j = 0; j < d; j++)
		{
			m[i][j] ^= p1->s[i] & p2->s[j];
		}
	}
	#endif
	// compute ans
	for (size_t i = 0; i < d; i++)
	{
		ans.s[i] = 0;
		for (size_t j = 0; j < d; j++)
		{
			ans.s[i] ^= m[i][j];
		}
	}
	return ans;
}

inline dshare dS_ROTR(dshare* p_share, int n) {
	dshare ans;
	uint64_t tmp;
	for (int i = 0; i < d; i++)
	{
		tmp = p_share->s[i];
		ans.s[i] = uint64_rotr(tmp, n);
	}
	return ans;
}

inline dshare dS_LDL(dshare* p_share, int rot1, int rot2) {
	dshare ans;
	dshare tmp;
	for (int i = 0; i < d; i++)
	{
		ans.s[i] = p_share->s[i];
	}
	tmp = dS_ROTR(p_share, rot1);
	for (int i = 0; i < d; i++)
	{
		ans.s[i] ^= tmp.s[i];
	}
	tmp = dS_ROTR(p_share, rot2);
	for (int i = 0; i < d; i++)
	{
		ans.s[i] ^= tmp.s[i];
	}
	return ans;
}

inline void dS_ROUND(int C) {
	dshare* x = d_share_x;
	dshare* t = d_share_t;
	// 0
	x[2].s[0] ^= C;
	x[0] = dS_XOR(x + 0, x + 4);
	// 1
	x[4] = dS_XOR(x + 4, x + 3);
	x[2] = dS_XOR(x + 2, x + 1);
	// 3
	for (int i = 0; i < 5; i++)
	{
		t[i] = dS_NOT(x + i);
	}
	// 4
	int tmp;
	for (int i = 0; i < 5; i++)
	{
		tmp = ((i + 1) % 5);
		t[i] = dS_AND(t + i, x + tmp);
	}
	for (int i = 0; i < 5; i++)
	{
		tmp = ((i + 1) % 5);
		x[i] = dS_XOR(x + i, t + tmp);
	}
	// 5
	x[1] = dS_XOR(x + 1, x + 0);
	x[0] = dS_XOR(x + 0, x + 4);
	x[3] = dS_XOR(x + 3, x + 2);
	x[2] = dS_NOT(x + 2);
	// 6
	x[0] = dS_LDL(x+0, 19, 28);
	x[1] = dS_LDL(x + 1, 39, 61);
	x[2] = dS_LDL(x + 2, 1, 6);
	x[3] = dS_LDL(x + 3, 10, 17);
	x[4] = dS_LDL(x + 4, 7, 41);
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
		rand += d-1;
	}
}

inline void GetDShares() {
	uint64_t ans[5];
	for (int i = 0; i < 5; i++)
	{
		ans[i] = dS_GET(d_share_x + i);
		printf("%.16llx ", ans[i]);
	}
}