
#define _8S_LDL(share, rot1, rot2) \
	z0 = _mm512_loadu_si512(share); \
	z1 = _mm512_ror_epi64(z0, rot1); \
	z2 = _mm512_ror_epi64(z0, rot2); \
	z3 = _mm512_xor_epi64(z0, z1); \
	z4 = _mm512_xor_epi64(z2, z3); \
	_mm512_storeu_si512(share, z4);
