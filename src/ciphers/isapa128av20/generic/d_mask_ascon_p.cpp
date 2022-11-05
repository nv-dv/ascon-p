#include "ciphers/isapa128av20/generic/d_mask_ascon_p.h"
#include "gadgets/and_gate/generic/d_mask_and_gate.h"
#include "gadgets/ldl_gate/generic/d_mask_ldl_gate.h"
#include "gadgets/state/generic/d_mask_state.h"

#define CEILING(x,y) (((x) + (y) - 1) / (y))

#ifdef UMA_AND
#define AND_RAND_COUNT (MASKING_ORDER)*CEILING(MASKING_ORDER-1, 4)
#else
#define AND_RAND_COUNT (MASKING_ORDER)*(MASKING_ORDER-1) / 2
#endif

#define AND dS_AND

dshare d_share_x[5];
dshare d_share_t[5];
uint64_t Rd[5*AND_RAND_COUNT];

void dS_ROUND(int C) {
	randbuf.GetBytes((uint8_t*)Rd, sizeof(Rd));
	dshare* x = d_share_x;
	dshare* t = d_share_t;
	dshare rot[5];
	//x2 ^= c
	x[2].s[0] ^= C;
	//x0 ^= x4;     x4 ^= x3;    x2 ^= x1;
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		x[0].s[i] ^= x[4].s[i];
		x[4].s[i] ^= x[3].s[i];
		x[2].s[i] ^= x[1].s[i];
	}
	//t0 = ~x0;    t1 = ~x1;    t2 = ~x2;    t3 = ~x3;    t4 = ~x4;
	t[0].s[0] = ~x[0].s[0];
	t[1].s[0] = ~x[1].s[0];
	t[2].s[0] = ~x[2].s[0];
	t[3].s[0] = ~x[3].s[0];
	t[4].s[0] = ~x[4].s[0];
	for (int i = 1; i < MASKING_ORDER; i++)
	{
		t[0].s[i] = x[0].s[i];
		t[1].s[i] = x[1].s[i];
		t[2].s[i] = x[2].s[i];
		t[3].s[i] = x[3].s[i];
		t[4].s[i] = x[4].s[i];
	}
	//t0 &= x1;    t1 &= x2;    t2 &= x3;    t3 &= x4;    t4 &= x0;
	t[0] = AND(t[0], x[1], 0);
	t[1] = AND(t[1], x[2], AND_RAND_COUNT);
	t[2] = AND(t[2], x[3], AND_RAND_COUNT*2);
	t[3] = AND(t[3], x[4], AND_RAND_COUNT*3);
	t[4] = AND(t[4], x[0], AND_RAND_COUNT*4);
	//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
	for (size_t i = 0; i<MASKING_ORDER; i++) {
		x[0].s[i] ^= t[1].s[i];
		x[1].s[i] ^= t[2].s[i];
		x[2].s[i] ^= t[3].s[i];
		x[3].s[i] ^= t[4].s[i];
		x[4].s[i] ^= t[0].s[i];
	}
	//x1 ^= x0;    x0 ^= x4;    x3 ^= x2;    x2 = ~x2;
	for (size_t i=0; i<MASKING_ORDER; i++) {
		x[1].s[i] ^= x[0].s[i];
		x[0].s[i] ^= x[4].s[i];
		x[3].s[i] ^= x[2].s[i];
	}
	x[2].s[0] = ~x[2].s[0];
	// linear layer
	for (size_t i = 0; i < MASKING_ORDER; i++) {
		dS_LDL(i, 0, 19, 28);
		dS_LDL(i, 1, 39, 61);
		dS_LDL(i, 2, 1, 6);
		dS_LDL(i, 3, 10, 17);
		dS_LDL(i, 4, 7, 41);
	}
	// end
}

void InitDShares(uint64_t x[], uint64_t rand[]) {
	for (int i = 0; i < 5; i++)
	{
		dS_LOAD(d_share_x + i, x[i], rand);
		rand += MASKING_ORDER-1;
	}
}

void GetDShares() {
	uint64_t ans[5];
	for (int i = 0; i < 5; i++)
	{
		ans[i] = dS_GET(d_share_x + i);
		printf("%.16llx ", ans[i]);
	}
	printf("\n");
}