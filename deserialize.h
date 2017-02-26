#ifndef _DESERIALIZE_H_
#define _DESERIALIZE_H_

#include <cstdint>

//deserilize the grpc object to local objects
//otherwise, it is inconvinient to proceed these data

typedef struct _TXHEADER {
    uint32_t incnt; //number of inputs
    uint8_t SenderPubkey[80]; //public key of the sender
    uint8_t DestPublic[80]; //receiver's public key
    int32_t timestamp = 4;
    uint32_t nonce = 5;
} lTxHeader;

class lTransaction
{
	lTxHeader header;
	uint8_t header_sig[80]; //signed(sha256(header))
	uint8_t *data; //the real data of this transaction
					//, with which we make automatic decision
};

typedef struct _LBLOCKHEADER {
	uint32_t version;
    uint8_t prev_block_hash[32];
    uint8_t merkel_root_hash[32];
    uint32_t time;
    uint32_t nBits;
    //uint32_t nonce; //both should be ok.
    uint8_t nonce[4];
}lBlockHeader;

//local block
class lBlock
{
	lBlockHeader header;
	int txn_count; //indicate how many transactions
	lTransaction *txn; //transactions
};

//local MsgBlock
class lMsgBlock
{
	int type;
	lBlock block;
	/* local object should not have data, it it only for server */
	//std::string data;
};

#endif
