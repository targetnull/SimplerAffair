#ifndef _DESERIALIZE_H_
#define _DESERIALIZE_H_

#include <iostream>
#include <cstdint>

#include "chainserver.grpc.pb.h"
//#include "server.h"

#include "crypto.h"

using chainserver::MsgType;
using chainserver::MsgBlock;
using chainserver::Block;
using chainserver::BlockHeader;
using chainserver::Transaction;
using chainserver::TxHeader;

#define PUBKEY_LEN	64
#define SIG_LEN		64
#define HASH_LEN	32

/*
 * Since gRPC has a high-level wrapper for c++ objects
 * such as std::string and array, we'd beter deserialize
 * these wrapped objects into local objects to meet our
 * own requirements. For example, MsgBlock is constant in gRPC
 * but by deserialization, we are able to modify it
 */

typedef struct InboundMsg InboundMsg;

typedef struct _TXHEADER {
    uint32_t incnt; //number of inputs
    uint8_t senderPubkey[PUBKEY_LEN]; //public key of the sender
    uint8_t destPubkey[PUBKEY_LEN]; //receiver's public key
	uint8_t datahash[HASH_LEN]; //hash of data related to the affairs
    int32_t timestamp = 4;
    uint32_t nonce = 5;
} lTxHeader;

typedef struct _LTRANSACTION {
	lTxHeader header;
	uint8_t header_sig[SIG_LEN]; //signed(sha256(header))
	std::string data; //the real data of this transaction, with which we make automatic decision
} lTransaction;

typedef struct _LBLOCKHEADER {
	//uint32_t version;
    uint8_t prev_header_hash[HASH_LEN];
    uint8_t merkel_root_hash[HASH_LEN];
    uint32_t time;
    uint32_t nBits;
    uint32_t nonce; //both should be ok.
    //uint8_t nonce[4];
}lBlockHeader;

typedef struct _LBLOCKCONT {
	lBlockHeader header;
	int txn_count; //indicate how many transactions
	lTransaction *txns; //transactions
} lBlock;

typedef struct _LMSGBLOCK {
	int type;
	lBlock block;
	/* TODO. [should remove] local object should not have data, it it only for server */
	std::string data;
} lMsgBlock;

void deserial_tx(const Transaction &tx, lTransaction *ltx);
void decode_header(const TxHeader &header, lTxHeader *lheader);

void deserial_block(const Block &block, lBlock *lblock);
void decode_header(const BlockHeader &header, lBlockHeader *lheader);

//deserialize a MsgBlock into a local object
void deserial_msgblock(const MsgBlock *msg, lMsgBlock *lmsg);

#endif
