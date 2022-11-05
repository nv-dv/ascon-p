#include "ciphers/isapa128av20/avx512/8_mask_ascon_p.h"

#define _zmm_rotr(a) _mm512_permutexvar_epi64(vec_ror, a)
#define _zmm_rotl(a) _mm512_permutexvar_epi64(vec_rol, a)

extern __m512i RandIn[5*3];
extern uint64_t SR8[5*3];
extern void _8S_AND(share8* share0, share8* share1, share8* res_share, int i, int j);
