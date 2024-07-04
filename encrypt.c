#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

int tls_client_handshake(int server_fd) {

    uint32_t client_rand;
    arc4random_buf(client_rand, sizeof(client_rand)); 
    write(server_fd, &client_rand, sizeof(client_rand));

    uint32_t server_rand;
    read(server_rand, &server_rand, sizeof(server_rand));

    return 0;
}


int tls_server_handshake(int client_fd) {

    uint32_t server_rand;
    arc4random_buf(server_rand, sizeof(server_rand)); 

    uint32_t client_rand;
    read(client_fd, &client_fd, sizeof(client_fd));
    return 0;
}


EVP_PKEY *get_peerkey(int fd) {
    uint8_t* peerkey_pub

}


unsigned char *ecdh(size_t *secret_len, int fd)
{
	EVP_PKEY_CTX *pctx, *kctx;
	EVP_PKEY_CTX *ctx;
	unsigned char *secret;
	EVP_PKEY *pkey = NULL, *peerkey, *params = NULL;
    
	/* NB: assumes pkey, peerkey have been already set up */

	/* Create the context for parameter generation */
	if(NULL == (pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL))) return NULL;

	/* Initialise the parameter generation */
	if(1 != EVP_PKEY_paramgen_init(pctx)) return NULL;

	/* We're going to use the ANSI X9.62 Prime 256v1 curve */
	if(1 != EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1)) return NULL;

	/* Create the parameter object params */
	if (!EVP_PKEY_paramgen(pctx, &params)) return NULL;

	/* Create the context for the key generation */
	if(NULL == (kctx = EVP_PKEY_CTX_new(params, NULL))) return NULL;

	/* Generate the key */
	if(1 != EVP_PKEY_keygen_init(kctx)) return NULL;
	if (1 != EVP_PKEY_keygen(kctx, &pkey)) return NULL;
        

	/* Get the peer's public key, and provide the peer with our public key -
	 * how this is done will be specific to your circumstances */
	peerkey = get_peerkey(fd);

	/* Create the context for the shared secret derivation */
	if(NULL == (ctx = EVP_PKEY_CTX_new(pkey, NULL))) return NULL;

	/* Initialise */
	if(1 != EVP_PKEY_derive_init(ctx)) return NULL;

	/* Provide the peer public key */
	if(1 != EVP_PKEY_derive_set_peer(ctx, peerkey)) return NULL;

	/* Determine buffer length for shared secret */
	if(1 != EVP_PKEY_derive(ctx, NULL, secret_len)) return NULL;

	/* Create the buffer */
	if(NULL == (secret = OPENSSL_malloc(*secret_len))) return NULL;

	/* Derive the shared secret */
	if(1 != (EVP_PKEY_derive(ctx, secret, secret_len))) return NULL;

	EVP_PKEY_CTX_free(ctx);
	EVP_PKEY_free(peerkey);
	EVP_PKEY_free(pkey);
	EVP_PKEY_CTX_free(kctx);
	EVP_PKEY_free(params);
	EVP_PKEY_CTX_free(pctx);

	/* Never use a derived secret directly. Typically it is passed
	 * through some hash function to produce a key */
	return secret;
}
