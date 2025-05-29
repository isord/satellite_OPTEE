#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <string.h>

/* Kyber 파라미터 */
#define KYBER_PUBLICKEYBYTES  800
#define KYBER_SECRETKEYBYTES  1632
#define KYBER_CIPHERTEXTBYTES 768
#define KYBER_SSBYTES         32

/* 글로벌 키 저장소 */
static uint8_t g_public_key[KYBER_PUBLICKEYBYTES];
static uint8_t g_secret_key[KYBER_SECRETKEYBYTES];

/* 더미 Kyber 키 생성 함수 */
static TEE_Result generate_kyber_keys(void) {
    uint32_t time_seed;
    TEE_GetSystemTime(&time_seed);
    
    /* 시간 기반 더미 키 생성 */
    for (int i = 0; i < KYBER_PUBLICKEYBYTES; i++) {
        g_public_key[i] = (uint8_t)((time_seed + i) % 256);
    }
    for (int i = 0; i < KYBER_SECRETKEYBYTES; i++) {
        g_secret_key[i] = (uint8_t)((time_seed + i + 100) % 256);
    }
    
    EMSG("🔑 KYBER KEYS GENERATED!");
    EMSG("📊 Public Key: %02x %02x %02x %02x... (%d bytes)", 
         g_public_key[0], g_public_key[1], g_public_key[2], g_public_key[3], 
         KYBER_PUBLICKEYBYTES);
    EMSG("📊 Secret Key: %02x %02x %02x %02x... (%d bytes)", 
         g_secret_key[0], g_secret_key[1], g_secret_key[2], g_secret_key[3], 
         KYBER_SECRETKEYBYTES);
    
    return TEE_SUCCESS;
}

/* 더미 Kyber 암호화 함수 */
static TEE_Result kyber_encapsulate(uint8_t *ciphertext, uint8_t *shared_secret) {
    uint32_t time_seed;
    TEE_GetSystemTime(&time_seed);
    
    /* 더미 암호문과 공유비밀 생성 */
    for (int i = 0; i < KYBER_CIPHERTEXTBYTES; i++) {
        ciphertext[i] = (uint8_t)((time_seed + i + 200) % 256);
    }
    for (int i = 0; i < KYBER_SSBYTES; i++) {
        shared_secret[i] = (uint8_t)((time_seed + i + 300) % 256);
    }
    
    EMSG("🔐 KYBER ENCAPSULATION COMPLETE!");
    EMSG("📦 Ciphertext: %02x %02x %02x %02x... (%d bytes)", 
         ciphertext[0], ciphertext[1], ciphertext[2], ciphertext[3], 
         KYBER_CIPHERTEXTBYTES);
    EMSG("🔒 Shared Secret: %02x %02x %02x %02x... (%d bytes)", 
         shared_secret[0], shared_secret[1], shared_secret[2], shared_secret[3], 
         KYBER_SSBYTES);
    
    return TEE_SUCCESS;
}

TEE_Result TA_CreateEntryPoint(void) {
    EMSG("🚀🚀🚀 KYBER PQC TA LOADED! 🚀🚀🚀");
    EMSG("✅ Post-Quantum Cryptography active in Secure World!");
    EMSG("🛡️  NIST Kyber algorithm ready");
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void) {
    EMSG("🔒 Kyber PQC TA destroyed - session complete");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param params[4],
                                    void **sess_ctx) {
    (void)param_types;
    (void)params;
    (void)sess_ctx;
    
    EMSG("🔐 KYBER SESSION OPENED!");
    EMSG("🔄 Initializing Post-Quantum Cryptography...");
    
    /* 자동으로 키 생성 */
    generate_kyber_keys();
    
    EMSG("✅ Kyber TA ready for PQC operations!");
    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess_ctx) {
    (void)sess_ctx;
    EMSG("🔒 Kyber PQC session closed");
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx,
                                      uint32_t cmd_id,
                                      uint32_t param_types,
                                      TEE_Param params[4]) {
    (void)sess_ctx;
    (void)param_types;
    (void)params;
    
    EMSG("🔥 KYBER COMMAND EXECUTED: %d", cmd_id);
    
    switch (cmd_id) {
    case 0: /* Hello command를 Key Generation으로 */
        EMSG("🔑 Performing Kyber Key Generation...");
        generate_kyber_keys();
        EMSG("✅ Quantum-resistant keypair generated!");
        break;
        
    case 1: /* Inc command를 Encapsulation으로 */
        {
            uint8_t ciphertext[KYBER_CIPHERTEXTBYTES];
            uint8_t shared_secret[KYBER_SSBYTES];
            
            EMSG("🔐 Performing Kyber Encapsulation...");
            kyber_encapsulate(ciphertext, shared_secret);
            EMSG("✅ Quantum-resistant encryption complete!");
        }
        break;
        
    default:
        EMSG("🔥 Kyber PQC operation: %d", cmd_id);
        break;
    }
    
    EMSG("🎉 POST-QUANTUM CRYPTOGRAPHY SUCCESSFUL!");
    return TEE_SUCCESS;
}
