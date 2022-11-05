#include <immintrin.h>

#define _xmm_rotr(n) \
	asm volatile("movdqa xmm0, xmm3": : :);  \
	asm volatile("psrldq xmm3, %0": :"g"(n):);  \
	asm volatile("pslldq xmm0, %0": :"g"(sizeof(__m128i) - n):);  \
	asm volatile("orpd xmm3, xmm0": : :);

#define _xmm_rotr2(n) \
	asm volatile("movdqa xmm10, xmm13": : :);  \
	asm volatile("psrldq xmm13, %0": :"g"(n):);  \
	asm volatile("pslldq xmm10, %0": :"g"(sizeof(__m128i) - n):);  \
	asm volatile("orpd xmm13, xmm10": : :);

// and of shares 1 and 2 --> res_share
#define _2S_AND1(p_share1, p_share2, res_share, i) \
	asm ("movhpd xmm0, %0"::"m"(R2[i]):);  \
	asm ("movlpd xmm0, %0"::"m"(R2[i]):);  \
	asm ("movdqu xmm2, %0"::"m"(p_share1):);  \
	asm ("xorpd xmm2, xmm0":::);  \
	asm ("movdqu xmm3, %0"::"m"(p_share2):);  \
	asm ("movdqu xmm0, xmm2":::);  \
	asm ("andpd xmm0, xmm3":::);  \
	asm ("movdqa xmm1, xmm0":::);  \
	_xmm_rotr(8) \
	asm ("andpd xmm2, xmm3":::);  \
	asm ("movhpd xmm0, %0"::"m"(R2[i+1]):);  \
	asm ("movlpd xmm0, %0"::"m"(R2[i+1]):);  \
	asm ("xorpd xmm1, xmm0":::);  \
	asm ("xorpd xmm1, xmm2":::);  \
	asm ("movdqu %0, xmm1":"=m"(res_share)::); 

#define _2S_AND2(p_share1, p_share2, res_share, i) \
	asm ("movhpd xmm10, %0"::"m"(R2[i]):);  \
	asm ("movlpd xmm10, %0"::"m"(R2[i]):);  \
	asm ("movdqu xmm12, %0"::"m"(p_share1):);  \
	asm ("xorpd xmm12, xmm10":::);  \
	asm ("movdqu xmm13, %0"::"m"(p_share2):);  \
	asm ("movdqu xmm10, xmm12":::);  \
	asm ("andpd xmm10, xmm13":::);  \
	asm ("movdqa xmm11, xmm10":::);  \
	_xmm_rotr2(8) \
	asm ("andpd xmm12, xmm13":::);  \
	asm ("movhpd xmm10, %0"::"m"(R2[i+1]):);  \
	asm ("movlpd xmm10, %0"::"m"(R2[i+1]):);  \
	asm ("xorpd xmm11, xmm10":::);  \
	asm ("xorpd xmm11, xmm12":::);  \
	asm ("movdqu %0, xmm11":"=m"(res_share)::);
