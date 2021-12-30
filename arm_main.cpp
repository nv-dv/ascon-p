// ascon-p.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "RandomBuffer/RandomBuffer.h"

#include "d_mask/dmask_ascon-p.h"
#include "isapa128av20/opt_64/isap.h"
#include "consts.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define CpB "\nCycles / bit: %f\n"
#define CPU_CLK 1.6*1000*1000*1000

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
//(? ? ? ) x2 = not x2	

extern void bench_speed(size_t count);
extern void usuba_get_shares();

unsigned long getCycles() {
    //asm volatile ("MRC p15, #0, R0, c9, c13, #0":);
    return (unsigned long)clock();
}

unsigned long inline getInstructionPointer() {
	unsigned long pc;
	#if defined(__arm__)
    asm volatile("MOV %0, pc" : "=r"(pc));
    #endif
    #if defined(__aarch64__)
    asm volatile("ADR %0, ." : "=r"(pc));
    #endif
    return pc;
}

#define BENCH_CODESZ(func, offset) \
	eip = getInstructionPointer(); \
	func; \
	eip = getInstructionPointer()-eip; \
	printf("\ncode-size: %lu", eip); \

int main(int argc, char* argv[])
{
	// enable counter...
	//printf("clock @ %fMHz\n", double(CLOCKS_PER_SEC/1000000));
	uint64_t x[5];
	size_t count;
	int slim = 0;
	if (argc>=2 && argv[1][0]=='1')
		slim = 1;
	if (argc>=3 && strtol(argv[2], 0, 10)) {
		count = strtol(argv[2], 0, 10);
	}
	else {
		count = COUNT;
	}
	uint64_t before;
	unsigned long eip;
	uint64_t rand[100];
	//printf("-----begin testbed-----\n");
	for (size_t i = 0; i < 100; i++)
	{
		rand[i] = 0x4141414141414141+i;
	}
	//printf("-----end testbed-----\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	// -----------------------------
	if (!slim) {
	}
	randbuf.LoadMax();
	printf("C generic masking(d=%d)", MASKING_ORDER);
	BENCH_CODESZ(dS_P12, 0);
	for (size_t i = 0; i < 5; i++)
	{
		x[i] = i;
	}
	InitDShares(x, rand);
	before = getCycles();
	for (size_t i = 0; i < count; i++)
	{
		dS_P12;
	}
	printf(CpB, (double)(getCycles() - before) / count * (CPU_CLK / CLOCKS_PER_SEC) / BITS);
	GetDShares();
	printf(">>>> %u\n", randbuf.GetSz()-randbuf.GetRdy());

	randbuf.LoadMax();
	printf("usuba generic masking(d=%d)", MASKING_ORDER);
	printf("\ncode-size: 0");
	before = getCycles();
	for (size_t i = 0; i < count; i++)
	{
		bench_speed(i);
	}
	printf(CpB, (double)(getCycles() - before) / count * (CPU_CLK / CLOCKS_PER_SEC) / BITS);
	usuba_get_shares();
	printf(">>>> %u\n", randbuf.GetSz()-randbuf.GetRdy());
	if (!slim) {
		printf("opt_64");
		BENCH_CODESZ(P12, 0);
	    x0 = 0;
	    x1 = 1;
	    x2 = 2;
	    x3 = 3;
	    x4 = 4;
	    before = getCycles();
	    for (size_t i = 0; i < count; i++)
	    {
	        P12;
	    }
	    printf(CpB, (double)(getCycles() - before) / count * (CPU_CLK / CLOCKS_PER_SEC) / BITS);
	    printf("%.16llx %.16llx %.16llx %.16llx %.16llx\n", x0, x1, x2, x3, x4);
	    printf(">>>> 0\n");
	}
    return 0;
}
