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
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    unsigned char buffer[AES_BLOCK_SIZE] = {0};
    unsigned char key[AES_BLOCK_SIZE] = "0123456789abcdef";
    unsigned char decryptedtext[AES_BLOCK_SIZE] = {0};
    unsigned char ciphertext[AES_BLOCK_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    read(new_socket, buffer, AES_BLOCK_SIZE);
    aes_decrypt(buffer, decryptedtext, key);
    printf("Decrypted message from client: %s\n", decryptedtext);

    aes_encrypt((unsigned char *)"Hello from server", ciphertext, key);
    send(new_socket, ciphertext, AES_BLOCK_SIZE, 0);
    printf("Encrypted hello message sent\n");

    close(new_socket);
    close(server_fd);
    return 0;
}
