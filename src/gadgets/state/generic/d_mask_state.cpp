#include "gadgets/state/generic/d_mask_state.h"

void dS_LOAD(dshare* p_share, uint64_t x, uint64_t rand[]) {
	uint64_t tmp = x;
	for (int i = 0; i < MASKING_ORDER-1; i++)
	{
		tmp ^= rand[i];
		p_share->s[i] = rand[i];
	}
	p_share->s[MASKING_ORDER - 1] = tmp;
}

uint64_t dS_GET(dshare* p_share) {
	uint64_t tmp = p_share->s[0];
	for (int i = 1; i < MASKING_ORDER; i++)
	{
		tmp ^= p_share->s[i];
	}
	return tmp;
}