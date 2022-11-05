
// ymm3 >> 64 --> ymm3
#define _ymm3_rotl() \
	/*         0x93='0b10010011' --> 2 1 0 3 */ \
	asm volatile("vpermq ymm3, ymm3, 0x93");  

#define _ymm3_rotr() \
	/*         0x93='0b00111001' --> 0 3 2 1 */ \
	asm volatile("vpermq ymm3, ymm3, 0x39");  

#define _ymm4_rotr() \
	asm volatile("vpermq ymm4, ymm4, 0x39");  

#define _ymm13_rotl() \
	/*         0x93='0b10010011' --> 2 1 0 3 */ \
	asm volatile("vpermq ymm13, ymm13, 0x93");  

#define _ymm13_rotr() \
	/*         0x93='0b00111001' --> 0 3 2 1 */ \
	asm volatile("vpermq ymm13, ymm13, 0x39");  

#define _ymm14_rotr() \
	asm volatile("vpermq ymm14, ymm14, 0x39");  

// and of shares 1 and 2 --> res_share
#define _4S_AND1(p_share1, p_share2, res_share, i) \
	/* Inputs refresh */ \
	asm ("vmovdqa ymm4, %0"::"m"((((__m256*)R4)[i])):);  \
	asm ("vmovdqa ymm0, %0"::"m"(p_share1):);  \
	asm ("vmovdqa ymm3, %0"::"m"(p_share2):);  \
	asm ("vxorpd ymm1, ymm1, ymm1":::);  \
	asm ("vxorpd ymm0, ymm0, ymm4":::);  \
	_ymm4_rotr() \
	asm ("vxorpd ymm0, ymm0, ymm4":::);  \
	asm ("vmovdqa ymm4, %0"::"m"((((__m256*)R4)[i+1])):);  \
	/*body of AND gate*/   \
	/* Do 4 times */  \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	/* first refresh */  \
	asm ("vxorpd ymm1, ymm1, ymm4");  \
	_ymm4_rotr() \
	_ymm3_rotr() \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	_ymm3_rotr() \
	_ymm3_rotr() \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	/*Second refresh*/  \
	asm ("vxorpd ymm1, ymm1, ymm4");  \
	_ymm3_rotl() \
	asm ("vandpd ymm2, ymm0, ymm3":::);  \
	asm ("vxorpd ymm1, ymm1, ymm2":::);  \
	/* Write results */  \
	asm ("vmovdqa %0, ymm1":"=m"(res_share)::);  

#define _4S_AND2(p_share1, p_share2, res_share, i) \
	/* Inputs refresh */ \
	asm ("vmovdqa ymm14, %0"::"m"((((__m256*)R4)[i])):);  \
	asm ("vmovdqa ymm10, %0"::"m"(p_share1):);  \
	asm ("vmovdqa ymm13, %0"::"m"(p_share2):);  \
	asm ("vxorpd ymm11, ymm11, ymm11":::);  \
	asm ("vxorpd ymm10, ymm10, ymm14":::);  \
	_ymm14_rotr() \
	asm ("vxorpd ymm10, ymm10, ymm14":::);  \
	asm ("vmovdqa ymm14, %0"::"m"((((__m256*)R4)[i+1])):);  \
	/*body of AND gate*/   \
	/* Do 4 times */  \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	/* first refresh */  \
	asm ("vxorpd ymm11, ymm11, ymm14");  \
	_ymm14_rotr() \
	_ymm13_rotr() \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	_ymm13_rotr() \
	_ymm13_rotr() \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	/*Second refresh*/  \
	asm ("vxorpd ymm11, ymm11, ymm14");  \
	_ymm13_rotl() \
	asm ("vandpd ymm12, ymm10, ymm13":::);  \
	asm ("vxorpd ymm11, ymm11, ymm12":::);  \
	/* Write results */  \
	asm ("vmovdqa %0, ymm11":"=m"(res_share)::);
