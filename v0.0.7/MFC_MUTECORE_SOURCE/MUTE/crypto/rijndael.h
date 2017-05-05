#ifndef CRYPTOPP_RIJNDAEL_H
#define CRYPTOPP_RIJNDAEL_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

//! base class, do not use directly
/*!
  \b AES Standard \b <br>
  Information<br>
     http://www.rijndael.com/<br>
     http://www.esat.kuleuven.ac.be/~rijmen/rijndael/<br>
  <br>
  Keylength = 128/192/256 bits<br>
  Blocksize = 128 bits<br>
  Rounds    = Variable<br>    
*/
class Rijndael : public FixedBlockSize<16>, public VariableKeyLength<16, 16, 32, 8>
{
protected:
	Rijndael(const byte *userKey, unsigned int keylength);

	static const word32 Te0[256];
	static const word32 Te1[256];
	static const word32 Te2[256];
	static const word32 Te3[256];
	static const word32 Te4[256];

	static const word32 Td0[256];
	static const word32 Td1[256];
	static const word32 Td2[256];
	static const word32 Td3[256];
	static const word32 Td4[256];

	static const word32 rcon[];

	unsigned int m_rounds;
	SecBlock<word32> m_key;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Rijndael">Rijndael</a>
class RijndaelEncryption : public Rijndael
{
public:
	RijndaelEncryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH)
		: Rijndael(userKey, keylength) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{RijndaelEncryption::ProcessBlock(inoutBlock, inoutBlock);}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Rijndael">Rijndael</a>
class RijndaelDecryption : public Rijndael
{
public:
	RijndaelDecryption(const byte *userKey, unsigned int keylength=DEFAULT_KEYLENGTH);

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{RijndaelDecryption::ProcessBlock(inoutBlock, inoutBlock);}
};

NAMESPACE_END

#endif
