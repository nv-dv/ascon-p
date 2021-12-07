#pragma once
#include "../RandomBuffer/RandomBuffer.h"


#if d==2
//insert d=2
inline void refresh(dshare* p1, dshare* p2, RandomBuffer* rb) {
	uint64_t r = rb->GetQWORD();
	for (size_t i = 0; i < d; i++)
	{
		p1->s[i] ^= r;
	}
}
#endif // d==2
#if d==3
//insert d=3
inline void refresh(dshare* p1, dshare* p2, RandomBuffer* rb) {
	uint64_t r[d];
	r[0] = rb->GetQWORD();
	r[1] = rb->GetQWORD();
	r[2] = r[0] ^ r[1];
	for (size_t i = 0; i < d; i++)
	{
		p1->s[i] ^= r[i];
	}
}
#endif // d==3
#if d==4 || d==5
	//insert d=4 or d=5
inline void refresh(dshare* p1, dshare* p2, RandomBuffer* rb) {
	uint64_t s[d];
	uint64_t tmp;
	// get random
	for (size_t i = 0; i < d; i++) {s[i] = rb->GetQWORD();}
	tmp = s[d-1];
	// rotate
	for (size_t i = d - 1; i > 0; i--) {s[i] = s[i] ^ s[i - 1];}
	s[0] = s[0] ^ tmp;
	// apply random
	for (size_t i = 0; i < d; i++) {p1->s[i] ^= s[i];}
}
#endif // d==4

#if d==6
	//insert d=6
inline void refresh(dshare* p1, dshare* p2, RandomBuffer* rb) {
	uint64_t s[d];
	uint64_t tmp;
	// get random
	for (size_t i = 0; i < d; i++) { s[i] = rb->GetQWORD(); }
	tmp = s[d - 1];
	// rotate
	for (size_t i = d - 1; i > 0; i--) { s[i] = s[i] ^ s[i - 1]; }
	s[0] = s[0] ^ tmp;
	// add half random
	tmp = rb->GetQWORD();
	s[0] ^= tmp;
	s[3] ^= tmp;
	// apply random
	for (size_t i = 0; i < d; i++) { p1->s[i] ^= s[i]; }
}
#endif


