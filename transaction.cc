#include "transaction.h"

//#define HEADER_HASH_LEN 	(CryptoPP::SHA256::DIGESTSIZE)

bool ProcessTxns(lBlock *block)
{
	int num = block->txn_count;
	for(int i = 0; i < num; ++i)
		if(!dealSingleTxn(&block->txns[i]))
			return false;

	return true;
}

bool dealSingleTxn(lTransaction *txn)
{
	//verify the header signature and the datahash
	//1. if header_sig == Sign(SHA256(txheader)) then right else wrong
	//2. then check the data hash
	if(!verifyHeaderSig(txn) || !check_datahash(txn))
		return false;

	return true;
}

bool verifyHeaderSig(lTransaction *txn)
{
	uint8_t header_hash[HASH_LEN];
	SHA256((const uint8_t *)&txn->header, sizeof(lTxHeader), header_hash);
/*
	// TODO. pubkeyVerify to be implemented...
	if(!pubkeyVerify(txn->header_sig, tnx->header.senderPubkey, header_hash))
	{
		std::cout << "header signature verification not passed...\n";
		return false;
	}
*/
	return true;
}

bool check_datahash(lTransaction *txn)
{
	uint8_t data_hash[HASH_LEN];
	uint8_t *tmpdata = new uint8_t[txn->data.size()];
	memcpy(tmpdata, txn->data.c_str(), txn->data.size());
    SHA256(tmpdata, txn->data.size(), data_hash);
	delete tmpdata; //delete right away

	if(!memcmp(data_hash, txn->header.datahash, HASH_LEN))
		return false;

	return true;
}

