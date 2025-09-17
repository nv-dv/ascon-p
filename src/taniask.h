#ifndef _TANIASK_H_
#define _TANIASK_H_

#include <stdint.h>

void TANIASK_set_D(uint8_t* seed);
void TANIASK_set_P(uint8_t* seed);
void TANIASK_set_U(uint8_t* seed);
void TANIASK_encrypt(uint8_t* input, uint8_t* Youtput, uint8_t* Ztoutput);
void TANIASK_decrypt(uint8_t* Yinput, uint8_t* Ztinput, uint8_t* output);
void TANIASK_set_rng_seed(unsigned int seed);

#endif //_TANIASK_H_