#include "block.h"
#include "crypto.h"

#include <set>

#define SHA_DIGEST_LENGTH	(CryptoPP::SHA256::DIGESTSIZE)

std::set<std::string> blocks; //all block header hashes
lBlock *cur_block = NULL; //currently latest block

static std::string calc_merkle_root(lTransaction *txns, int num)
{
	std::string merkle_root;
	std::vector<std::string *> old_hashes;
	std::vector<std::string *> new_hashes;
	for (int i = 0; i < num; ++i) {
		unsigned char hash1[SHA_DIGEST_LENGTH];
		SHA256((const uint8_t *)&txns[i].header, PUBKEY_LEN, (unsigned char *)hash1);
		old_hashes.push_back(new std::string((const char *)hash1));
	}

	// Until we reach root, construct each level in turn.
	while (1) {
		unsigned char buffer[SHA_DIGEST_LENGTH * 2];
		for (int i = 0; i < old_hashes.size(); i += 2) {
			unsigned char hash2[SHA_DIGEST_LENGTH];

			// Concatenate leaves into a single buffer and SHA-1 it.
			memcpy(buffer, old_hashes[i], SHA_DIGEST_LENGTH);
			memcpy(buffer + SHA_DIGEST_LENGTH, old_hashes[i+1], SHA_DIGEST_LENGTH);
			SHA256((const uint8_t *)buffer, 2 * SHA_DIGEST_LENGTH, hash2);
			new_hashes.push_back(new std::string((const char *)hash2));
		}
		if (new_hashes.size() == 1) {
			break;
		}
		old_hashes = new_hashes;
		new_hashes.clear();
	}
	if (new_hashes[0] == NULL)
		assert(false);

	//delete all the string other than the first one.
	for (std::vector<std::string *>::iterator it = old_hashes.begin() ; it != old_hashes.end(); ++it)
		delete *it;

	merkle_root = (char*) new_hashes[0];
	delete new_hashes[0];
	return merkle_root;
}

static bool checkPow(int pow_zeros, uint8_t *headerhash)
{
	if(pow_zeros <= 0)
		return true;

	if(pow_zeros > (SHA_DIGEST_LENGTH >> 1))
		return false;

	//check if it meets the requirement of POW
	int bytes = pow_zeros >> 1;
	int rem = pow_zeros & 1;
	for(int i = 0; i < bytes; ++i)
		if(headerhash[i] != 0)
			return false;

	if(pow_zeros & 1)
		if((headerhash[bytes] & 0xf0) != 0)
			return false;

	return true;
}

bool VerifyBlock(lBlock *block)
{
    // 1. calculate merkel root and compare with hash in the header
    std::string merkel_root = calc_merkle_root(block->txns, block->txn_count);
    uint8_t root_hash_in_header[SHA_DIGEST_LENGTH];
    memcpy(root_hash_in_header, block->header.merkel_root_hash, SHA_DIGEST_LENGTH);
    if(!memcmp(root_hash_in_header, merkel_root.c_str(), SHA_DIGEST_LENGTH))
        return false; //merkel_root_hash mismatch

	//calculate header hash
    uint8_t headerhash[SHA_DIGEST_LENGTH];
    SHA256((const uint8_t *)&block->header, sizeof(lBlockHeader), headerhash);

    // 2. Check POW
    if(!checkPow(POW_ZEROS, headerhash))
		return false;

	// 3. see if the block already exists
	std::set<std::string>::iterator it;
	std::string thisblock((const char *)headerhash);
	it = blocks.find(thisblock);
	if(it != blocks.end()) {
		std::cout << "block already exists!\n";
		return false;
	}

    // 4. verify if prev_header_hash == SHA256(&cur_block->header) ?
	// TODO. could have some better way when retrieving blocks, 
	//       but for simplicity, just use this simple rule
	if(cur_block) {
		uint8_t cur_blk_header_hash[SHA_DIGEST_LENGTH];
		SHA256((const uint8_t *)&cur_block->header, sizeof(lBlockHeader), cur_blk_header_hash);
		if(!memcmp(block->header.prev_header_hash, cur_blk_header_hash, SHA_DIGEST_LENGTH))
		{
			std::cout << "some blocks are lost..\n";
			return false;
		}
	}

	cur_block = block; //update current latest block
	blocks.insert(thisblock); //insert the block into the set
	return true;;
}

