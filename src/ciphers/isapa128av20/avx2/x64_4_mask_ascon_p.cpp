#ifdef __x86_64__
#include "ciphers/isapa128av20/avx2/4_mask_ascon_p.h"
#include "gadgets/and_gate/avx2/x64/4_mask_and_gate.h"
#include "gadgets/ldl_gate/avx2/x64/4_mask_ldl_gate.h"
#include "gadgets/state/avx2/4_mask_state.h"

// state words x0..x4 (uint64), temporary variables t0..t4 (uint64)
// 0
//x2 ^= c
//x0 ^= x4; //1 x4 ^= x3;    x2 ^= x1;
// 2
//t0 = x0;    t1 = x1;    t2 = x2;    t3 = x3;    t4 = x4;
// 3
//t0 = ~t0;    t1 = ~t1;    t2 = ~t2;    t3 = ~t3;    t4 = ~t4;
//t0 &= x1;    t1 &= x2;    t2 &= x3;    t3 &= x4;    t4 &= x0;
// 4
//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
// 5
//x1 ^= x0;    x0 ^= x4;    x3 ^= x2;    x2 = ~x2;
// linear layer
//t0 = x0 ^ ROTR(x0, 28);
//t1 = x1 ^ ROTR(x1, 39);
//t2 = x2 ^ ROTR(x2, 6);
//t3 = x3 ^ ROTR(x3, 17);
//t4 = x4 ^ ROTR(x4, 41);
//t0 ^= ROTR(x0, 19);
//t1 ^= ROTR(x1, 61);
//t2 ^= ROTR(x2, 1);
//t3 ^= ROTR(x3, 10);
//t4 ^= ROTR(x4, 7);
//for (int i = 0;i < 5;++i) x[i] = t[i];

uint64_t D[4] = {0,0,0,0};

uint64_t R4[40];
share_4 ssx0;
share_4 ssx1;
share_4 ssx2;
share_4 ssx3;
share_4 ssx4;
share_4 sst0;
share_4 sst1;
share_4 sst2;
share_4 sst3;
share_4 sst4;

