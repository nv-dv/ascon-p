
// ymm3 >> 64 --> ymm3
#define _ymm3_rotl() \
	/*         0x93='0b10010011' --> 2 1 0 3 */ \
	asm("vpermq ymm3, ymm3, 0x93");

#define _ymm3_rotr() \
	/*         0x93='0b00111001' --> 0 3 2 1 */ \
	asm("vpermq ymm3, ymm3, 0x39");

#define _ymm4_rotr() \
	asm("vpermq ymm4, ymm4, 0x39");

// and of shares 1 and 2 --> res_share
#define _4S_AND(p_share1, p_share2, res_share, i) \
	/* Inputs refresh */ \
	asm("vmovdqu ymm4, %0"::"m"((((__m256*)R4)[i])):);  \
	asm("vmovdqu ymm0, %0"::"m"(p_share1):);  \
	asm("vxorpd ymm0, ymm0, ymm4":::);  \
	_ymm4_rotr() \
	asm("vxorpd ymm0, ymm0, ymm4":::);  \
	asm("vxorpd ymm1, ymm1, ymm1":::);  \
	/*body of AND gate*/   \
	asm("vmovdqu ymm3, %0"::"m"(p_share2):);  \
	/* Do 4 times */  \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	/* first refresh */  \
	asm("vmovdqu ymm4, %0"::"m"((((__m256*)R4)[i+1])):);  \
	asm("vxorpd ymm1, ymm1, ymm4");  \
	_ymm4_rotr() \
	_ymm3_rotr() \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	_ymm3_rotr() \
	_ymm3_rotr() \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	/*Second refresh*/  \
	asm("vxorpd ymm1, ymm1, ymm4");  \
	_ymm3_rotl() \
	asm("vandpd ymm2, ymm0, ymm3":::);  \
	asm("vxorpd ymm1, ymm1, ymm2":::);  \
	/* Write results */  \
	asm("vmovdqu %0, ymm1":"=m"(res_share)::);
