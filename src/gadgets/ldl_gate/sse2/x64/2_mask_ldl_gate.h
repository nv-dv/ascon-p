
#define _2S_LDL1(p_share, rot1, rot2) \
	/*_2S_ROTR1(p_share, rot1)*/ \
	/*_2S_ROTR2(p_share, rot2)*/ \
	asm ("movdqu xmm0, %0"::"m"(p_share):);  \
	asm ("movdqu xmm3, %0"::"m"(p_share):);  \
	asm ("movdqu xmm1, xmm0":::);  \
	asm ("movdqu xmm4, xmm3":::);  \
	asm ("psrlq xmm0, %0"::"i"(rot1):);  \
	asm ("psllq xmm1, %0"::"i"(64-rot1):);  \
	asm ("psrlq xmm3, %0"::"i"(rot2):);  \
	asm ("psllq xmm4, %0"::"i"(64-rot2):);  \
	asm ("orpd xmm0, xmm1":::);  \
	asm ("orpd xmm3, xmm4":::);  \
	asm ("vxorpd xmm2, xmm0, xmm3":::);  \
	asm ("movdqu xmm3, %0"::"m"(p_share):);  \
	asm ("xorpd xmm2, xmm3":::);  \
	asm ("movdqu %0, xmm2":"=m"(p_share)::); 

#define _2S_LDL2(p_share, rot1, rot2) \
	/*_2S_ROTR1(p_share, rot1)*/ \
	/*_2S_ROTR2(p_share, rot2)*/ \
	asm ("movdqu xmm5, %0"::"m"(p_share):);  \
	asm ("movdqu xmm8, %0"::"m"(p_share):);  \
	asm ("movdqu xmm6, xmm5":::);  \
	asm ("movdqu xmm9, xmm8":::);  \
	asm ("psrlq xmm5, %0"::"i"(rot1):);  \
	asm ("psllq xmm6, %0"::"i"(64-rot1):);  \
	asm ("psrlq xmm8, %0"::"i"(rot2):);  \
	asm ("psllq xmm9, %0"::"i"(64-rot2):);  \
	asm ("orpd xmm5, xmm6":::);  \
	asm ("orpd xmm8, xmm9":::);  \
	asm ("vxorpd xmm7, xmm5, xmm8":::);  \
	asm ("movdqu xmm8, %0"::"m"(p_share):);  \
	asm ("xorpd xmm7, xmm8":::);  \
	asm ("movdqu %0, xmm7":"=m"(p_share)::);

#define _2S_LDL3(p_share, rot1, rot2) \
	/*_2S_ROTR1(p_share, rot1)*/ \
	/*_2S_ROTR2(p_share, rot2)*/ \
	asm ("movdqu xmm10, %0"::"m"(p_share):);  \
	asm ("movdqu xmm13, %0"::"m"(p_share):);  \
	asm ("movdqu xmm11, xmm10":::);  \
	asm ("movdqu xmm14, xmm13":::);  \
	asm ("psrlq xmm10, %0"::"i"(rot1):);  \
	asm ("psllq xmm11, %0"::"i"(64-rot1):);  \
	asm ("psrlq xmm13, %0"::"i"(rot2):);  \
	asm ("psllq xmm14, %0"::"i"(64-rot2):);  \
	asm ("orpd xmm10, xmm11":::);  \
	asm ("orpd xmm13, xmm14":::);  \
	asm ("vxorpd xmm12, xmm10, xmm13":::);  \
	asm ("movdqu xmm13, %0"::"m"(p_share):);  \
	asm ("xorpd xmm12, xmm13":::);  \
	asm ("movdqu %0, xmm12":"=m"(p_share)::);
