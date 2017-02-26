#include "deserialize.h"

/*
 * class lTransaction
 */
lTransaction::lTransaction(const Transaction &tx)
{
	decode_header(tx.header());
	memcpy(header_sig, tx.header_sig().c_str(), 80);
	data = tx.data().c_str();
}

void lTransaction::deserial_tx(const Transaction &tx)
{
	decode_header(tx.header());
    memcpy(header_sig, tx.header_sig().c_str(), 80);
    data = tx.data().c_str();
}

void lTransaction::decode_header(const TxHeader &header)
{
	this->header.incnt = header.incnt();
	memcpy(this->header.senderPubkey, header.senderpubkey().c_str(), 80);
	memcpy(this->header.destPubkey, header.destpubkey().c_str(), 80);
	this->header.timestamp = header.timestamp();
	this->header.nonce = header.nonce();
}


/*
 * class lBlock
 */
lBlock::lBlock(const Block &block)
{
	//TODO. may need another way to decode header.
	decode_header(block.header());
	txn_count = block.txn_count();
	txns = new lTransaction[txn_count];
	for(int i = 0; i < txn_count; ++i)
		txns[i].deserial_tx(block.txns(i));
}

void lBlock::deserial_block(const Block &block)
{
	//TODO. may need another way to decode header.
    decode_header(block.header());
    txn_count = block.txn_count();
    txns = new lTransaction[txn_count];
    for(int i = 0; i < txn_count; ++i)
        txns[i].deserial_tx(block.txns(i));
}

void lBlock::decode_header(const BlockHeader &header)
{
	this->header.version = header.version();
	memcpy(this->header.prev_header_hash, header.prev_header_hash().c_str(), 80);
	memcpy(this->header.merkel_root_hash, header.merkel_root_hash().c_str(), 80);
	this->header.time = header.time();
	this->header.nBits = header.nbits();
	this->header.nonce = header.nonce();
}


/*
 * class lMsgBlock
 */
lMsgBlock::lMsgBlock(const MsgBlock *msg)
{
	type = msg->type();
	block.deserial_block(msg->block());
	data = msg->data();
}


void lMsgBlock::deserial_msgblock(const MsgBlock *msg)
{
	type = msg->type();
	block.deserial_block(msg->block());
	data = msg->data();
}



