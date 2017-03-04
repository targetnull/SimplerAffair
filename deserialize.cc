#include "deserialize.h"

/*
 * class lTransaction
 */

void deserial_tx(const Transaction &tx, lTransaction *ltx)
{
	decode_header(tx.header(), &ltx->header);
    memcpy(ltx->header_sig, tx.header_sig().c_str(), SIG_LEN);
    ltx->data = tx.data().c_str();
}

void decode_header(const TxHeader &header, lTxHeader *lheader)
{
	lheader->incnt = header.incnt();
	memcpy(lheader->senderPubkey, header.senderpubkey().c_str(), PUBKEY_LEN);
	memcpy(lheader->destPubkey, header.destpubkey().c_str(), PUBKEY_LEN);
	lheader->timestamp = header.timestamp();
	lheader->nonce = header.nonce();
}

void deserial_block(const Block &block, lBlock *lblock)
{
	//TODO. may need another way to decode header.
    decode_header(block.header(), &lblock->header);
    lblock->txn_count = block.txn_count();
    lblock->txns = new lTransaction[lblock->txn_count];
    for(int i = 0; i < lblock->txn_count; ++i)
        deserial_tx(block.txns(i), &lblock->txns[i]);
}

void decode_header(const BlockHeader &header, lBlockHeader *lheader)
{
	lheader->version = header.version();
	memcpy(lheader->prev_header_hash, header.prev_header_hash().c_str(), HASH_LEN);
	memcpy(lheader->merkel_root_hash, header.merkel_root_hash().c_str(), HASH_LEN);
	lheader->time = header.time();
	lheader->nBits = header.nbits();
	lheader->nonce = header.nonce();
}

void deserial_msgblock(const MsgBlock *msg, lMsgBlock *lmsg)
{
	lmsg->type = msg->type();
	deserial_block(msg->block(), &lmsg->block);
	lmsg->data = msg->data();
}



