
#define _4S_LDL1(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa ymm0, %0"::"m"(p_share):);  \
	asm ("vpsllq ymm1, ymm0, %0"::"i"(64-rot1):);  \
	asm ("vpsllq ymm4, ymm0, %0"::"i"(64-rot2):);  \
	asm ("vpsrlq ymm3, ymm0, %0"::"i"(rot2):);  \
	asm ("vpsrlq ymm0, ymm0, %0"::"i"(rot1):);  \
	asm ("vorpd ymm0, ymm0, ymm1":::);  \
	asm ("vorpd ymm3, ymm3, ymm4":::);  \
	asm ("vxorpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm2, ymm2, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, ymm2":"=m"(p_share)::);

#define _4S_LDL2(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa ymm5, %0"::"m"(p_share):);  \
	asm ("vpsllq ymm9, ymm5, %0"::"i"(64-rot2):);  \
	asm ("vpsllq ymm6, ymm5, %0"::"i"(64-rot1):);  \
	asm ("vpsrlq ymm8, ymm5, %0"::"i"(rot2):);  \
	asm ("vpsrlq ymm5, ymm5, %0"::"i"(rot1):);  \
	asm ("vorpd ymm5, ymm5, ymm6":::);  \
	asm ("vorpd ymm8, ymm8, ymm9":::);  \
	asm ("vxorpd ymm7, ymm5, ymm8":::);  \
	asm ("vxorpd ymm7, ymm7, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, ymm7":"=m"(p_share)::);

#define _4S_LDL3(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm ("vmovdqa ymm10, %0"::"m"(p_share):);  \
	asm ("vpsllq ymm14, ymm10, %0"::"i"(64-rot2):);  \
	asm ("vpsllq ymm11, ymm10, %0"::"i"(64-rot1):);  \
	asm ("vpsrlq ymm13, ymm10, %0"::"i"(rot2):);  \
	asm ("vpsrlq ymm10, ymm10, %0"::"i"(rot1):);  \
	asm ("vorpd ymm10, ymm10, ymm11":::);  \
	asm ("vorpd ymm13, ymm13, ymm14":::);  \
	asm ("vxorpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm12, ymm12, %0"::"m"(p_share):);  \
	asm ("vmovdqa %0, ymm12":"=m"(p_share)::);
