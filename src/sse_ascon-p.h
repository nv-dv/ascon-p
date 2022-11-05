#pragma once
#include<cstdint>
#include <immintrin.h>
// state words x0..x4 (uint64), temporary variables t0..t4 (uint64)
// 0
//x2 ^= c
//x0 ^= x4; //1 x4 ^= x3;    x2 ^= x1;
// 2
//t0 = ~x0;    t1 = ~x1;    t2 = ~x2;    t3 = ~x3;    t4 = ~x4;
// 3
//t0 &= x1;    t1 &= x2;    t2 &= x3;    t3 &= x4;    t4 &= x0;
// 4
//x0 ^= t1;    x1 ^= t2;    x2 ^= t3;    x3 ^= t4;    x4 ^= t0;
// 5
//x0 ^= x4;    x1 ^= x0;    x2 = ~x2;    x3 ^= x2;
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

//#include<stdio.h>

#ifndef CONSTS
	#define CONSTS
	uint64_t C_not = 0xffffffffffffffff;
	uint64_t C;
	RandomBuffer randbuf = RandomBuffer(500 * 2<<20);;
	#define TYPE_UINT64 8
	#define TYPE_UINT128 16
	#define TYPE_UINT256 32
#endif

#define INIT \
	uint64_t x[5];\
	uint64_t t[5];\

#define _LOADXMM(from) \
	asm("movdqu xmm0, %0"::"m"(((__m128*)from)[0]):);  \
	asm("movdqu xmm1, %0"::"m"(((__m128*)from)[1]):);  \
	asm("movlpd xmm2, %0"::"m"(from[4]):);  \

# define _STOREXMM(buf) \
	asm("movdqu %0, xmm0":"=m"(((__m128*)buf)[0])::);  \
	asm("movdqu %0, xmm1":"=m"(((__m128*)buf)[1])::);  \
	asm("movlpd %0, xmm2":"=m"(buf[4])::);  \

#define _ROTR(x, i) \
	asm("movq xmm0, %0"::"m"(x):);  \
	asm("movq xmm1, %0"::"m"(x):);  \
	asm("psrlq xmm0, %0"::"i"(i):);  \
	asm("psllq xmm1, %0"::"i"(64-i):);  \
	asm("por xmm0, xmm1":::);  \

#define _LDL(i, rot1, rot2) \
	_ROTR(x[i], rot1); \
	asm("movdqa xmm2, xmm0":::);  \
	_ROTR(x[i], rot2); \
	asm("movdqa xmm3, xmm0":::);  \
	asm("xorpd xmm2, xmm3":::);  \
	asm("movq xmm3, %0"::"m"(x[i]):);  \
	asm("xorpd xmm2, xmm3":::);  \
	asm("movq %0, xmm2":"=m"(x[i])::);  \

// C must be a uint64_t holding a const
#define _ROUND(c) \
	C = c;\
		/*0*/\
		asm("movhpd xmm0, %0"::"m"(x[2]):);  \
		asm("movlpd xmm0, %0"::"m"(x[0]):);  \
		asm("movhpd xmm1, %0"::"m"(C):);  \
		asm("movlpd xmm1, %0"::"m"(x[4]):);  \
		asm("xorpd xmm0, xmm1":::);  \
		asm("movhpd %0, xmm0":"=m"(x[2])::);  \
		asm("movlpd %0, xmm0":"=m"(x[0])::);  \
		/*1*/\
		asm("movhpd xmm2, %0"::"m"(x[4]):);  \
		asm("movlpd xmm2, %0"::"m"(x[2]):);  \
		asm("movhpd xmm3, %0"::"m"(x[3]):);  \
		asm("movlpd xmm3, %0"::"m"(x[1]):);  \
		asm("xorpd xmm2, xmm3":::);  \
		asm("movhpd %0, xmm2":"=m"(x[4])::);  \
		asm("movlpd %0, xmm2":"=m"(x[2])::);  \
	/*2*/\
	for (size_t i = 0; i < 10; ++i) ((size_t*)t)[i] = ~((size_t*)x)[i];\
	/*3*/\
		_LOADXMM(t); \
		/*offset'd load*/ \
		asm("movdqu xmm3, %0"::"m"(((__m128*)(x+1))[0]):);  \
		asm("movdqu xmm4, %0"::"m"(((__m128*)(x+3))[0]):);  \
		asm("movlpd xmm5, %0"::"m"(x[0]):);  \
		asm("andpd xmm0, xmm3");  \
		asm("andpd xmm1, xmm4");  \
		asm("andpd xmm2, xmm5");  \
		_STOREXMM(t); \
	/*4*/\
		_LOADXMM(x); \
		asm("movdqu xmm3, %0"::"m"(((__m128*)(t+1))[0]):);  \
		asm("movdqu xmm4, %0"::"m"(((__m128*)(t+3))[0]):);  \
		asm("movlpd xmm5, %0"::"m"(t[0]):);  \
		asm("xorpd xmm0, xmm3");  \
		asm("xorpd xmm1, xmm4");  \
		asm("xorpd xmm2, xmm5");  \
		_STOREXMM(x); \
	/*5*/\
		asm("movdqu xmm0, %0"::"m"(((__m128*)x)[0]):);  \
		asm("movlpd xmm2, %0"::"m"(x[4]):);  \
		asm("movhpd xmm2, %0"::"m"(x[0]):);  \
		asm("xorpd xmm0, xmm2");  \
		asm("movdqu xmm1, %0"::"m"(((__m128*)x)[1]):);  \
		asm("movq xmm3, %0"::"m"(C_not):);  \
		asm("movhpd xmm3, %0"::"m"(x[2]):);  \
		asm("xorpd xmm1, xmm3");  \
		asm("movdqu %0, xmm0":"=m"(((__m128*)x)[0])::);  \
		asm("movdqu %0, xmm1":"=m"(((__m128*)x)[1])::);  \
	/*6*/\
		_LDL(0, 19, 28); \
		_LDL(1, 39, 61); \
		_LDL(2, 1, 6); \
		_LDL(3, 10, 17); \
		_LDL(4, 7, 41); \
/*end*/

#define _P12 \
    _ROUND(0xf0);\
    _ROUND(0xe1);\
    _ROUND(0xd2);\
    _ROUND(0xc3);\
    _ROUND(0xb4);\
    _ROUND(0xa5);\
    _ROUND(0x96);\
    _ROUND(0x87);\
    _ROUND(0x78);\
    _ROUND(0x69);\
    _ROUND(0x5a);\
    _ROUND(0x4b);\


#define _P6 \
    _ROUND(0x96);\
    _ROUND(0x87);\
    _ROUND(0x78);\
    _ROUND(0x69);\
    _ROUND(0x5a);\
    _ROUND(0x4b);\


#define _P1 \
    _ROUND(0x4b);\

