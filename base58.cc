#include <iostream>
//#include <cstdio>  
#include <cstdlib>  
#include <cstring>  
#include <openssl/bn.h>  

/*
 * This file is from http://www.8btc.com/btc_ecc_dsa_b
 */
#if 0
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

size_t Base58Encode(const unsigned char *begin, size_t size, char *to)
{
	size_t cb = 0;
	BN_CTX * ctx = NULL;
	unsigned char c;
	unsigned char *pzero = (unsigned char *)begin;
	unsigned char *pend = (unsigned char *)(begin + size);
	char *p = to;
	// bool fSign = 0;
	BIGNUM bn, dv, rem, bn58, bn0;
	if((NULL == begin) || (size == 0)) return 0; // invalid parameter
	cb = size * 138 /100+1; // the size of output will less than (138/100 * sizeof(src))
	//** output buffer should be allocated enough memory
	if(NULL == to) return cb;
	BN_init(&bn58); BN_init(&bn0);
	BN_set_word(&bn58, 58);
	BN_zero(&bn0);
	BN_init(&bn); BN_init(&dv); BN_init(&rem);
	BN_bin2bn(begin, size, &bn);
	ctx = BN_CTX_new();

	if(NULL==ctx)
		return 0;
	// All the initialization done, begin the real algorithm
	while(BN_cmp(&bn, &bn0) > 0)
	{
		if(!BN_div(&dv, &rem, &bn, &bn58, ctx)) break;
		bn = dv;
		c = BN_get_word(&rem);
		*(p++) = pszBase58[c];
	}
	// 添加前导的0
	while(*(pzero++)==0)
	{
		*(p++) = pszBase58[0];
		if(pzero > pend) break;
	}
	*p = '\0';
	cb = p - to;
	//ReverseBytes((unsigned char *)to, cb); // output as a big endian integer
	int pbegin = 0, pend = p - to;
	while(pbegin < pend) {  
		char c = to[pbegin]; 
		to[pbegin] = to[--pend];  
		to[++pbein] = c;
        }
	BN_CTX_free(ctx);
	return cb;
}
#endif

#if 1
const char * BASE58TABLE="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string base58encode(const std::string & hexstring)  
{  
	std::string result = "";  
	BN_CTX * bnctx = BN_CTX_new();  
	BN_CTX_init(bnctx);  
	BIGNUM * bn = BN_new();  
	BIGNUM * bn0= BN_new();  
	BIGNUM * bn58=BN_new();  
	BIGNUM * dv = BN_new();  
	BIGNUM * rem= BN_new();  
	BN_init(bn);   
	BN_init(bn0);  
	BN_init(bn58);  
	BN_init(dv);  
	BN_init(rem);  

	BN_hex2bn(&bn, hexstring.c_str());  
	//printf("bn:%s\n", BN_bn2dec(bn));  
	BN_hex2bn(&bn58, "3a");//58  
	BN_hex2bn(&bn0,"0");  

	while(BN_cmp(bn, bn0)>0){  
		BN_div(dv, rem, bn, bn58, bnctx);  
		BN_copy(bn, dv);  
		//printf("dv: %s\n", BN_bn2dec(dv));  
		//printf("rem:%s\n", BN_bn2dec(rem));  
		char base58char = BASE58TABLE[BN_get_word(rem)];  
		result += base58char;  
	}  

	std::string::iterator pbegin = result.begin();  
	std::string::iterator pend   = result.end();  
	while(pbegin < pend) {  
		char c = *pbegin;  
		*(pbegin++) = *(--pend);  
		*pend = c;  
	}
	BN_free(bn);
	BN_free(bn0);
	BN_free(bn58);
	BN_free(dv);
	BN_free(rem);
	return result;  
}
#endif

static const int8_t b58digits[] = {
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-1,-1,-1,-1,-1,-1,
	-1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
	22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
	-1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
};

size_t Base58Decode(const char *begin, size_t size, unsigned char *to)
{
	unsigned char c;
	unsigned char *p = (unsigned char *)begin;
	unsigned char *pend = p + size;
	size_t cb;
	BIGNUM bn, bnchar;
	BIGNUM bn58, bn0;
	BN_CTX *ctx = BN_CTX_new();
	if(NULL == ctx) return 0;
	BN_init(&bn58);
	BN_init(&bn0);
	BN_init(&bn); BN_init(&bnchar);
	BN_set_word(&bn58, 58);
	BN_zero(&bn0);
	while(p < pend)
	{
		c = *p;
		if(c & 0x80) goto label_errexit;
		if(-1 == b58digits[c]) goto label_errexit;
		BN_set_word(&bnchar, b58digits[c]);
		if(!BN_mul(&bn, &bn, &bn58, ctx)) goto label_errexit;
		BN_add(&bn, &bn, &bnchar);
		p++;
	}
	cb = BN_num_bytes(&bn);
	if(NULL == to) return cb;
	BN_bn2bin(&bn, to);
	BN_CTX_free(ctx);
	return cb;
label_errexit:
	if(NULL != ctx) BN_CTX_free(ctx);
	return 0;
}
