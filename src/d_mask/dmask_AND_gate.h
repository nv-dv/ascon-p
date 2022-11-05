#pragma once
#include "random/RandomBuffer.h"

#if defined(REFRESH_HPC)
	#if MASKING_ORDER==2
	//insert d=2
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t r = rb->GetQWORD();
		for (size_t i = 0; i < MASKING_ORDER; i++)
		{
			p1.s[i] ^= r;
		}
	}
	#elif MASKING_ORDER==3
	//insert d=3
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t r[MASKING_ORDER];
		r[0] = rb->GetQWORD();
		r[1] = rb->GetQWORD();
		r[2] = r[0] ^ r[1];
		for (size_t i = 0; i < MASKING_ORDER; i++)
		{
			p1.s[i] ^= r[i];
		}
	}
	#elif MASKING_ORDER==4 || MASKING_ORDER==5
	//insert d=4 or d=5
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		tmp = s[MASKING_ORDER-1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) {s[i] = s[i] ^ s[i - 1];}
		s[0] = s[0] ^ tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) {p1.s[i] ^= s[i];}
	}

	#elif MASKING_ORDER==6
	//insert d=6
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		tmp = rb->GetQWORD();
		s[0] ^= tmp;
		s[3] ^= tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#elif MASKING_ORDER==7
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		tmp = rb->GetQWORD();
		s[0] ^= tmp;
		s[4] ^= tmp;
		tmp = rb->GetQWORD();
		s[2] ^= tmp;
		s[6] ^= tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#elif MASKING_ORDER==8
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		tmp = rb->GetQWORD();
		s[0] ^= tmp;
		s[4] ^= tmp;
		tmp = rb->GetQWORD();
		s[1] ^= tmp;
		s[5] ^= tmp;
		tmp = rb->GetQWORD();
		s[2] ^= tmp;
		s[6] ^= tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#elif MASKING_ORDER==9
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		tmp = rb->GetQWORD();
		s[0] ^= tmp;
		s[4] ^= tmp;
		tmp = rb->GetQWORD();
		s[1] ^= tmp;
		s[6] ^= tmp;
		tmp = rb->GetQWORD();
		s[3] ^= tmp;
		s[7] ^= tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#elif MASKING_ORDER==10
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t ss[MASKING_ORDER/2];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		rb->GetBytes(ss, sizeof(ss));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		for (size_t i = 0; i < MASKING_ORDER/2; ++i) {
			s[i] ^= ss[i]; s[i+MASKING_ORDER/2] ^= ss[i];
		}
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#elif MASKING_ORDER==11
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t ss[MASKING_ORDER/2];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		rb->GetBytes(ss, sizeof(ss));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		for (size_t i = 0; i < MASKING_ORDER/2; ++i) {
			s[i] ^= ss[i]; s[i+MASKING_ORDER/2] ^= ss[i];
		}
		tmp = rb->GetQWORD();
		s[7] ^= tmp;	
		s[10] ^= tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#elif MASKING_ORDER==12
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t ss[MASKING_ORDER/2];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		rb->GetBytes(ss, sizeof(ss));
		tmp = s[MASKING_ORDER - 1];
		// rotate
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		// add half random
		for (size_t i = 0; i < MASKING_ORDER/2; ++i) {
			s[i] ^= ss[i]; s[i+MASKING_ORDER/2] ^= ss[i];
		}
		tmp = rb->GetQWORD();
		s[2] ^= tmp;	
		s[5] ^= tmp;
		tmp = rb->GetQWORD();
		s[8] ^= tmp;	
		s[11] ^= tmp;
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}
	#else
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
		uint64_t s[MASKING_ORDER];
		uint64_t ss[MASKING_ORDER];
		uint64_t tmp;
		// get random
		rb->GetBytes(s, sizeof(s));
		rb->GetBytes(ss, sizeof(ss));
		// rotate
		tmp = s[MASKING_ORDER - 1];
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
		s[0] = s[0] ^ tmp;
		tmp = ss[MASKING_ORDER - 1];
		for (size_t i = MASKING_ORDER - 1; i > 0; i--) { ss[i] = ss[i] ^ ss[i - 1]; }
		ss[0] = ss[0] ^ tmp;
		// add half random
		for (size_t i = 0; i < MASKING_ORDER; ++i) {
			s[i] ^= ss[i];
		}
		// apply random
		for (size_t i = 0; i < MASKING_ORDER; i++) { p1.s[i] ^= s[i]; }
	}

	#endif
#elif defined(REFRESH_ISW)
	inline void refresh(dshare &p1, dshare &p2, RandomBuffer* rb) {
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
