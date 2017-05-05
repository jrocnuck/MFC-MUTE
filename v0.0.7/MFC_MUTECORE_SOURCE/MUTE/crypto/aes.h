#ifndef CRYPTOPP_AES_H
#define CRYPTOPP_AES_H

/** \file
	AES winner announced on 10/2/2000
*/

#include "rijndael.h"

NAMESPACE_BEGIN(CryptoPP)

//! AES Encrypting Class - Rijndael Block Cipher      
typedef RijndaelEncryption AESEncryption;

//! AES Decryption Class - Rijndael Block Cipher      
typedef RijndaelDecryption AESDecryption;

NAMESPACE_END

#endif
