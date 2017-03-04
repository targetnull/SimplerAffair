#include "crypto.h"

#if 0
void SHA256(std::string &data, std::string& res)
{
    byte const* pbData = (byte*) data.data();
    unsigned int nDataLen = data.size();
    byte abDigest[CryptoPP::SHA256::DIGESTSIZE];

    CryptoPP::SHA256().CalculateDigest(abDigest, pbData, nDataLen);

    res = std::string((char *)abDigest);
}
#endif

void SHA256(const uint8_t *data, size_t size, uint8_t *res)
{
	//byte abDigest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(res, data, size);
	//memcpy(res, (void *)abDigest, CryptoPP::SHA256::DIGESTSIZE);
}
