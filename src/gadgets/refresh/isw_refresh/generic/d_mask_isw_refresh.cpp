#include "gadgets/refresh/d_mask_refresh.h"
#include "random/RandomBuffer.h"

#ifdef REFRESH_ISW
	void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t localRand[(MASKING_ORDER*(MASKING_ORDER-1)) / 2];
		size_t randindex = 0;
		dshare ans;
		rb->GetBytes(localRand, sizeof(localRand));
		for (size_t i = 0; i < MASKING_ORDER; i++) {
			ans.s[i] = 0;
		}
		for (size_t i = 0; i < MASKING_ORDER; i++)
		{
			ans.s[i] ^= p1.s[i] & p1.s[i];
			for (size_t j = i+1; j < MASKING_ORDER; j++)
			{
				ans.s[i] ^= localRand[randindex];
				ans.s[i] ^= p1.s[i] & p1.s[j];
				ans.s[j] ^= localRand[randindex];
				ans.s[j] ^= p1.s[j] & p1.s[i];
				randindex++;
			}
		}
		// write back ans
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] = ans.s[i];}
	}
#endif