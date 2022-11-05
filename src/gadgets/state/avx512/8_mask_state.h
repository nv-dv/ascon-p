
#define _8S_LOAD(p_share, x, rand) \
	asm volatile("movq xmm0, %0"::"m"(x):);  \
	for (size_t i = 0; i < 8-1; ++i) {  \
	asm volatile("movq xmm1, %0"::"m"(rand[i]));  \
	asm volatile("xorpd xmm0, xmm1");  \
	p_share.s[i] = rand[i];}  \
	asm volatile("movq %0, xmm0":"=m"(p_share.s[7])::);

// s0 ^ s1 --> result
#define _8S_GET(p_share, result) \
	asm volatile("movq xmm0, %0"::"m"(p_share.s[0]):);  \
	for (size_t i = 1; i < 8; ++i) {  \
	asm volatile("movq xmm1, %0"::"m"(p_share.s[i]));  \
	asm volatile("xorpd xmm0, xmm1");}  \
	asm volatile("movq %0, xmm0":"=m"(result)::);
