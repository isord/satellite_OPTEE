#ifndef __KYBER_WRAPPER_H
#define __KYBER_WRAPPER_H

#include <tee_internal_api.h>

#define KYBER_PUBLICKEYBYTES  800
#define KYBER_SECRETKEYBYTES  1632
#define KYBER_CIPHERTEXTBYTES 768
#define KYBER_SSBYTES         32

TEE_Result kyber_simple_keygen(uint8_t *pk, uint8_t *sk);
TEE_Result kyber_simple_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk);
TEE_Result kyber_simple_decaps(uint8_t *ss, const uint8_t *ct, const uint8_t *sk);

#endif
