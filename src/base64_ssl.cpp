#include "base64_ssl.h"

//base64编码
std::string base64_encode(const std::string& input) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);
    std::string output(bufferPtr->data, bufferPtr->length);
    BUF_MEM_free(bufferPtr);
    return output;
}

//base64解码
std::string base64_decode(const std::string& input) {
    BIO *bio, *b64;
    char buffer[input.length()];
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input.c_str(), input.length());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_length = BIO_read(bio, buffer, input.length());
    BIO_free_all(bio);
    return std::string(buffer, decoded_length);
}