#include<stdio.h>
#include<stdlib.h>
#include <immintrin.h>
#include<cstdint>

uint64_t getCycles() {
	uint32_t lo, hi;
	asm volatile ("rdtsc":"=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
	//#ifdef __X86_64__
		//asm volatile ("shl %0, 32":"=d"(hi));
		//asm volatile ("or %0, %1":"=d"(low):"d"(hi));
	//#endif
}

void bench_datapath(long sz = 10000) {
	uint64_t time0;
	uint64_t* in = new uint64_t[sz * 4];
	uint64_t* out = new uint64_t[sz * 4];
	__m128i* in1 = (__m128i*)in;
	__m128i* out1 = (__m128i*)out;
	__m256i* in2 = (__m256i*)in;
	__m256i* out2 = (__m256i*)out;
	#if defined(__AVX512F__)
	__m512i* in3 = (__m512i*)in;
	__m512i* out3 = (__m512i*)out;
	#endif
	//printf(">>> begin bench\n");
	time0 = getCycles();
	for (long i = 0; i < sz * 4; ++i)
	{
		out[i] = in[i];
	}
	printf("Nat: %lu\n", getCycles() - time0);

	time0 = getCycles();
	for (long i = 0; i < sz * 2; ++i)
	{
		asm("vmovdqu xmm0, %0"::"m"(in1[i]) : );
		asm("vmovdqu %0, xmm0":"=m"(out1[i])::);
	}
	printf("Xmm: %lu\n", getCycles() - time0);
	time0 = getCycles();
	for (long i = 0; i < sz; ++i)
	{
		asm("vmovdqu ymm0, %0"::"m"(in2[i]) : );
		asm("vmovdqu %0, ymm0":"=m"(out2[i])::);
	}
	printf("Ymm: %lu\n", getCycles() - time0);
	#if defined(__AVX512F__)
	time0 = getCycles();
	for (long i = 0; i < sz/2; ++i)
	{
		asm("vmovdqu64 zmm0, %0"::"m"(in3[i]) : );
		asm("vmovdqu64 %0, zmm0":"=m"(out3[i])::);
	}
	printf("Zmm: %lu\n", getCycles() - time0);
	#endif
	//printf(">>> end bench\n");
	delete[] in;
	delete[] out;
}

int main(int argc, char* argv[]) {
	long sz = 2<<10;
	if (argc >=2 && strtol(argv[1], 0, 10))
		sz = strtol(argv[1], 0, 10);
	bench_datapath(sz);
}