#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

/* Kyber 정의들 */
#define KYBER_PUBLICKEYBYTES  800
#define KYBER_SECRETKEYBYTES  1632
#define KYBER_CIPHERTEXTBYTES 768
#define KYBER_SSBYTES         32

#define TA_KYBER_TEST_CMD_KEYGEN    0
#define TA_KYBER_TEST_CMD_ENCAPS    1
#define TA_KYBER_TEST_CMD_DECAPS    2

/* 전역 변수 */
static uint8_t g_public_key[KYBER_PUBLICKEYBYTES];
static uint8_t g_secret_key[KYBER_SECRETKEYBYTES];

/* 더미 kyber 함수들 구현 - 함수 호출보다 먼저 정의 */
static TEE_Result kyber_simple_keygen(uint8_t *pk, uint8_t *sk) {
    /* 더미 키 생성 - 실제로는 난수로 채워야 함 */
    TEE_MemFill(pk, 0xAA, KYBER_PUBLICKEYBYTES);
    TEE_MemFill(sk, 0xBB, KYBER_SECRETKEYBYTES);
    DMSG("Dummy Kyber keypair generated");
    return TEE_SUCCESS;
}

static TEE_Result kyber_simple_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    (void)pk; /* 공개키 사용하지 않음 (더미) */
    /* 더미 암호문과 공유비밀 생성 */
    TEE_MemFill(ct, 0xCC, KYBER_CIPHERTEXTBYTES);
    TEE_MemFill(ss, 0xDD, KYBER_SSBYTES);
    DMSG("Dummy Kyber encapsulation performed");
    return TEE_SUCCESS;
}

static TEE_Result kyber_simple_decaps(uint8_t *ss, const uint8_t *ct, const uint8_t *sk) {
    (void)ct; /* 암호문 사용하지 않음 (더미) */
    (void)sk; /* 비밀키 사용하지 않음 (더미) */
    /* 더미 공유비밀 생성 */
    TEE_MemFill(ss, 0xDD, KYBER_SSBYTES);
    DMSG("Dummy Kyber decapsulation performed");
    return TEE_SUCCESS;
}

/* TA Entry Points */
TEE_Result TA_CreateEntryPoint(void) {
    DMSG("Kyber TA: Create");
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void) {
    DMSG("Kyber TA: Destroy");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param params[4],
                                    void **sess_ctx) {
    (void)param_types;
    (void)params;
    (void)sess_ctx;
    DMSG("Kyber TA: Open session");
    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ctx) {
    (void)sess_ctx;
    DMSG("Kyber TA: Close session");
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx,
                                      uint32_t cmd_id,
                                      uint32_t param_types,
                                      TEE_Param params[4]) {
    (void)sess_ctx;
    (void)param_types;
    
    DMSG("Kyber TA: Command %d", cmd_id);
    
    switch (cmd_id) {
    case TA_KYBER_TEST_CMD_KEYGEN:
        DMSG("Kyber TA: Generating keypair");
        return kyber_simple_keygen(g_public_key, g_secret_key);
        
    case TA_KYBER_TEST_CMD_ENCAPS: {
        uint8_t ciphertext[KYBER_CIPHERTEXTBYTES];
        uint8_t shared_secret[KYBER_SSBYTES];
        TEE_Result res;
        
        DMSG("Kyber TA: Performing encapsulation");
        res = kyber_simple_encaps(ciphertext, shared_secret, g_public_key);
        
        if (res == TEE_SUCCESS && params[0].memref.size >= KYBER_CIPHERTEXTBYTES) {
            TEE_MemMove(params[0].memref.buffer, ciphertext, KYBER_CIPHERTEXTBYTES);
            DMSG("Ciphertext copied to output buffer");
        }
        return res;
    }
    
    case TA_KYBER_TEST_CMD_DECAPS: {
        uint8_t shared_secret[KYBER_SSBYTES];
        TEE_Result res;
        
        if (params[0].memref.size < KYBER_CIPHERTEXTBYTES) {
            EMSG("Input ciphertext too small");
            return TEE_ERROR_BAD_PARAMETERS;
        }
                
        DMSG("Kyber TA: Performing decapsulation");
        res = kyber_simple_decaps(shared_secret, 
                                 (uint8_t*)params[0].memref.buffer, 
                                 g_secret_key);
        
        if (res == TEE_SUCCESS && params[1].memref.size >= KYBER_SSBYTES) {
            TEE_MemMove(params[1].memref.buffer, shared_secret, KYBER_SSBYTES);
            DMSG("Shared secret copied to output buffer");
        }
        return res;
    }
    
    default:
        EMSG("Kyber TA: Unknown command %d", cmd_id);
        return TEE_ERROR_NOT_SUPPORTED;
    }
}
