#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tee_client_api.h>

/* TA UUID */
#define TA_KYBER_TEST_UUID \
    { 0x8aaaf200, 0x2450, 0x11e4, \
        { 0xab, 0xe2, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }

/* TA Commands */
#define TA_KYBER_TEST_CMD_KEYGEN    0
#define TA_KYBER_TEST_CMD_ENCAPS    1
#define TA_KYBER_TEST_CMD_DECAPS    2
#define TA_KYBER_TEST_CMD_ENCRYPT_DATA  3
#define TA_KYBER_TEST_CMD_TEST_DATA  4

/* Kyber parameters */
#define KYBER_PUBLICKEYBYTES  800
#define KYBER_SECRETKEYBYTES  1632
#define KYBER_CIPHERTEXTBYTES 768
#define KYBER_SSBYTES         32

int main(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_KYBER_TEST_UUID;
    uint32_t err_origin;
    
    uint8_t ciphertext[KYBER_CIPHERTEXTBYTES];
    
    printf("=== Kyber TA Test ===\n");
    
    /* Initialize context */
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    
    /* Open session */
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
                          TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
             res, err_origin);

    /* Test 0: Data Transfer Test (교수님 요청사항) */
    printf("0. Testing Data Transfer to Secure World...\n");
    char input[] = "Hello from Normal World! This is test data for OP-TEE.";
    char output[256];
    
    printf("   Sending: %s\n", input);
    
    memset(&op, 0, sizeof(op));
    // change the parameter type to MEMREF
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE, TEEC_NONE);
    // set TEE parameter to user input
    op.params[0].tmpref.buffer = input;
    op.params[0].tmpref.size = strlen(input) + 1;
    
    // set output buffer
    op.params[1].tmpref.buffer = output;
    op.params[1].tmpref.size = sizeof(output);
    
    res = TEEC_InvokeCommand(&sess, TA_KYBER_TEST_CMD_TEST_DATA, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("   FAILED: Data transfer error 0x%x origin 0x%x\n", res, err_origin);
    } else {
        printf("   SUCCESS: Data transferred to TA!\n");
        printf("   Received back: %s\n", output);
        printf("   Check Secure World tab for TA logs.\n");
    }
    
    /* Test 1: Key Generation */
    printf("1. Testing Kyber Key Generation...\n");
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);
    
    res = TEEC_InvokeCommand(&sess, TA_KYBER_TEST_CMD_KEYGEN,
                            &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("   FAILED: Key generation error 0x%x origin 0x%x\n", res, err_origin);
    } else {
        printf("   SUCCESS: Kyber keypair generated!\n");
    }
    
    /* Test 2: Encapsulation */
    printf("2. Testing Kyber Encapsulation...\n");
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = ciphertext;
    op.params[0].tmpref.size = KYBER_CIPHERTEXTBYTES;
    
    res = TEEC_InvokeCommand(&sess, TA_KYBER_TEST_CMD_ENCAPS,
                            &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("   FAILED: Encapsulation error 0x%x origin 0x%x\n", res, err_origin);
    } else {
        printf("   SUCCESS: Kyber encapsulation completed!\n");
        printf("   Ciphertext (first 16 bytes): ");
        for (int i = 0; i < 16; i++) {
            printf("%02x ", ciphertext[i]);
        }
        printf("\n");
    }
    
    /* Cleanup */
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
    
    printf("=== Test Complete ===\n");
    return 0;
}
