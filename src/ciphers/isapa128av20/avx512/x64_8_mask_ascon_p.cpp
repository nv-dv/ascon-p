#ifdef __AVX512F__
#include "ciphers/isapa128av20/avx512/8_mask_ascon_p.h"
#include "gadgets/and_gate/avx512/x64/8_mask_and_gate.h"
#include "gadgets/ldl_gate/avx512/x64/8_mask_ldl_gate.h"
#include "gadgets/state/avx512/8_mask_state.h"

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

__m512i DD = {0,0,0,0,0,0,0,0};

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

void Get8Shares() {
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

void Init8Shares(uint64_t state[], uint64_t rand[]) {
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
#endif