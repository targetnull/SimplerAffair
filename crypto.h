#ifndef _CRPYTO_H_
#define _CRPYTO_H_

#include <iostream>
#include <string>
#include <stdexcept>
using std::runtime_error;

#include <cstdlib>
#include <cstdio>

#include "cryptopp/osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "cryptopp/eccrypto.h"
using CryptoPP::ECP;
using CryptoPP::ECDSA;

#include "cryptopp/sha.h"
using CryptoPP::SHA1;

#include "cryptopp/queue.h"
using CryptoPP::ByteQueue;

#include "cryptopp/oids.h"
using CryptoPP::OID;

// ASN1 is a namespace, not an object
#include "cryptopp/asn.h"
using namespace CryptoPP::ASN1;

#include "cryptopp/integer.h"
using CryptoPP::Integer;

#include <sstream>
#include "cryptopp/hex.h"
using CryptoPP::HexDecoder;
using CryptoPP::HexEncoder;

#include "cryptopp/filters.h"
using CryptoPP::StringSink;

#include "cryptopp/base64.h"
using CryptoPP::Base64Encoder;
using CryptoPP::Base64Decoder;

//unknown...
using CryptoPP::StringSource;
using CryptoPP::ArraySource;

void SHA256(const uint8_t *data, size_t size, uint8_t *res);

#if 0
#include <openssl/sha.h>
#include <openssl/crypto.h> // OPENSSL_cleanse
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ec.h>

#include "cryptopp/rsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/files.h"
#endif

#endif
