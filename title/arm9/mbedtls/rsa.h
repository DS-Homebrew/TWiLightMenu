
#define MBEDTLS_ERR_RSA_PUBLIC_FAILED                     -0x4280  /**< The public key operation failed. */

#include "bignum.h"

typedef struct {
	size_t len;
	mbedtls_mpi N;
	mbedtls_mpi E;
	mbedtls_mpi RN;
} rsa_context_t;

void rsa_init(rsa_context_t *rsa);

int rsa_set_pubkey(rsa_context_t *rsa, const unsigned char * n_buf, size_t n_len,
	const unsigned char * e_buf, size_t e_len);

int rsa_public(rsa_context_t *rsa, const unsigned char *input, unsigned char *output);
