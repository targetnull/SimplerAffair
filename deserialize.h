#ifndef _DESERIALIZE_H_
#define _DESERIALIZE_H_

#include <cstdint>
#include "server.h"

/*
 * Since gRPC has a high-level wrapper for c++ objects
 * such as std::string and array, we'd beter deserialize
 * these wrapped objects into local objects to meet our
 * own requirements. For example, MsgBlock is constant in gRPC
 * but by deserialization, we are able to modify it
 */

typedef struct _TXHEADER {
    uint32_t incnt; //number of inputs
    uint8_t senderPubkey[80]; //public key of the sender
    uint8_t destPubkey[80]; //receiver's public key
    int32_t timestamp = 4;
    uint32_t nonce = 5;
} lTxHeader;

class lTransaction
{
public:
	lTransaction(){}
	lTransaction(const Transaction &tx);
	~lTransaction(){}

	void deserial_tx(const Transaction &tx);
	void decode_header(const TxHeader &header);

	lTxHeader header;
	uint8_t header_sig[80]; //signed(sha256(header))
	std::string data; //the real data of this transaction
					//, with which we make automatic decision
};

typedef struct _LBLOCKHEADER {
	uint32_t version;
    uint8_t prev_header_hash[32];
    uint8_t merkel_root_hash[32];
    uint32_t time;
    uint32_t nBits;
    uint32_t nonce; //both should be ok.
    //uint8_t nonce[4];
}lBlockHeader;

//local Block
class lBlock
{
public:
	lBlock(){}
	lBlock(const Block &block);
	~lBlock(){}

	void deserial_block(const Block &block);
	void decode_header(const BlockHeader &header);

	lBlockHeader header;
	int txn_count; //indicate how many transactions
	lTransaction *txns; //transactions
};

//local MsgBlock
class lMsgBlock
{
public:
	lMsgBlock(){}
	lMsgBlock(const MsgBlock *msg);
	~lMsgBlock(){}

	//deserialize a MsgBlock into a local object
	void deserial_msgblock(const MsgBlock *msg);

	int type;
	lBlock block;
	/* TODO. [should remove] local object should not have data, it it only for server */
	std::string data;
};

#endif
