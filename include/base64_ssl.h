#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string>
#include <openssl/crypto.h>
#include <openssl/buffer.h>

//base64编码
std::string base64_encode(const std::string& input);

//base64解码
std::string base64_decode(const std::string& input);