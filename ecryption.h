#pragma once
#ifndef ENCRYTPION
#define ENCRYPTION

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <iostream>

void derive_key(const std::string& password, const unsigned char* salt, unsigned char* key) {
    const int iterations = 10000; // You can adjust the number of iterations
    const int key_length = 16; // For AES-128, use 16 bytes

    if (PKCS5_PBKDF2_HMAC_SHA1(password.c_str(), password.length(),
        salt, 16, // Salt length
        iterations, key_length, key) == 0) {
        // Handle error
        std::cerr << "Key derivation failed." << std::endl;
    }
}

// Example of generating a salt and deriving a key
void generate_key(const std::string& password, unsigned char* key) {
    unsigned char salt[16]; // 16 bytes for salt
    if (!RAND_bytes(salt, sizeof(salt))) {
        std::cerr << "Salt generation failed." << std::endl;
        return;
    }

    // Now derive the key using the password and the generated salt
    derive_key(password, salt, key);
}

#endif // !ENCRYTPION

