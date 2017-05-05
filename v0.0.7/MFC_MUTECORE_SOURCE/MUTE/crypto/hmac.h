// hmac.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_HMAC_H
#define CRYPTOPP_HMAC_H

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

//! <a href="http://www.weidai.com/scan-mirror/mac.html#HMAC">HMAC</a>
/*! HMAC(K, text) = H(K XOR opad, H(K XOR ipad, text)) */
template <class T> class HMAC : public MessageAuthenticationCode, public VariableKeyLength<16, 0, UINT_MAX>
{
public:
	// put enums here for Metrowerks 4
	enum {DIGESTSIZE=T::DIGESTSIZE, BLOCKSIZE=T::BLOCKSIZE};

	// CW50 workaround: can't use DEFAULT_KEYLENGTH here
	HMAC(const byte *userKey, unsigned int keylength = 16);
	void Update(const byte *input, unsigned int length);
	void TruncatedFinal(byte *mac, unsigned int size);
	unsigned int DigestSize() const {return DIGESTSIZE;}

private:
	enum {IPAD=0x36, OPAD=0x5c};

	void Init();

	SecByteBlock k_ipad, k_opad;
	T hash;
};

template <class T>
HMAC<T>::HMAC(const byte *userKey, unsigned int keylength)
	: k_ipad(T::BLOCKSIZE), k_opad(T::BLOCKSIZE)
{
	assert(keylength == KeyLength(keylength));

	if (keylength <= T::BLOCKSIZE)
		memcpy(k_ipad, userKey, keylength);
	else
	{
		T().CalculateDigest(k_ipad, userKey, keylength);
		keylength = T::DIGESTSIZE;
	}

	assert(keylength <= T::BLOCKSIZE);
	memset(k_ipad+keylength, 0, T::BLOCKSIZE-keylength);

	for (unsigned int i=0; i<T::BLOCKSIZE; i++)
	{
		k_opad[i] = k_ipad[i] ^ OPAD;
		k_ipad[i] ^= IPAD;
	}

	Init();
}

template <class T>
void HMAC<T>::Init()
{
	hash.Update(k_ipad, T::BLOCKSIZE);
}

template <class T>
void HMAC<T>::Update(const byte *input, unsigned int length)
{
	hash.Update(input, length);
}

template <class T>
void HMAC<T>::TruncatedFinal(byte *mac, unsigned int size)
{
	hash.Final(mac);

	hash.Update(k_opad, T::BLOCKSIZE);
	hash.Update(mac, DIGESTSIZE);
	hash.TruncatedFinal(mac, size);
	Init();
}

NAMESPACE_END

#endif
