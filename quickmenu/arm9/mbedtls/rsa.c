
// mbedtls RSA public
// only the pubkey function for signatures verifying
// original rsa.c had too many extra functions not used and too many dependencies

#include <string.h>
#include "bignum.h"
#include "rsa.h"

void rsa_init(rsa_context_t *ctx) {
	memset(ctx, 0, sizeof(rsa_context_t));
}

// I don't know why mbedtls doesn't provide this
// instead, all callers set N/E/len manually
// this could be seen in mbedtls_rsa_self_test(rsa.c), main(dh_client.c) and main(rsa_verify.c)
int rsa_set_pubkey(rsa_context_t *ctx, const unsigned char * n_buf, size_t n_len,
	const unsigned char * e_buf, size_t e_len)
{
	int ret0 = (mbedtls_mpi_read_binary(&ctx->N, n_buf, n_len));
	int ret1 = (mbedtls_mpi_read_binary(&ctx->E, e_buf, e_len));
	if (ret0 == 0 && ret1 == 0) {
		ctx->len = (mbedtls_mpi_bitlen(&ctx->N) + 7) >> 3;
		// we should check the key now to be safe?
		// anyway usually we load known working keys, so it's omitted
		return 0;
	} else {
		return ret0 || ret1;
	}
}

// basically mbedtls_rsa_public
int rsa_public(rsa_context_t *ctx, const unsigned char *input, unsigned char *output) {
	int ret;
	size_t olen;
	mbedtls_mpi T;

	mbedtls_mpi_init(&T);

	MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&T, input, ctx->len));

	if (mbedtls_mpi_cmp_mpi(&T, &ctx->N) >= 0)
	{
		ret = MBEDTLS_ERR_MPI_BAD_INPUT_DATA;
		goto cleanup;
	}

	olen = ctx->len;
	MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(&T, &T, &ctx->E, &ctx->N, &ctx->RN));
	MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&T, output, olen));

cleanup:

	mbedtls_mpi_free(&T);

	if (ret != 0)
		return(MBEDTLS_ERR_RSA_PUBLIC_FAILED + ret);

	return(0);
}

