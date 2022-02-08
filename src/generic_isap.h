#pragma once
#include<cstdint>
#include <immintrin.h>
#include "RandomBuffer/RandomBuffer.h"

#ifndef ISAP_H
#define ISAP_H
#define CRYPTO_KEYBYTES 16
#define CRYPTO_NSECBYTES 0
#define CRYPTO_NPUBBYTES 16
#define CRYPTO_ABYTES 16
#define CRYPTO_NOOVERLAP 1
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

const uint8_t ISAP_IV1[] = {0x01,ISAP_K,ISAP_rH,ISAP_rB,ISAP_sH,ISAP_sB,ISAP_sE,ISAP_sK};
const uint8_t ISAP_IV2[] = {0x02,ISAP_K,ISAP_rH,ISAP_rB,ISAP_sH,ISAP_sB,ISAP_sE,ISAP_sK};
const uint8_t ISAP_IV3[] = {0x03,ISAP_K,ISAP_rH,ISAP_rB,ISAP_sH,ISAP_sB,ISAP_sE,ISAP_sK};

#define ROTR(x,n) (((x)>>(n))|((x)<<(64-(n))))
#define EXT_BYTE(x,n) ((u8)((u64)(x)>>(8*(7-(n)))))
#define INS_BYTE(x,n) ((u64)(x)<<(8*(7-(n))))

#define U64BIG(x) \
    ((ROTR(x, 8) & (0xFF000000FF000000ULL)) | \
     (ROTR(x,24) & (0x00FF000000FF0000ULL)) | \
     (ROTR(x,40) & (0x0000FF000000FF00ULL)) | \
     (ROTR(x,56) & (0x000000FF000000FFULL)))
#endif

void isap_mac(
	const uint8_t *k,
	const uint8_t *npub,
	const uint8_t *ad, const uint64_t adlen,
	const uint8_t *c, const uint64_t clen,
	uint8_t *tag
);

void isap_rk(
	const uint8_t *k,
	const uint8_t *iv,
	const uint8_t *in,
	const uint64_t inlen,
	uint8_t *out,
	const uint64_t outlen
);

void isap_enc(
	const uint8_t *k,
	const uint8_t *npub,
	const uint8_t *m, const uint64_t mlen,
	uint8_t *c
);