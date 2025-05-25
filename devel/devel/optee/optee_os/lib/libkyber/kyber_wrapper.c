#include <tee_internal_api.h>
#include <string.h>
#include "kem.h"

// 간단한 Kyber KEM 래퍼 함수들
TEE_Result kyber_simple_keygen(uint8_t *pk, uint8_t *sk) {
    if (pqcrystals_kyber512_ref_keypair(pk, sk) == 0)
        return TEE_SUCCESS;
    return TEE_ERROR_GENERIC;
}

TEE_Result kyber_simple_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    if (pqcrystals_kyber512_ref_enc(ct, ss, pk) == 0)
        return TEE_SUCCESS;
    return TEE_ERROR_GENERIC;
}

TEE_Result kyber_simple_decaps(uint8_t *ss, const uint8_t *ct, const uint8_t *sk) {
    if (pqcrystals_kyber512_ref_dec(ss, ct, sk) == 0)
        return TEE_SUCCESS;
    return TEE_ERROR_GENERIC;
}