void _4S_ROUND(uint64_t c) {
	randbuf.GetBytes((uint8_t*)R4, sizeof(R4)); 
	/*load state*/ 
	D[3] = c;
	asm volatile("vmovdqa ymm11, %0"::"m"((((__m256*)D)[0])):); 
	asm volatile("vmovdqa ymm0, %0"::"m"(ssx0):);  
	asm volatile("vmovdqa ymm4, %0"::"m"(ssx4):);  
	asm volatile("vmovdqa ymm2, %0"::"m"(ssx2):);  
	asm volatile("vmovdqa ymm3, %0"::"m"(ssx3):);  
	asm volatile("vmovdqa ymm1, %0"::"m"(ssx1):);  
	D[3] = C_not;  
	asm volatile("vmovdqa ymm5, %0"::"m"((((__m256*)D)[0])):);  
	asm volatile("vxorpd ymm2, ymm2, ymm11":::);  /*XOR(ssx2, c)*/  
	asm volatile("vxorpd ymm0, ymm4, ymm0":::);  /*XOR(ssx0, ssx4)*/  
	asm volatile("vxorpd ymm4, ymm4, ymm3":::);  /*XOR(ssx4, ssx3)*/  
	asm volatile("vxorpd ymm2, ymm2, ymm1":::);  /*XOR(ssx2, ssx1)*/  
	asm volatile("vxorpd ymm11, ymm1, ymm5":::);  /*sst1 = NOT(ssx1)*/  
	asm volatile("vxorpd ymm13, ymm3, ymm5":::);  /*sst3 = NOT(ssx3)*/  
	asm volatile("vxorpd ymm10, ymm0, ymm5":::);  /*sst0 = NOT(ssx0)*/  
	asm volatile("vxorpd ymm14, ymm4, ymm5":::);  /*sst4 = NOT(ssx4)*/  
	asm volatile("vxorpd ymm12, ymm2, ymm5":::);  /*sst2 = NOT(ssx2)*/ 
	/* write back state*/ 
	asm volatile("vmovdqa %0, ymm0":"=m"(ssx0)::);  
	asm volatile("vmovdqa %0, ymm2":"=m"(ssx2)::);  
	asm volatile("vmovdqa %0, ymm4":"=m"(ssx4)::);  
	asm volatile("vmovdqa %0, ymm10":"=m"(sst0)::);  
	asm volatile("vmovdqa %0, ymm11":"=m"(sst1)::);  
	asm volatile("vmovdqa %0, ymm12":"=m"(sst2)::);  
	asm volatile("vmovdqa %0, ymm13":"=m"(sst3)::);  
	asm volatile("vmovdqa %0, ymm14":"=m"(sst4)::);  
	/*LOAD Random*/
	_4S_AND1(sst0, ssx1, sst0 ,0) 
	_4S_AND2(sst1, ssx2, sst1, 2) 
	_4S_AND1(sst2, ssx3, sst2, 4) 
	_4S_AND2(sst3, ssx4, sst3, 6) 
	_4S_AND1(sst4, ssx0, sst4, 8) 
	/*load state*/ 
	asm volatile("vmovdqa ymm0, %0"::"m"(ssx0):);  
	asm volatile("vmovdqa ymm11, %0"::"m"(sst1):);  
	asm volatile("vmovdqa ymm1, %0"::"m"(ssx1):);  
	asm volatile("vmovdqa ymm12, %0"::"m"(sst2):);  
	asm volatile("vmovdqa ymm2, %0"::"m"(ssx2):);  
	asm volatile("vmovdqa ymm13, %0"::"m"(sst3):);  
	asm volatile("vmovdqa ymm3, %0"::"m"(ssx3):);  
	asm volatile("vmovdqa ymm14, %0"::"m"(sst4):);  
	asm volatile("vmovdqa ymm4, %0"::"m"(ssx4):);  
	asm volatile("vmovdqa ymm10, %0"::"m"(sst0):);  
	asm volatile("vmovdqa ymm5, %0"::"m"((((__m256*)D)[0])):);  
	asm volatile("vxorpd ymm0, ymm0, ymm11":::);  /*XOR(ssx0, sst1)*/  
	asm volatile("vxorpd ymm1, ymm1, ymm12":::);  /*XOR(ssx1, sst2)*/  
	asm volatile("vxorpd ymm2, ymm2, ymm13":::);  /*XOR(ssx2, sst3)*/  
	asm volatile("vxorpd ymm3, ymm3, ymm14":::);  /*XOR(ssx3, sst4)*/  
	asm volatile("vxorpd ymm4, ymm4, ymm10":::);  /*XOR(ssx4, sst0)*/  
	asm volatile("vxorpd ymm1, ymm1, ymm0":::);  /*XOR(ssx1, ssx0)*/  
	asm volatile("vxorpd ymm0, ymm0, ymm4":::);  /*XOR(ssx0, ssx4)*/  
	asm volatile("vxorpd ymm3, ymm3, ymm2":::);  /*XOR(ssx3, ssx2)*/  
	asm volatile("vxorpd ymm2, ymm2, ymm5":::);  /*NOT(ssx2)*/  
	/*write back state*/
	asm volatile("vmovdqa %0, ymm4":"=m"(ssx4)::);  
	asm volatile("vmovdqa %0, ymm0":"=m"(ssx0)::);  
	asm volatile("vmovdqa %0, ymm1":"=m"(ssx1)::);  
	asm volatile("vmovdqa %0, ymm3":"=m"(ssx3)::);  
	asm volatile("vmovdqa %0, ymm2":"=m"(ssx2)::);  
	_4S_LDL1(ssx0, 19, 28) 
	_4S_LDL2(ssx1, 39, 61) 
	_4S_LDL3(ssx2, 1, 6) 
	_4S_LDL2(ssx3, 10, 17) 
	_4S_LDL1(ssx4, 7, 41) 
	/*end*/ }

void Init4Shares(uint64_t state[], uint64_t rand[]) {
	uint64_t tmp = state[0];
	_4S_LOAD(ssx0, tmp, rand);
	tmp = state[1];
	rand += 3;
	_4S_LOAD(ssx1, tmp, rand);
	tmp = state[2];
	rand += 3;
	_4S_LOAD(ssx2, tmp, rand);
	tmp = state[3];
	rand += 3;
	_4S_LOAD(ssx3, tmp, rand);
	tmp = state[4];
	rand += 3;
	_4S_LOAD(ssx4, tmp, rand);
}

void Get4Shares() {
	uint64_t res;
	_4S_GET(ssx0, res);
	printf("%.16llx ", res);
	_4S_GET(ssx1, res);
	printf("%.16llx ", res);
	_4S_GET(ssx2, res);
	printf("%.16llx ", res);
	_4S_GET(ssx3, res);
	printf("%.16llx ", res);
	_4S_GET(ssx4, res);
	printf("%.16llx\n", res);
}
#endif