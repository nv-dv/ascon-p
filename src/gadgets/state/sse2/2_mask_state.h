
#define _2S_LOAD(p_share, x, rand) \
	asm volatile("movq xmm0, %0\n"::"m"(x):);  \
	asm volatile("movq xmm1, %0\n"::"m"(rand):);  \
	asm volatile("xorpd xmm0, xmm1\n");  \
	asm volatile("movhpd xmm0, %0\n"::"m"(rand):);  \
	asm volatile("movdqu %0, xmm0":"=m" (p_share)::);

// s0 ^ s1 --> result
# define _2S_GET(p_share, result) \
	asm volatile("movdqu xmm0, %0": :"m"(p_share) :);  \
	asm volatile("xorpd xmm1, xmm1": : :);  \
	asm volatile("movhpd xmm1, %0": :"m"(p_share.s0) :);  \
	asm volatile("xorpd xmm0, xmm1": : :);  \
	asm volatile("movhpd %0, xmm0":"=m"(result) : :);
