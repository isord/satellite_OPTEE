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
#define TA_KYBER_TEST_CMD_TEST_DATA 4

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
    
    /* Test 0: Data Transfer Test */
    printf("0. Testing Data Transfer to Secure World...\n");
    char input[] = "Hello from Normal World!";
    char output[256];
    
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = input;
    op.params[0].tmpref.size = strlen(input) + 1;
    op.params[1].tmpref.buffer = output;
    op.params[1].tmpref.size = sizeof(output);
    
    res = TEEC_InvokeCommand(&sess, TA_KYBER_TEST_CMD_TEST_DATA, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        printf("   FAILED: Data transfer error 0x%x origin 0x%x\n", res, err_origin);
    } else {
        printf("   SUCCESS: Data transferred!\n");
        printf("   Sent: %s\n", input);
        printf("   Received: %s\n", output);
    }
    
    /* Test 1: Key Generation */
    printf("1. Testing Kyber Key Generation...\n");
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);
    
    res = TEEC_InvokeCommand(&sess, TA_KYBER_TEST_CMD_KEYGEN, &op, &err_origin);
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
    
    res = TEEC_InvokeCommand(&sess, TA_KYBER_TEST_CMD_ENCAPS, &op, &err_origin);
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
