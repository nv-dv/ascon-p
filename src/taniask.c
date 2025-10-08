
/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/
#include <stdint.h>
#include "taniask.h"
#include "matrix.h"

/*****************************************************************************/
/* Defines:                                                                  */
/*****************************************************************************/

// Matrix size in bits (key_length+1)
extern size_t N;

/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/

// The matrices which store the keys
static matrix D;
static matrix DInverse;
static matrix P;
static matrix PInverse;
static matrix U;
static matrix UInverse;

/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/
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

static uint64_t rndword(void *data) {
    uint64_t val = 0;
    for (int i = 0; i < 8; i++) {
        val = (val << 8) ^ (rand() & 0xFF);
    }
    return val;
}

// Cipher is the main function that encrypts the PlainText.
static void Cipher(matrix plaintext, matrix Y, matrix Zt)
{

    // Generate Z
    matrix Z = mzd_init(N, N);
    mzd_randomize_custom(Z, rndword, NULL);

    matrix Ucol = mzd_submatrix(NULL, U, 1, N-1, N, N);
    matrix Zrow = mzd_submatrix(NULL, Z, N-1, 0, N, N-1);
    matrix znn = mzd_mul(NULL, Zrow, Ucol, 0);

    BIT bit = mzd_read_bit(znn, 0, 0);
    mzd_write_bit(Z, N-1, N-1, bit);

    mzd_free(znn);
    mzd_free(Ucol);
    mzd_free(Zrow);

    mzd_mul(Zt, P, Z, 0);

    // Calculate Y~
    matrix I = mzd_init(N, N);
    mzd_set_ui(I, 1); 

    matrix u = mzd_init(N, 1);
    mzd_submatrix(u, U, 1, N-1, N, N);
    mzd_write_bit(u, N-1, 0, 1);

    matrix vt = mzd_init(1, N);
    mzd_submatrix(vt, Z, N-1, 0, N, N);

    matrix uv_T = mzd_mul(NULL, u, vt, 0);
    matrix iuv_T = mzd_add(NULL, I, uv_T);

    matrix temp1 = mzd_init(N, N);
    matrix temp2 = mzd_init(N, N);
    mzd_mul(temp1 ,plaintext, U, 0);
    mzd_mul(temp2, temp1, iuv_T, 0);
    mzd_mul(temp1, temp2, D, 0);
    mzd_mul(temp2, temp1, iuv_T, 0);
    mzd_mul(Y, temp2, UInverse, 0);

    mzd_free(temp1);
    mzd_free(temp2);
    mzd_free(I);
    mzd_free(u);
    mzd_free(vt);
    mzd_free(uv_T);
    mzd_free(iuv_T);
    mzd_free(Z);
}

static void InvCipher(matrix Yin, matrix Ztin, matrix M)
{
    matrix Z = mzd_init(N, N);
    mzd_mul(Z, PInverse, Ztin, 0);

    matrix I = mzd_init(N, N);
    mzd_set_ui(I, 1); 

    matrix u = mzd_init(N, 1);
    mzd_submatrix(u, U, 1, N-1, N, N);
    mzd_write_bit(u, N-1, 0, 1);

    matrix vt = mzd_init(1, N);
    mzd_submatrix(vt, Z, N-1, 0, N, N);

    matrix uv_T = mzd_mul(NULL, u, vt, 0);
    matrix iuv_T = mzd_add(NULL, I, uv_T);

    matrix temp1 = mzd_init(N, N);
    matrix temp2 = mzd_init(N, N);
    mzd_mul(temp1, Yin, U, 0);
    mzd_mul(temp2, temp1, iuv_T, 0);
    mzd_mul(temp1, temp2, DInverse, 0);
    mzd_mul(temp2, temp1, iuv_T, 0);
    mzd_mul(M, temp2, UInverse, 0);

    mzd_free(temp1);
    mzd_free(temp2);
    mzd_free(I);
    mzd_free(u);
    mzd_free(vt);
    mzd_free(uv_T);
    mzd_free(iuv_T);
    mzd_free(Z);

}

// void SetD(uint8_t* seed)
// {
//     matrix tempD = CCF(seed, N);
//     matrix tempDInverse = ICCF(seed, N);
//     D = tempD;
//     DInverse = tempDInverse;
// }

// void SetP(uint8_t* seed)
// {
//     matrix tempP = CCF(seed, N);
//     matrix tempPInverse = ICCF(seed, N);
//     P = tempP;
//     PInverse = tempPInverse;
// }

// void SetU(uint8_t* seed)
// {
//     matrix tempU = CCF(seed, N);
//     matrix tempUInverse = ICCF(seed, N);
//     U = tempU;
//     UInverse = tempUInverse;
// }

/*****************************************************************************/
/* Public functions:                                                         */
/*****************************************************************************/



// void TANIASK_set_keys(uint8_t* dseed, uint8_t* pseed, uint8_t* useed)
// {
//     SetD(dseed);
//     SetP(pseed);
//     SetU(useed);
// }

void TANIASK_set_D(uint8_t* seed)
{
    D = CCF(seed, N);
    DInverse = ICCF(seed, N);
}

void TANIASK_set_P(uint8_t* seed)
{
    P = CCF(seed, N);
    PInverse = ICCF(seed, N);
}

void TANIASK_set_U(uint8_t* seed)
{
    U = CCF(seed, N);
    UInverse = ICCF(seed, N);
}

void TANIASK_set_rng_seed(unsigned int seed)
{
    srand(seed);
}

void TANIASK_encrypt(uint8_t* input, uint8_t* Youtput, uint8_t* Ztoutput)
{
  matrix plaintext = matrix_from_array(N, N, input);

  matrix Y = mzd_init(N, N);
  matrix Zt = mzd_init(N, N);
  Cipher(plaintext, Y, Zt);

  array_from_matrix(Y,Youtput);
  array_from_matrix(Zt,Ztoutput);

  mzd_free(plaintext);
  mzd_free(Y);
  mzd_free(Zt);
}

uint64_t TANIASK_encrypt_cycles(uint8_t* input, uint8_t* Youtput, uint8_t* Ztoutput)
{
  matrix plaintext = matrix_from_array(N, N, input);

  matrix Y = mzd_init(N, N);
  matrix Zt = mzd_init(N, N);

  uint64_t before = getCycles();
  Cipher(plaintext, Y, Zt);
  uint64_t after = getCycles();

  array_from_matrix(Y,Youtput);
  array_from_matrix(Zt,Ztoutput);

  mzd_free(plaintext);
  mzd_free(Y);
  mzd_free(Zt);

  return after-before;
}

void TANIASK_decrypt(uint8_t* Yinput, uint8_t* Ztinput, uint8_t* output)
{
    matrix Y = matrix_from_array(N, N, Yinput);
    matrix Zt = matrix_from_array(N, N, Ztinput);

    matrix plaintext = mzd_init(N, N);
    InvCipher(Y, Zt, plaintext);
  
    array_from_matrix(plaintext, output);

    mzd_free(plaintext);
    mzd_free(Y);
    mzd_free(Zt);
}


