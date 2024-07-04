#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>

void generateKeys();
uint8_t* encrypt(uint8_t*src,uint len,int*length);
uint8_t*decrypt(uint8_t*src,int len);
