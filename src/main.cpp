#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <numeric>
#include "taniask.h"
#include "random/RandomBuffer.h"
#include <cmath>



// ---- cycle counter ----
static inline uint64_t getCycles() {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline double benchCycles(void (*fn)(void), size_t count) {
    uint64_t before = getCycles();
    for (size_t i = 0; i < count; i++) {
        fn();
    }
    uint64_t after = getCycles();
    return (double)(after - before) / count;
}


int main(int argc, char* argv[]) {
    size_t count = (argc >= 3) ? strtoul(argv[2], NULL, 10) : 1000000;
    size_t N = (argc >= 2) ? strtoul(argv[1], NULL, 10) : 128;

    uint8_t* D_seed = (uint8_t*)malloc(N); 
    uint8_t* P_seed = (uint8_t*)malloc(N); 
    uint8_t* U_seed = (uint8_t*)malloc(N); 
    uint8_t* Y = (uint8_t*)malloc(N*N); 
    uint8_t* Zt = (uint8_t*)malloc(N*N); 
    uint8_t* input = (uint8_t*)malloc(N*N); 
    uint8_t* output = (uint8_t*)malloc(N*N);

    if (!(D_seed && P_seed && U_seed && Y && Zt && input && output)) {
        printf("malloc failed!!\n");
        exit(1);
    }

    // Initialize cipher with seeds
    RandomBuffer rng = RandomBuffer(4096);
    rng.GetBytes(D_seed, N);
    rng.GetBytes(P_seed, N);
    rng.GetBytes(U_seed, N);
    
    TANIASK_set_D(D_seed);
    TANIASK_set_P(P_seed);
    TANIASK_set_U(U_seed);

    // Warmup
    rng.GetBytes(input, N*N);
    TANIASK_encrypt(input, Y, Zt);
    // TANIASK_decrypt(Y, Zt, output);
    

    std::vector<uint64_t> cycles(count);
    for (size_t i = 0; i < count; i++) {
        rng.GetBytes(input, N*N); 
	
	if (i % 50 == 0 || 1) {
		printf("Encryption no. %d out of %d. %f%", i, count, (double)i/count);	
	}
        uint64_t before = getCycles();
        TANIASK_encrypt(input, Y, Zt);
        uint64_t after = getCycles();

        uint64_t cycle = after - before;
        cycles[i] = cycle;
    }
   
    uint64_t minCycles = cycles[0];
    uint64_t maxCycles = cycles[0];
    double sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        if (cycles[i] < minCycles) minCycles = cycles[i];
        if (cycles[i] > maxCycles) maxCycles = cycles[i];
        sum += cycles[i];
    }
    double average = sum/count;

    double squareSum = 0.0;
    for (size_t i = 0; i < count; i++) {
        double diff = cycles[i] - average;
        squareSum += diff * diff;
    }
    double stddev = std::sqrt(squareSum / count);

    printf("Encryption statistics:\n");
    printf("  mean: %.1f\n", average);
    printf("  min:  %llu\n", minCycles);
    printf("  max:  %llu\n", maxCycles);
    printf("  std:  %.1f\n", stddev);
    printf("  cycle/bit: %.1f\n", average/(N*N));
    uint64_t before = getCycles();
    for (size_t i = 0; i < count; i++) {
	    TANIASK_decrypt(Y, Zt, output);
    }
    uint64_t after = getCycles();
    double avg = (double)(after-before)/count;
    
    printf("Decryption statistics:\n");
    printf("  mean: %.1f\n", avg);
    printf("  cycle/bit: %1.f\n", avg/(N*N));

    // Print sample output (first run only)
    printf("Sample ciphertext Y: ");
    for (int i = 0; i < N; i++) printf("%02x", Y[i]);
    printf("\n");
    
    printf("Sample ciphertext Zt: ");
    for (int i = 0; i < N; i++) printf("%02x", Zt[i]);
    printf("\n");
   
    printf("Sample decrypted output: ");
    for (int i = 0; i < N; i++) printf("%02x", output[i]);
    printf("\n");

    free(D_seed);
    free(P_seed);
    free(U_seed);
    free(Y);
    free(Zt);
    free(input);
    free(output);

   

    return 0;
}
