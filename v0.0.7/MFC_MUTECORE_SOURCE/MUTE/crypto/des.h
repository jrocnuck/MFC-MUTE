#ifndef CRYPTOPP_DES_H
#define CRYPTOPP_DES_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/* The DES implementation in Crypto++ ignores the parity bits
   (the least significant bits of each byte) in the key. However
   you can use these two functions to check or correct the parity
   bits if you wish. */
bool DES_CheckKeyParityBits(const byte *key); //!< Checks DES Key for correct parity
void DES_CorrectKeyParityBits(byte *key);     //!< Corrects a Key for correct parity

/// base class, do not use directly
class DES : public FixedBlockSize<8>, public FixedKeyLength<8>
{
public:
	DES(const byte *userKey, CipherDir);

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES::ProcessBlock(inoutBlock, inoutBlock);}

	// exposed for faster Triple-DES
	void RawProcessBlock(word32 &l, word32 &r) const;

protected:
	static const word32 Spbox[8][64];

	SecBlock<word32> k;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DES">DES</a>
class DESEncryption : public DES
{
public:
	DESEncryption(const byte * userKey, unsigned int = 0)
		: DES (userKey, ENCRYPTION) {}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DES">DES</a>
class DESDecryption : public DES
{
public:
	DESDecryption(const byte * userKey, unsigned int = 0)
		: DES (userKey, DECRYPTION) {}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DESede">DES-EDE2</a>
class DES_EDE2_Encryption : public FixedBlockSize<8>, public FixedKeyLength<16>
{
public:
	DES_EDE2_Encryption(const byte * userKey, unsigned int = 0)
		: e(userKey, ENCRYPTION), d(userKey + DES::KEYLENGTH, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES_EDE2_Encryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	DES e, d;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DESede">DES-EDE2</a>
class DES_EDE2_Decryption : public FixedBlockSize<8>, public FixedKeyLength<16>
{
public:
	DES_EDE2_Decryption(const byte * userKey, unsigned int = 0)
		: d(userKey, DECRYPTION), e(userKey + DES::KEYLENGTH, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES_EDE2_Decryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	DES d, e;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DESede">DES-EDE3</a>
class DES_EDE3_Encryption : public FixedBlockSize<8>, public FixedKeyLength<24>
{
public:
	DES_EDE3_Encryption(const byte * userKey, unsigned int = 0)
		: e1(userKey, ENCRYPTION), d2(userKey + DES::KEYLENGTH, DECRYPTION),
		  e3(userKey + 2*DES::KEYLENGTH, ENCRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES_EDE3_Encryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	DES e1, d2, e3;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DESede">DES-EDE3</a>
class DES_EDE3_Decryption : public FixedBlockSize<8>, public FixedKeyLength<24>
{
public:
	DES_EDE3_Decryption(const byte * userKey, unsigned int = 0)
		: d1(userKey, DECRYPTION), e2(userKey + DES::KEYLENGTH, ENCRYPTION),
		  d3(userKey + 2*DES::KEYLENGTH, DECRYPTION) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES_EDE3_Decryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	DES d1, e2, d3;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DESX">DES-XEX3</a>, AKA DESX
class DES_XEX3_Encryption : public FixedBlockSize<8>, public FixedKeyLength<24>
{
public:
	DES_XEX3_Encryption(const byte * userKey, unsigned int = 0)
		: x1(userKey, 8), e2(userKey + 8, ENCRYPTION), x3(userKey + 16, 8) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES_XEX3_Encryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	SecByteBlock x1;
	DES e2;
	SecByteBlock x3;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#DESX">DES-XEX3</a>, AKA DESX
class DES_XEX3_Decryption : public FixedBlockSize<8>, public FixedKeyLength<24>
{
public:
	DES_XEX3_Decryption(const byte * userKey, unsigned int = 0)
		: x1(userKey, 8), d2(userKey + 8, DECRYPTION), x3(userKey + 16, 8) {}

	void ProcessBlock(const byte *inBlock, byte * outBlock) const;
	void ProcessBlock(byte * inoutBlock) const
		{DES_XEX3_Decryption::ProcessBlock(inoutBlock, inoutBlock);}

private:
	SecByteBlock x1;
	DES d2;
	SecByteBlock x3;
};

NAMESPACE_END

#endif
