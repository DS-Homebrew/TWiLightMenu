// u128_math

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// left rotate by (shift % 128) bits
void u128_lrot(uint8_t *num, uint32_t shift) ;

// right rotate by (shift % 128) bits
void u128_rrot(uint8_t *num, uint32_t shift) ;

// bitwise xor strored to a
void u128_xor(uint8_t *a, const uint8_t *b) ;

// bitwise or strored to a
void u128_or(uint8_t *a, const uint8_t *b) ;

// bitwise and strored to a
void u128_and(uint8_t *a, const uint8_t *b) ;

// add b to a and store in a
void u128_add(uint8_t *a, const uint8_t *b) ;

// add 32 bit value b to a and store in a
void u128_add32(uint8_t *a, const uint32_t b) ;

// substract b from a and store in a
void u128_sub(uint8_t *a, const uint8_t *b) ;

// swap byte order 
void u128_swap(uint8_t *out, const uint8_t *in) ;

#ifdef __cplusplus
}
#endif