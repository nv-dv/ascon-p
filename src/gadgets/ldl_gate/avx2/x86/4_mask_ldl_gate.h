
#define _4S_LDL(p_share, rot1, rot2) \
	/*_4S_ROTR(p_share, rot1)*/ \
	/*_4S_ROTR(p_share, rot2)*/ \
	asm("vmovdqu ymm0, %0"::"m"(p_share):);  \
	asm("vmovdqu ymm3, ymm0":::);  \
	asm("vmovdqu ymm1, ymm0":::);  \
	asm("vmovdqu ymm4, ymm3":::);  \
	asm("vpsrlq ymm0, ymm0, %0"::"i"(rot1):);  \
	asm("vpsllq ymm1, ymm1, %0"::"i"(64-rot1):);  \
	asm("vpsrlq ymm3, ymm3, %0"::"i"(rot2):);  \
	asm("vpsllq ymm4, ymm4, %0"::"i"(64-rot2):);  \
	asm("vorpd ymm0, ymm0, ymm1":::);  \
	asm("vorpd ymm3, ymm3, ymm4":::);  \
	asm("vxorpd ymm2, ymm0, ymm3":::);  \
	asm("vmovdqu ymm3, %0"::"m"(p_share):);  \
	asm("vxorpd ymm2, ymm2, ymm3":::);  \
	asm("vmovdqu %0, ymm2":"=m"(p_share)::);
