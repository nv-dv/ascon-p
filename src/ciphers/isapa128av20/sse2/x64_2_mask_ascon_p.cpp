#ifdef __x86_64__
#include "ciphers/isapa128av20/sse2/2_mask_ascon_p.h"
#include "gadgets/and_gate/sse2/x64/2_mask_and_gate.h"
#include "gadgets/ldl_gate/sse2/x64/2_mask_ldl_gate.h"
#include "gadgets/state/sse2/2_mask_state.h"

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

uint64_t R2[10];
share_2 sx0;
share_2 sx1;
share_2 sx2;
share_2 sx3;
share_2 sx4;
share_2 st0;
share_2 st1;
share_2 st2;
share_2 st3;
share_2 st4;

void _2S_ROUND(uint64_t c) {
	/*LOAD R*/
	randbuf.GetBytes((uint8_t*)R2, sizeof(R2)); 
	C = c;
	asm volatile("vmovq xmm11, %0"::"m"(C):);  
	/*0*/
	asm volatile("vmovdqu xmm0, %0"::"m"(sx0):);  
	asm volatile("vmovdqu xmm4, %0"::"m"(sx4):);  
	asm volatile("vmovdqu xmm2, %0"::"m"(sx2):);  
	asm volatile("vmovdqu xmm3, %0"::"m"(sx3):);  
	asm volatile("vmovdqu xmm1, %0"::"m"(sx1):);  
	asm volatile("movq xmm5, %0"::"m"(C_not):);  
	asm volatile("xorpd xmm2, xmm11":::);  
	asm volatile("xorpd xmm0, xmm4":::);  
	asm volatile("xorpd xmm4, xmm3":::);  
	asm volatile("vmovdqu %0, xmm1":"=m"(sx1)::);  
	asm volatile("xorpd xmm2, xmm1":::);  
	/*2*/
	/*_2S_NOT(sx0)*/ 
	asm volatile("vxorpd xmm10, xmm0, xmm5":::);  
	/*_2S_NOT(sx1)*/  
	asm volatile("vxorpd xmm11, xmm1, xmm5":::);  
	/*_2S_NOT(sx2)*/ 
	asm volatile("vxorpd xmm12, xmm2, xmm5":::);  
	/*_2S_NOT(sx3)*/ 
	asm volatile("vxorpd xmm13, xmm3, xmm5":::);  
	/*_2S_NOT(sx4)*/ 
	asm volatile("vmovdqu %0, xmm0":"=m"(sx0)::);  
	asm volatile("vmovdqu %0, xmm4":"=m"(sx4)::);  
	asm volatile("vmovdqu %0, xmm2":"=m"(sx2)::);  
	asm volatile("vxorpd xmm14, xmm4, xmm5":::);  
	asm volatile("movdqu %0, xmm10":"=m"(st0)::);  
	asm volatile("movdqu %0, xmm11":"=m"(st1)::);  
	asm volatile("movdqu %0, xmm12":"=m"(st2)::);  
	asm volatile("movdqu %0, xmm13":"=m"(st3)::);  
	asm volatile("movdqu %0, xmm14":"=m"(st4)::);  
	/*3*/
	_2S_AND1(st0, sx1, st0 ,0) 
	_2S_AND2(st1, sx2, st1, 2) 
	_2S_AND1(st2, sx3, st2, 4) 
	_2S_AND2(st3, sx4, st3, 6) 
	_2S_AND1(st4, sx0, st4, 8) 
	/*4*/
	/*_2S_XOR(sx0, st1)*/ 
	asm volatile("movdqu xmm0, %0"::"m"(sx0):);  
	asm volatile("movdqu xmm1, %0"::"m"(st1):);  
	asm volatile("movdqu xmm2, %0"::"m"(sx1):);  
	asm volatile("movdqu xmm3, %0"::"m"(st2):);  
	asm volatile("movdqu xmm4, %0"::"m"(sx2):);  
	asm volatile("movdqu xmm5, %0"::"m"(st3):);  
	asm volatile("movdqu xmm6, %0"::"m"(sx3):);  
	asm volatile("movdqu xmm7, %0"::"m"(st4):);  
	asm volatile("movdqu xmm8, %0"::"m"(sx4):);  
	asm volatile("movdqu xmm9, %0"::"m"(st0):);  
	asm volatile("movq xmm10, %0"::"m"(C_not):);  
	asm volatile("xorpd xmm0, xmm1":::);  
	/*_2S_XOR(sx2, st3)*/ 
	asm volatile("xorpd xmm4, xmm5":::);  
	/*_2S_XOR(sx3, st4)*/ 
	asm volatile("xorpd xmm6, xmm7":::);  
	/*_2S_XOR(sx4, st0)*/ 
	asm volatile("xorpd xmm8, xmm9":::);  
	/*_2S_XOR(sx1, st2)*/ 
	asm volatile("xorpd xmm2, xmm3":::);  
	/*5*/
	/*_2S_XOR(sx3, sx2)*/ 
	asm volatile("xorpd xmm6, xmm4":::);  
	/*_2S_XOR(sx1, sx0)*/ 
	asm volatile("xorpd xmm2, xmm0":::);  
	/*_2S_XOR(sx0, sx4)*/ 
	asm volatile("xorpd xmm0, xmm8":::);  
	/*_2S_NOT(sx2)*/ 
	asm volatile("vxorpd xmm4, xmm4, xmm10":::);  
	asm volatile("movdqu %0, xmm2":"=m"(sx1)::);  
	asm volatile("movdqu %0, xmm0":"=m"(sx0)::);  
	asm volatile("movdqu %0, xmm8":"=m"(sx4)::);  
	asm volatile("movdqu %0, xmm6":"=m"(sx3)::);  
	asm volatile("movdqu %0, xmm4":"=m"(sx2)::);  
	/*6*/
	_2S_LDL1(sx0, 19, 28) 
	_2S_LDL2(sx1, 39, 61) 
	_2S_LDL3(sx2, 1, 6) 
	_2S_LDL2(sx3, 10, 17) 
	_2S_LDL1(sx4, 7, 41) 
	/*end*/
}

void Init2Shares(uint64_t state[], uint64_t rand[]) {
	uint64_t tmp = state[0];
	uint64_t rnd = rand[0];
	_2S_LOAD(sx0, tmp, rnd);
	tmp = state[1];
	rnd = rand[1];
	_2S_LOAD(sx1, tmp, rnd);
	tmp = state[2];
	rnd = rand[2];
	_2S_LOAD(sx2, tmp, rnd);
	tmp = state[3];
	rnd = rand[3];
	_2S_LOAD(sx3, tmp, rnd);
	tmp = state[4];
	rnd = rand[4];
	_2S_LOAD(sx4, tmp, rnd);
}

void Get2Shares() {
	uint64_t res;
	_2S_GET(sx0, res);
	printf("%.16llx ", res);
	_2S_GET(sx1, res);
	printf("%.16llx ", res);
	_2S_GET(sx2, res);
	printf("%.16llx ", res);
	_2S_GET(sx3, res);
	printf("%.16llx ", res);
	_2S_GET(sx4, res);
	printf("%.16llx\n", res);
}
#endif