#pragma once

#ifndef ISAP_H
typedef unsigned char u8;
typedef unsigned long long u64;
typedef unsigned long u32;
typedef long long i64;
#define ISAP_H
// Rate in bits
#define ISAP_rH 64
#define ISAP_rB 1

// Number of rounds
#define ISAP_sH 12
#define ISAP_sB 1
#define ISAP_sE 6
#define ISAP_sK 12

// State size in bytes
#define ISAP_STATE_SZ 40

// Size of rate in bytes
#define ISAP_rH_SZ ((ISAP_rH+7)/8)

// Size of zero truncated IV in bytes
#define ISAP_IV_SZ 8

// Size of tag in bytes
#define ISAP_TAG_SZ 16

// Security level
#define ISAP_K 128

#define RATE (64 / 8)
#define PA_ROUNDS 12
#define PB_ROUNDS 6

const u8 ISAP_IV1[] = {0x01,ISAP_K,ISAP_rH,ISAP_rB,ISAP_sH,ISAP_sB,ISAP_sE,ISAP_sK};
const u8 ISAP_IV2[] = {0x02,ISAP_K,ISAP_rH,ISAP_rB,ISAP_sH,ISAP_sB,ISAP_sE,ISAP_sK};
const u8 ISAP_IV3[] = {0x03,ISAP_K,ISAP_rH,ISAP_rB,ISAP_sH,ISAP_sB,ISAP_sE,ISAP_sK};
static const int _R[5][2] = {
    {19, 28}, {39, 61}, {1, 6}, {10, 17}, {7, 41}
};
u64 x0, x1, x2, x3, x4;
u64 t0, t1, t2, t3, t4;
u32 rem_bytes;
u64* src64;
u32 idx64;

#define ROTR(x,n) (((x)>>(n))|((x)<<(64-(n))))
#define EXT_BYTE(x,n) ((u8)((u64)(x)>>(8*(7-(n)))))
#define INS_BYTE(x,n) ((u64)(x)<<(8*(7-(n))))

#define U64BIG(x) \
    ((ROTR(x, 8) & (0xFF000000FF000000ULL)) | \
     (ROTR(x,24) & (0x00FF000000FF0000ULL)) | \
     (ROTR(x,40) & (0x0000FF000000FF00ULL)) | \
     (ROTR(x,56) & (0x000000FF000000FFULL)))

void ROUND(u64 C) {\
    x2 ^= C;\
    x0 ^= x4;\
    x4 ^= x3;\
    x2 ^= x1;\
    t0 = x0;\
    t4 = x4;\
    t3 = x3;\
    t1 = x1;\
    t2 = x2;\
    x0 = t0 ^ ((~t1) & t2);\
    x2 = t2 ^ ((~t3) & t4);\
    x4 = t4 ^ ((~t0) & t1);\
    x1 = t1 ^ ((~t2) & t3);\
    x3 = t3 ^ ((~t4) & t0);\
    x1 ^= x0;\
    t1  = x1;\
    x1 = ROTR(x1, _R[1][0]);\
    x3 ^= x2;\
    t2  = x2;\
    x2 = ROTR(x2, _R[2][0]);\
    t4  = x4;\
    t2 ^= x2;\
    x2 = ROTR(x2, _R[2][1] - _R[2][0]);\
    t3  = x3;\
    t1 ^= x1;\
    x3 = ROTR(x3, _R[3][0]);\
    x0 ^= x4;\
    x4 = ROTR(x4, _R[4][0]);\
    t3 ^= x3;\
    x2 ^= t2;\
    x1 = ROTR(x1, _R[1][1] - _R[1][0]);\
    t0  = x0;\
    x2 = ~x2;\
    x3 = ROTR(x3, _R[3][1] - _R[3][0]);\
    t4 ^= x4;\
    x4 = ROTR(x4, _R[4][1] - _R[4][0]);\
    x3 ^= t3;\
    x1 ^= t1;\
    x0 = ROTR(x0, _R[0][0]);\
    x4 ^= t4;\
    t0 ^= x0;\
    x0 = ROTR(x0, _R[0][1] - _R[0][0]);\
    x0 ^= t0;}\

#define P12 \
    ROUND(0xf0);\
    ROUND(0xe1);\
    ROUND(0xd2);\
    ROUND(0xc3);\
    ROUND(0xb4);\
    ROUND(0xa5);\
    ROUND(0x96);\
    ROUND(0x87);\
    ROUND(0x78);\
    ROUND(0x69);\
    ROUND(0x5a);\
    ROUND(0x4b);\

#define P6 \
    ROUND(0x96);\
    ROUND(0x87);\
    ROUND(0x78);\
    ROUND(0x69);\
    ROUND(0x5a);\
    ROUND(0x4b);\

#define P1 \
    ROUND(0x4b);\

#define ABSORB_LANES(src, len)  \
    rem_bytes = len; \
    src64 = (u64 *)src; \
    idx64 = 0; \
    while(1){ \
        if(rem_bytes>ISAP_rH_SZ){ \
            x0 ^= U64BIG(src64[idx64]); \
            idx64++; \
            P12; \
            rem_bytes -= ISAP_rH_SZ; \
        } else if(rem_bytes==ISAP_rH_SZ){ \
            x0 ^= U64BIG(src64[idx64]); \
            P12; \
            x0 ^= 0x8000000000000000ULL; \
            P12; \
            break; \
        } else { \
            u64 lane64; \
            u8 *lane8 = (u8 *)&lane64; \
            u32 idx8 = idx64*8; \
            for (u32 i = 0; i < 8; i++) { \
                if(i<(rem_bytes)){ \
                    lane8[i] = src[idx8]; \
                    idx8++; \
                } else if(i==rem_bytes){ \
                    lane8[i] = 0x80; \
                } else { \
                    lane8[i] = 0x00; \
                } \
            } \
            x0 ^= U64BIG(lane64); \
            P12; \
            break; \
        } \
    } \


void isap_mac(
	const unsigned char *k,
	const unsigned char *npub,
	const unsigned char *ad, const unsigned long long adlen,
	const unsigned char *c, const unsigned long long clen,
	unsigned char *tag
);

void isap_rk(
	const unsigned char *k,
	const unsigned char *iv,
	const unsigned char *in,
	const unsigned long long inlen,
	unsigned char *out,
	const unsigned long long outlen
);

void isap_enc(
	const unsigned char *k,
	const unsigned char *npub,
	const unsigned char *m, const unsigned long long mlen,
	unsigned char *c
);

#endif