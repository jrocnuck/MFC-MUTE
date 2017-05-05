#ifndef CRYPTOPP_RANDPOOL_H
#define CRYPTOPP_RANDPOOL_H

#include "cryptlib.h"
#include "filters.h"

NAMESPACE_BEGIN(CryptoPP)

//! Randomness Pool
/*! This class can be used to generate
pseudorandom bytes after seeding the pool with
the Put() methods */
class RandomPool : public RandomNumberGenerator,
				   public Sink
{
public:
	//! poolSize must be greater than 16
	RandomPool(unsigned int poolSize=384);

	//! seed the pool
	void Put(byte inByte);
	//! seed the pool
	void Put(const byte *inString, unsigned int length);
	
	byte GenerateByte();
	void GenerateBlock(byte *output, unsigned int size);

protected:
	void Stir();

private:
	SecByteBlock pool, key;
	unsigned int addPos, getPos;
};

NAMESPACE_END

#endif
