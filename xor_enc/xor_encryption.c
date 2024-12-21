
#include <stdio.h>
#include <string.h>

#define KEY 0xAA // Simple XOR key

// Function to encrypt and decrypt data using XOR
void xor_encrypt_decrypt(char *data, size_t len, char key) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key;
    }
}

int main() {
    char message[] = "Hello, World!";
    size_t len = strlen(message);

    printf("Original message: %s\n", message);

    // Encrypt the message
    xor_encrypt_decrypt(message, len, KEY);
    printf("Encrypted message: %s\n", message);

    // Decrypt the message
    xor_encrypt_decrypt(message, len, KEY);
    printf("Decrypted message: %s\n", message);

    return 0;
}
