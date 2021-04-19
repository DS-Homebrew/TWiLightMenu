// u128_math

#include "u128_math.h"
#include <string.h>

// rotate a 128bit, little endian by shift bits in direction of increasing significance.
void u128_lrot(uint8_t *num, uint32_t shift)
{
  uint8_t tmp[16];
  for (int i=0;i<16;i++)
  {
    // rot: rotate to more significant.
    // LSB is tmp[0], MSB is tmp[15]
    const uint32_t byteshift = shift / 8 ;
    const uint32_t bitshift = shift % 8;
    tmp[(i+byteshift) % 16] = (num[i] << bitshift)
           | ((num[(i+16-1) % 16] >> (8-bitshift)) & 0xff);    
  }
  memcpy(num, tmp, 16) ;
}

// rotate a 128bit, little endian by shift bits in direction of decreasing significance.
void u128_rrot(uint8_t *num, uint32_t shift)
{
  uint8_t tmp[16];
  for (int i=0;i<16;i++)
  {
    // rot: rotate to less significant.
    // LSB is tmp[0], MSB is tmp[15]
    const uint32_t byteshift = shift / 8 ;
    const uint32_t bitshift = shift % 8;
    tmp[i] = (num[(i+byteshift) % 16] >> bitshift)
           | ((num[(i+byteshift+1) % 16] << (8-bitshift)) & 0xff);    
  }
  memcpy(num, tmp, 16) ;
}

// xor two 128bit, little endian values and store the result into the first
void u128_xor(uint8_t *a, const uint8_t *b)
{
  for (int i=0;i<16;i++)
  {
    a[i] = a[i] ^ b[i] ;
  }
}

// or two 128bit, little endian values and store the result into the first
void u128_or(uint8_t *a, const uint8_t *b)
{
  for (int i=0;i<16;i++)
  {
    a[i] = a[i] | b[i] ;
  }
}

// and two 128bit, little endian values and store the result into the first
void u128_and(uint8_t *a, const uint8_t *b)
{
  for (int i=0;i<16;i++)
  {
    a[i] = a[i] & b[i] ;
  }
}


// add two 128bit, little endian values and store the result into the first
void u128_add(uint8_t *a, const uint8_t *b)
{
  uint8_t carry = 0 ;
  for (int i=0;i<16;i++)
  {
    uint16_t sum = a[i] + b[i] + carry ;
    a[i] = sum & 0xff ;
    carry = sum >> 8 ;
  }
}

// add two 128bit, little endian values and store the result into the first
void u128_add32(uint8_t *a, const uint32_t b)
{
  uint8_t _b[16];
  memset(_b, 0, sizeof(_b)) ;
  for (int i=0;i<4;i++)
    _b[i] = b >> (i*8) ;
  u128_add(a, _b);
}

// sub two 128bit, little endian values and store the result into the first
void u128_sub(uint8_t *a, const uint8_t *b)
{
  uint8_t carry = 0 ;
  for (int i=0;i<16;i++)
  {
    uint16_t sub = a[i] - b[i] - (carry & 1);
    a[i] = sub & 0xff ;
    carry = sub >> 8 ;
  }
}

void u128_swap(uint8_t *out, const uint8_t *in) 
{
  for (int i=0;i<16;i++)
  {
    out[15-i] = in[i];
  }
}