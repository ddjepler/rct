#include "SHA256.h"

#include <stdio.h>
#ifdef OS_Darwin
#include "CommonCrypto/CommonDigest.h"
#define SHA256_Update        CC_SHA256_Update
#define SHA256_Init          CC_SHA256_Init
#define SHA256_Final         CC_SHA256_Final
#define SHA256_CTX           CC_SHA256_CTX
#define SHA256_DIGEST_LENGTH CC_SHA256_DIGEST_LENGTH
#else
#include <openssl/sha.h>
#endif

class SHA256Private
{
public:
    SHA256_CTX ctx;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    bool finalized;
};

SHA256::SHA256()
    : priv(new SHA256Private)
{
    reset();
}

SHA256::~SHA256()
{
    delete priv;
}

void SHA256::update(const char *data, unsigned int size)
{
    if (!size)
        return;
    if (priv->finalized)
        priv->finalized = false;
    SHA256_Update(&priv->ctx, data, size);
}

void SHA256::update(const String &data)
{
    if (data.isEmpty())
        return;
    if (priv->finalized)
        priv->finalized = false;
    SHA256_Update(&priv->ctx, data.constData(), data.size());
}

void SHA256::reset()
{
    priv->finalized = false;
    SHA256_Init(&priv->ctx);
}

static const char* const hexLookup = "0123456789abcdef";

static inline String hashToHex(SHA256Private* priv)
{
    String out(SHA256_DIGEST_LENGTH * 2, '\0');
    const unsigned char* get = priv->hash;
    char* put = out.data();
    const char* const end = out.data() + out.size();
    for (; put != end; ++get) {
        *(put++) = hexLookup[(*get >> 4) & 0xf];
        *(put++) = hexLookup[*get & 0xf];
    }
    return out;
}

String SHA256::hash(MapType type) const
{
    if (!priv->finalized) {
        SHA256_Final(priv->hash, &priv->ctx);
        SHA256_Init(&priv->ctx);
        priv->finalized = true;
    }
    if (type == Hex)
        return hashToHex(priv);
    return String(reinterpret_cast<char*>(priv->hash), SHA256_DIGEST_LENGTH);
}

String SHA256::hash(const String& data, MapType type)
{
    return SHA256::hash(data.constData(), data.size(), type);
}

String SHA256::hash(const char* data, unsigned int size, MapType type)
{
    SHA256Private priv;
    SHA256_Init(&priv.ctx);
    SHA256_Update(&priv.ctx, data, size);
    SHA256_Final(priv.hash, &priv.ctx);
    if (type == Hex)
        return hashToHex(&priv);
    return String(reinterpret_cast<char*>(priv.hash), SHA256_DIGEST_LENGTH);
}

String SHA256::hashFile(const Path& file, MapType type)
{
    const int fd = ::open(file.nullTerminated(), 0);
    if (fd < 0)
        return String();

    struct stat st;
    if (fstat(fd, &st)) {
        ::close(fd);
        return String();
    }
    void *mapped = ::mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED)
        return String();

    const String ret = SHA256::hash(const_cast<const char *>(static_cast<char*>(mapped)), st.st_size, type);

    ::munmap(mapped, st.st_size);
    ::close(fd);

    return ret;
}
