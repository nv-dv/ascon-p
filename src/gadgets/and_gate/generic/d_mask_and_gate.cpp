#include "gadgets/and_gate/generic/d_mask_and_gate.h"
#ifdef REFRESH
#include "gadgets/refresh/d_mask_refresh.h"
#endif
#include "globals.h"

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

#ifdef UMA_AND
dshare dS_AND(dshare &a, dshare &b, size_t randindex) {
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
#else
// general dS_AND with refresh
dshare dS_AND(dshare &p1, dshare &p2, size_t randindex) {
	#ifdef REFRESH
	refresh(p1, p2, &randbuf);
	#endif // REFRESH
	dshare ans;
	// matrix mult. with minimal randomness...
	#ifndef OPTIMIZE_SSE2
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
#endif