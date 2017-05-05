// randpool.cpp - written and placed in the public domain by Wei Dai
// The algorithm in this module comes from PGP's randpool.c

#include "pch.h"
#include "randpool.h"
#include "mdc.h"
#include "sha.h"
#include "modes.h"

NAMESPACE_BEGIN(CryptoPP)

typedef MDC<SHA> RandomPoolCipher;

RandomPool::RandomPool(unsigned int poolSize)
	: pool(poolSize), key(RandomPoolCipher::DEFAULT_KEYLENGTH)
{
	assert(poolSize > key.size);

	addPos=0;
	getPos=poolSize;
	memset(pool, 0, poolSize);
	memset(key, 0, key.size);
}

void RandomPool::Stir()
{
	for (int i=0; i<2; i++)
	{
		RandomPoolCipher cipher(key);
		CFBEncryption cfb(cipher, pool+pool.size-cipher.BlockSize());
		cfb.ProcessString(pool, pool.size);
		memcpy(key, pool, key.size);
	}

	addPos = 0;
	getPos = key.size;
}

void RandomPool::Put(byte inByte)
{
	if (addPos == pool.size)
		Stir();

	pool[addPos++] ^= inByte;
	getPos = pool.size; // Force stir on get
}

void RandomPool::Put(const byte *inString, unsigned int length)
{
	unsigned t;

	while (length > (t = pool.size - addPos))
	{
		xorbuf(pool+addPos, inString, t);
		inString += t;
		length -= t;
		Stir();
	}

	if (length)
	{
		xorbuf(pool+addPos, inString, length);
		addPos += length;
		getPos = pool.size; // Force stir on get
	}
}

byte RandomPool::GenerateByte()
{
	if (getPos == pool.size)
		Stir();

	return pool[getPos++];
}

void RandomPool::GenerateBlock(byte *outString, unsigned int size)
{
	unsigned t;

	while (size > (t = pool.size - getPos))
	{
		memcpy(outString, pool+getPos, t);
		outString += t;
		size -= t;
		Stir();
	}

	if (size)
	{
		memcpy(outString, pool+getPos, size);
		getPos += size;
	}
}

NAMESPACE_END
