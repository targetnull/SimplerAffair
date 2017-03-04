#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include "deserialize.h"

bool ProcessTxns(lBlock *block);
bool dealSingleTxn(lTransaction *txn);
bool verifyHeaderSig(lTransaction *txn);
bool check_datahash(lTransaction *txn);

#endif

