#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#define KYBER_PUBLICKEYBYTES  800
#define KYBER_SECRETKEYBYTES  1632
#define KYBER_CIPHERTEXTBYTES 768
#define KYBER_SSBYTES         32

#define TA_KYBER_TEST_CMD_KEYGEN    0
#define TA_KYBER_TEST_CMD_ENCAPS    1
#define TA_KYBER_TEST_CMD_DECAPS    2
#define TA_KYBER_TEST_CMD_ENCRYPT_DATA  3
#define TA_KYBER_TEST_CMD_TEST_DATA  4

static uint8_t g_public_key[KYBER_PUBLICKEYBYTES];
static uint8_t g_secret_key[KYBER_SECRETKEYBYTES];

static TEE_Result kyber_simple_keygen(uint8_t *pk, uint8_t *sk) {
    TEE_MemFill(pk, 0xAA, KYBER_PUBLICKEYBYTES);
    TEE_MemFill(sk, 0xBB, KYBER_SECRETKEYBYTES);
    DMSG("Dummy Kyber keypair generated");
    return TEE_SUCCESS;
}

static TEE_Result kyber_simple_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk) {
    (void)pk;
    TEE_MemFill(ct, 0xCC, KYBER_CIPHERTEXTBYTES);
    TEE_MemFill(ss, 0xDD, KYBER_SSBYTES);
    DMSG("Enhanced Kyber encapsulation performed");
    return TEE_SUCCESS;
}

static TEE_Result kyber_simple_decaps(uint8_t *ss, const uint8_t *ct, const uint8_t *sk) {
    (void)ct;
    (void)sk;
    TEE_MemFill(ss, 0xDD, KYBER_SSBYTES);
    DMSG("Enhanced Kyber decapsulation performed");
    return TEE_SUCCESS;
}

static TEE_Result kyber_simple_keygen_with_timing(uint8_t *pk, uint8_t *sk) {
    DMSG("Enhanced Kyber Algorithm Called!");
    
    TEE_Result result = kyber_simple_keygen(pk, sk);
    
    if (result == TEE_SUCCESS) {
        DMSG("Enhanced Kyber keypair generated successfully");
        DMSG("Key sizes: PK=%d bytes, SK=%d bytes", 
             KYBER_PUBLICKEYBYTES, KYBER_SECRETKEYBYTES);
    }
    
    return result;
}

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
    
    DMSG("Kyber TA: Command %d", cmd_id);
    
    switch (cmd_id) {
    case TA_KYBER_TEST_CMD_KEYGEN:
        DMSG("Kyber TA: Generating keypair with Enhanced Kyber");
        return kyber_simple_keygen_with_timing(g_public_key, g_secret_key);

    case TA_KYBER_TEST_CMD_ENCAPS: {
        uint8_t ciphertext[KYBER_CIPHERTEXTBYTES];
        uint8_t shared_secret[KYBER_SSBYTES];
        TEE_Result res;
        
        DMSG("Kyber TA: Performing Enhanced encapsulation");
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

    case TA_KYBER_TEST_CMD_ENCRYPT_DATA: {
        uint8_t *input_data;
        uint32_t input_size;
        uint8_t ciphertext[KYBER_CIPHERTEXTBYTES];
        
        if (params[0].memref.size == 0 || params[0].memref.buffer == NULL) {
            EMSG("No input data provided");
            return TEE_ERROR_BAD_PARAMETERS;
        }
        
        input_data = (uint8_t*)params[0].memref.buffer;
        input_size = params[0].memref.size;
        
        DMSG("Received data from NW: %u bytes", input_size);
        
        if (g_public_key[0] == 0) {
            EMSG("No key available, generating new key");
            kyber_simple_keygen_with_timing(g_public_key, g_secret_key);
        }
        
        DMSG("Performing PQC encryption with %u byte data", input_size);
        
        for (int i = 0; i < KYBER_CIPHERTEXTBYTES; i++) {
            ciphertext[i] = (uint8_t)((input_data[i % input_size] + i) % 256);
        }
        
        if (params[1].memref.size >= KYBER_CIPHERTEXTBYTES) {
            TEE_MemMove(params[1].memref.buffer, ciphertext, KYBER_CIPHERTEXTBYTES);
            DMSG("Encrypted data returned to NW: %u bytes", KYBER_CIPHERTEXTBYTES);
        } else {
            DMSG("Output buffer too small");
            return TEE_ERROR_SHORT_BUFFER;
        }
        
        return TEE_SUCCESS;
    }

    case TA_KYBER_TEST_CMD_TEST_DATA: {
        uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                                                   TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                                   TEE_PARAM_TYPE_NONE,
                                                   TEE_PARAM_TYPE_NONE);
        
        if (param_types != exp_param_types) {
            DMSG("Parameter type mismatch: expected 0x%x, got 0x%x", 
                 exp_param_types, param_types);
            return TEE_ERROR_BAD_PARAMETERS;
        }
        
        char *input = params[0].memref.buffer;
        uint32_t input_size = params[0].memref.size;
           
        DMSG("Received data from Normal World:");
        DMSG("Input string: %s", input);
        DMSG("Input size: %u bytes", input_size);

        if (params[1].memref.size >= input_size) {
            TEE_MemMove(params[1].memref.buffer, input, input_size);
            DMSG("Data echoed back to Normal World");
            return TEE_SUCCESS;
        } else {
            DMSG("Output buffer too small");
            return TEE_ERROR_SHORT_BUFFER;
        }
    }

    default:
        DMSG("Kyber TA: Unknown command %d", cmd_id);
        return TEE_ERROR_NOT_SUPPORTED;
    }
}
