#ifndef CRYPTOPP_CBC_H
#define CRYPTOPP_CBC_H

/** \file
*/

#include "filters.h"
#include "modes.h"

NAMESPACE_BEGIN(CryptoPP)

//! <a href="http://www.weidai.com/scan-mirror/cs.html#mode_CBC">CBC mode</a>
/*! Compatible with FIPS 81.
	Padded with '\0's if plaintext length is not a multiple of block size.
*/
class CBCRawEncryptor : protected CipherMode, public FilterWithBufferedInput
{
public:
	CBCRawEncryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

protected:
	void FirstPut(const byte *) {}
	void NextPut(const byte *inString, unsigned int length);
	void LastPut(const byte *inString, unsigned int length);
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#mode_CBC">CBC mode</a>
/*! Compatible with FIPS 81.
	Padded with '\0's if plaintext length is not a multiple of block size.
*/
class CBCRawDecryptor : protected CipherMode, public FilterWithBufferedInput
{
public:
	CBCRawDecryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

protected:
	void FirstPut(const byte *) {}
	void NextPut(const byte *inString, unsigned int length);
	void LastPut(const byte *inString, unsigned int length);
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#mode_CBC">CBC mode</a> with <a href="http://www.weidai.com/scan-mirror/cs.html#pad_PKCSPadding">PKCS#5 padding</a>
/*! Compatible with RFC 1423 (and also 2040 and 2630).
*/
class CBCPaddedEncryptor : protected CipherMode, public FilterWithBufferedInput
{
public:
	CBCPaddedEncryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

protected:
	void FirstPut(const byte *) {}
	void NextPut(const byte *inString, unsigned int length);
	void LastPut(const byte *inString, unsigned int length);
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#mode_CBC">CBC mode</a> with <a href="http://www.weidai.com/scan-mirror/cs.html#pad_PKCSPadding">PKCS#5 padding</a>
/*! Compatible with RFC 1423 (and also 2040 and 2630).
*/
class CBCPaddedDecryptor : protected CipherMode, public FilterWithBufferedInput
{
public:
	CBCPaddedDecryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

protected:
	void FirstPut(const byte *) {}
	void NextPut(const byte *inString, unsigned int length);
	void LastPut(const byte *inString, unsigned int length);
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#pad_CTS">CBC/CTS mode</a> with <a href="http://www.weidai.com/scan-mirror/cs.html#pad_CTS">ciphertext stealing</a>
/*! Compatible with RFC 2040.
	Ciphertext stealing requires at least cipher.BlockSize()+1 bytes of plaintext.
	Shorter plaintext will be padded with '\0's unless IV stealing is specified.
*/
class CBC_CTS_Encryptor : protected CipherMode, public FilterWithBufferedInput
{
public:
	/*! If stealIV == true and length of plaintext < cipher.BlockSize()+1,
		IV will be modified, and the modified IV must be used for decryption.
		If stealIV == false or using the second constructor,
		shorter plaintexts will be padded with '\0's. */
	CBC_CTS_Encryptor(const BlockTransformation &cipher, byte *IV, BufferedTransformation *outQueue, bool stealIV);
	CBC_CTS_Encryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

protected:
	void FirstPut(const byte *inString);
	void NextPut(const byte *inString, unsigned int length);
	void LastPut(const byte *inString, unsigned int length);

	byte *m_iv;
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#pad_CTS">CBC/CTS mode</a> with <a href="http://www.weidai.com/scan-mirror/cs.html#pad_CTS">ciphertext stealing</a>
/*! Compatible with RFC 2040.
	Ciphertext stealing requires at least cipher.BlockSize()+1 bytes of plaintext.
	Shorter plaintext will be padded with '\0's unless IV stealing is specified.
*/
class CBC_CTS_Decryptor : protected CipherMode, public FilterWithBufferedInput
{
public:
	CBC_CTS_Decryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

protected:
	void FirstPut(const byte *inString) {}
	void NextPut(const byte *inString, unsigned int length);
	void LastPut(const byte *inString, unsigned int length);
};

NAMESPACE_END

#endif
