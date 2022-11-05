
// rotate right share by i bits --> xmm0
#define _2S_ROTR1(p_share, i) \
	asm volatile("movdqu xmm0, %0"::"m"(p_share):);  \
	asm volatile("movdqu xmm1, xmm0":::);  \
	asm volatile("psrlq xmm0, %0"::"i"(i):);  \
	asm volatile("psllq xmm1, %0"::"i"(64-i):);  \
	asm volatile("orpd xmm0, xmm1":::);

#define _2S_ROTR2(p_share, i) \
	asm volatile("movdqu xmm3, %0"::"m"(p_share):);  \
	asm volatile("movdqu xmm4, xmm3":::);  \
	asm volatile("psrlq xmm3, %0"::"i"(i):);  \
	asm volatile("psllq xmm4, %0"::"i"(64-i):);  \
	asm volatile("orpd xmm3, xmm4":::);

#define _2S_LDL(p_share, rot1, rot2) \
	_2S_ROTR1(p_share, rot1) \
	asm volatile("movdqa xmm2, xmm0":::);  \
	/* TODO: add refresh */ \
	_2S_ROTR2(p_share, rot2) \
	asm volatile("xorpd xmm2, xmm3":::);  \
	asm volatile("movdqu xmm3, %0"::"m"(p_share):);  \
	asm volatile("xorpd xmm2, xmm3":::);  \
	asm volatile("movdqu %0, xmm2":"=m"(p_share)::);
