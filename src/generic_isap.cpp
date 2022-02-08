#include <cstdint>
#include <stdio.h>
#include "generic_isap.h"

// change here
#define MASKING_ORDER 2
#include "sse2_mask/2mask_ascon-p.h"
using namespace sse2_mask;
// to here

uint32_t rem_bytes;
uint64_t* src64;
uint32_t idx64;
uint64_t x[5];
uint64_t t[5];
uint64_t randinit[(MASKING_ORDER - 1) * 5];

#define _P12 \
    randbuf.GetBytes(randinit, sizeof(randinit));\
    InitShares(x, randinit);\
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
    GetShares(x);\


#define _P6 \
    _ROUND(0x96);\
    _ROUND(0x87);\
    _ROUND(0x78);\
    _ROUND(0x69);\
    _ROUND(0x5a);\
    _ROUND(0x4b);\


#define _P1 \
    _ROUND(0x4b);\

#define ABSORB_LANES(src, len)  \
    rem_bytes = len; \
    src64 = (uint64_t *)src; \
    idx64 = 0; \
    while(1){ \
        if(rem_bytes>ISAP_rH_SZ){ \
            x[0] ^= U64BIG(src64[idx64]); \
            idx64++; \
            _P12; \
            rem_bytes -= ISAP_rH_SZ; \
        } else if(rem_bytes==ISAP_rH_SZ){ \
            x[0] ^= U64BIG(src64[idx64]); \
            _P12; \
            x[0] ^= 0x8000000000000000ULL; \
            _P12; \
            break; \
        } else { \
            uint64_t lane64; \
            uint8_t *lane8 = (uint8_t *)&lane64; \
            uint32_t idx8 = idx64*8; \
            for (uint32_t i = 0; i < 8; i++) { \
                if(i<(rem_bytes)){ \
                    lane8[i] = src[idx8]; \
                    idx8++; \
                } else if(i==rem_bytes){ \
                    lane8[i] = 0x80; \
                } else { \
                    lane8[i] = 0x00; \
                } \
            } \
            x[0] ^= U64BIG(lane64); \
            _P12; \
            break; \
        } \
    } \

/******************************************************************************/
/*                                   IsapRk                                   */
/******************************************************************************/

void isap_rk(const uint8_t *k, const uint8_t *iv, const uint8_t *y, const uint64_t ylen, uint8_t *out, const uint64_t outlen) {
    const uint64_t *k64 = (uint64_t *)k;
    const uint64_t *iv64 = (uint64_t *)iv;
    uint64_t *out64 = (uint64_t *)out;
    // Init state
    for (size_t i = 0; i < 5; ++i) {t[i] = 0;}
    x[0] = U64BIG(k64[0]);
    x[1] = U64BIG(k64[1]);
    x[2] = U64BIG(iv64[0]);
    x[3] = x[4] = 0;
    _P12;
    // Absorb Y
    for (size_t i = 0; i < ylen*8-1; i++){
        size_t cur_byte_pos = i/8;
        size_t cur_bit_pos = 7-(i%8);
        uint8_t cur_bit = ((y[cur_byte_pos] >> (cur_bit_pos)) & 0x01) << 7;
        x[0] ^= ((uint64_t)cur_bit) << 56;
        _P1;
    }
    uint8_t cur_bit = ((y[ylen-1]) & 0x01) << 7;
    x[0] ^= ((uint64_t)cur_bit) << 56;
    _P12;
    // Extract K*
    out64[0] = U64BIG(x[0]);
    out64[1] = U64BIG(x[1]);
    if(outlen == 24){
        out64[2] = U64BIG(x[2]);
    }
}

/******************************************************************************/
/*                                  IsapMac                                   */
/******************************************************************************/

void isap_mac(const uint8_t *k, const uint8_t *npub, const uint8_t *ad, const uint64_t adlen, const uint8_t *c, const uint64_t clen, uint8_t *tag) {
    uint8_t state[ISAP_STATE_SZ];
    const uint64_t *npub64 = (uint64_t *)npub;
    uint64_t *state64 = (uint64_t *)state;
    for (size_t i=0;i<5;i++) {t[i]=0;}
    // Init state
    x[0] = U64BIG(npub64[0]);
    x[1] = U64BIG(npub64[1]);
    x[2] = U64BIG(((uint64_t *)ISAP_IV1)[0]);
    x[3] = x[4] = 0;
    _P12;
    // Absorb AD
    ABSORB_LANES(ad,adlen);
    // Domain seperation
    x[4] ^= 0x0000000000000001ULL;
    // Absorb C
    ABSORB_LANES(c,clen);
    // Derive K*
    state64[0] = U64BIG(x[0]);
    state64[1] = U64BIG(x[1]);
    state64[2] = U64BIG(x[2]);
    state64[3] = U64BIG(x[3]);
    state64[4] = U64BIG(x[4]);
    isap_rk(k,ISAP_IV2,(uint8_t *)state64,CRYPTO_KEYBYTES,(uint8_t *)state64,CRYPTO_KEYBYTES);
    x[0] = U64BIG(state64[0]);
    x[1] = U64BIG(state64[1]);
    x[2] = U64BIG(state64[2]);
    x[3] = U64BIG(state64[3]);
    x[4] = U64BIG(state64[4]);
    // Squeeze tag
    _P12;
    uint64_t *tag64 = (uint64_t *)tag;
    tag64[0] = U64BIG(x[0]);
    tag64[1] = U64BIG(x[1]);
}

/******************************************************************************/
/*                                  IsapEnc                                   */
/******************************************************************************/

void isap_enc(const uint8_t *k, const uint8_t *npub, const uint8_t *m, const uint64_t mlen, uint8_t *c) {
    uint8_t state[ISAP_STATE_SZ];
    // Init state
    uint64_t *state64 = (uint64_t *)state;
    uint64_t *npub64 = (uint64_t *)npub;
    isap_rk(k, ISAP_IV3, npub, CRYPTO_NPUBBYTES, state, ISAP_STATE_SZ - CRYPTO_NPUBBYTES);
    for (size_t i=0;i<5;i++) {t[i]=0;}
    x[0] = U64BIG(state64[0]);
    x[1] = U64BIG(state64[1]);
    x[2] = U64BIG(state64[2]);
    x[3] = U64BIG(npub64[0]);
    x[4] = U64BIG(npub64[1]);
    _P6;
    // Squeeze key stream
    rem_bytes = mlen;
    uint64_t *m64 = (uint64_t *)m;
    uint64_t *c64 = (uint64_t *)c;
    idx64 = 0;
    while(1){
        if(rem_bytes>ISAP_rH_SZ){
            // Squeeze full lane
            c64[idx64] = U64BIG(x[0]) ^ m64[idx64];
            idx64++;
            _P6;
            rem_bytes -= ISAP_rH_SZ;
        } else if(rem_bytes==ISAP_rH_SZ){
            // Squeeze full lane and stop
            c64[idx64] = U64BIG(x[0]) ^ m64[idx64];
            break;
        } else {
            // Squeeze partial lane and stop
            uint64_t lane64 = U64BIG(x[0]);
            uint8_t *lane8 = (uint8_t *)&lane64;
            uint32_t idx8 = idx64*8;
            for (uint32_t i = 0; i < rem_bytes; i++) {
                c[idx8] = lane8[i] ^ m[idx8];
                idx8++;
            }
            break;
        }
    }
}