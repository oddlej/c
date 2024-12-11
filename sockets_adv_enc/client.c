#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#define PORT 8080
#define AES_KEY_SIZE 256
#define AES_BLOCK_SIZE 16

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

void aes_encrypt(const unsigned char *plaintext, unsigned char *ciphertext, const unsigned char *key) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, AES_BLOCK_SIZE)) handleErrors();
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
}

void aes_decrypt(const unsigned char *ciphertext, unsigned char *plaintext, const unsigned char *key) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key, NULL)) handleErrors();

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, AES_BLOCK_SIZE)) handleErrors();
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
}

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    unsigned char buffer[AES_BLOCK_SIZE] = {0};
    unsigned char key[AES_BLOCK_SIZE] = "0123456789abcdef";
    unsigned char plaintext[AES_BLOCK_SIZE] = "Hello";          //changed from "Hello from client" 2024/12/06 (too long)
    unsigned char ciphertext[AES_BLOCK_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    aes_encrypt(plaintext, ciphertext, key);
    send(sock, ciphertext, AES_BLOCK_SIZE, 0);
    printf("Encrypted hello message sent\n");

    valread = read(sock, buffer, AES_BLOCK_SIZE);
    aes_decrypt(buffer, plaintext, key);
    printf("Decrypted message from server: %s\n", plaintext);

    return 0;
}
