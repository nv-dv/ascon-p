#ifdef __AVX512F__
#include "gadgets/and_gate/avx512/x64/8_mask_and_gate.h"


__m512i E = {0,0,0,0,0,0,0,0};
// normal mask is 0 1 2 3 4 5 6 7
__m512i vec_ror = {7, 0, 1, 2, 3, 4, 5, 6};
__m512i vec_rol = {1, 2, 3, 4, 5, 6, 7, 0};

void _8S_AND(share8* share0, share8* share1, share8* res_share, int i, int j) {
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
#endif