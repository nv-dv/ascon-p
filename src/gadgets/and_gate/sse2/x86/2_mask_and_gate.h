#include <immintrin.h>

#define _xmm_rotr(n) \
	asm volatile("movdqa xmm0, xmm3": : :);  \
	asm volatile("psrldq xmm3, %0": :"g"(n):);  \
	asm volatile("pslldq xmm0, %0": :"g"(sizeof(__m128i) - n):);  \
	asm volatile("orpd xmm3, xmm0": : :);

// and of shares 1 and 2 --> res_share
#define _2S_AND(p_share1, p_share2, res_share, i) \
	asm volatile("movhpd xmm0, %0"::"m"(R2[i]):);  \
	asm volatile("movlpd xmm0, %0"::"m"(R2[i]):);  \
	asm volatile("movdqu xmm2, %0"::"m"(p_share1):);  \
	asm volatile("xorpd xmm2, xmm0":::);  \
	asm volatile("movdqu xmm3, %0"::"m"(p_share2):);  \
	asm volatile("movdqu xmm0, xmm2":::);  \
	asm volatile("andpd xmm0, xmm3":::);  \
	asm volatile("movdqa xmm1, xmm0":::);  \
	_xmm_rotr(8) \
	asm volatile("andpd xmm2, xmm3":::);  \
	asm volatile("movhpd xmm0, %0"::"m"(R2[i+1]):);  \
	asm volatile("movlpd xmm0, %0"::"m"(R2[i+1]):);  \
	asm volatile("xorpd xmm1, xmm0":::);  \
	asm volatile("xorpd xmm1, xmm2":::);  \
	asm volatile("movdqu %0, xmm1":"=m"(res_share)::);
