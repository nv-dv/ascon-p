#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "aes.h"
#include "taniask.h"
#include "random/RandomBuffer.h"


// ---- cycle counter ----
#include <stdint.h>

static inline uint64_t getCycles(void) {
    uint64_t cc = 0;

#if defined(__i386__) || defined(__x86_64__)
    // ---- x86/x86_64 ----
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    cc = ((uint64_t)hi << 32) | lo;

#elif defined(__aarch64__)
    // ---- ARMv8 / AArch64 ----
    asm volatile("mrs %0, cntvct_el0" : "=r"(cc));

#elif defined(__arm__)
    // ---- ARMv7 / 32-bit ARM ----
    // priveleged instructions - emulator doesnt work :(

    uint32_t cclo;
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(cclo));
    cc = (uint64_t)cclo;

#else
    #error "getCycles not supported on this architecture"
#endif

    return cc;
}

static inline double benchCycles(void (*fn)(void), size_t count) {
    uint64_t before = getCycles();
    for (size_t i = 0; i < count; i++) {
        fn();
    }
    uint64_t after = getCycles();
    return (double)(after - before) / count;
}

size_t N = 64;
int main(int argc, char* argv[]) {
    size_t count = (argc >= 3) ? strtoul(argv[2], NULL, 10) : 1000;
    N = (argc >= 2) ? strtoul(argv[1], NULL, 10) : 64;
    RandomBuffer rng = RandomBuffer(4096);

    size_t B_bits = N*N;
    size_t B_bytes = (B_bits + 7)/8;

    // TANIA-SK Benchmark
    uint8_t* D_seed = (uint8_t*)malloc(N); 
    uint8_t* P_seed = (uint8_t*)malloc(N); 
    uint8_t* U_seed = (uint8_t*)malloc(N); 
    uint8_t* Y = (uint8_t*)malloc(B_bits); 
    uint8_t* Zt = (uint8_t*)malloc(B_bits); 
    uint8_t* input = (uint8_t*)malloc(B_bits); 
    uint8_t* output = (uint8_t*)malloc(B_bits);

    if (!(D_seed && P_seed && U_seed && Y && Zt && input && output)) {
        printf("malloc failed!!\n");
        exit(1);
    }

    rng.GetBytes(D_seed, N);
    rng.GetBytes(P_seed, N);
    rng.GetBytes(U_seed, N);
    
    TANIASK_set_D(D_seed);
    TANIASK_set_P(P_seed);
    TANIASK_set_U(U_seed);

    uint64_t sumCycles = 0;
    for (size_t i = 0; i < count; i++) {
        rng.GetBytes(input, B_bits); 
	
        uint64_t cycle = TANIASK_encrypt_cycles(input, Y, Zt);
        sumCycles += cycle;
    }
   
    double average = (double)sumCycles/count;

    printf("Encryption statistics:\n");
    printf("  mean: %.1f\n", average);
    printf("  cycle/bit: %.1f\n", average/(B_bits));

    // AES-128 CTR benchmark using TinyAES
    uint8_t aes_key[16];
    rng.GetBytes(aes_key, 16);

    uint8_t* aes_input = (uint8_t*)malloc(B_bytes);
    uint8_t* aes_output = (uint8_t*)malloc(B_bytes);
    if (!aes_input || !aes_output) {
        printf("malloc failed!!\n");
        exit(1);
    }

    struct AES_ctx ctx;
    AES_init_ctx(&ctx, aes_key);

    uint64_t aes_sumCycles = 0;

    for (size_t i = 0; i < count; i++) {
        rng.GetBytes(aes_input, B_bytes);

        uint64_t start_cycles = getCycles();
        AES_CTR_xcrypt_buffer(&ctx, aes_input, B_bytes);
        uint64_t end_cycles = getCycles();

        aes_sumCycles += (end_cycles - start_cycles);
    }

    double aes_average = (double)aes_sumCycles / count;
    printf("AES-128 CTR encryption statistics:\n");
    printf("  mean: %.1f\n", aes_average);
    printf("  cycle/bit: %.1f\n", aes_average/(B_bits));

    free(D_seed);
    free(P_seed);
    free(U_seed);
    free(Y);
    free(Zt);
    free(input);
    free(output);
    free(aes_input);
    free(aes_output);

   

    return 0;
}
