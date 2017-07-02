#include <3ds.h>
#include <nettle/base64.h>
#include <nettle/sha.h>

void sha1(u8* dst, const u8* src, size_t src_length)
{
    struct sha1_ctx ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, src_length, src);
    sha1_digest(&ctx, SHA1_DIGEST_SIZE, dst);
}

void base64(u8* dst, const u8* src, size_t src_length)
{
    struct base64_encode_ctx ctx;
    base64_encode_init(&ctx);
    base64_encode_raw(dst, src_length, src);
}


void create_accept_key(char* dst, const char* client_key)
{
    u8 sha1buf[20], key_src[60];
    memcpy(key_src, client_key, 24);
    memcpy(key_src+24, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
    sha1(sha1buf, key_src, sizeof(key_src));
    base64((u8*)dst, sha1buf, 20);
    dst[BASE64_ENCODE_RAW_LENGTH(20)] = '\0';
}
