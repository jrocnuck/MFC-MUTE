 // mdc.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_MDC_H
#define CRYPTOPP_MDC_H

/** \file
*/

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/cs.html#MDC">MDC</a>
template <class T> class MDC : public FixedBlockSize<T::DIGESTSIZE>, public FixedKeyLength<T::BLOCKSIZE>
{
public:
	MDC(const byte *userKey, unsigned int = 0)
		: key(KEYLENGTH/4)
	{
		T::CorrectEndianess(key, (word32 *)userKey, KEYLENGTH);
	}

	void ProcessBlock(byte *inoutBlock) const
	{
		T::CorrectEndianess((word32 *)inoutBlock, (word32 *)inoutBlock, BLOCKSIZE);
		T::Transform((word32 *)inoutBlock, key);
		T::CorrectEndianess((word32 *)inoutBlock, (word32 *)inoutBlock, BLOCKSIZE);
	}

	void ProcessBlock(const byte *inBlock, byte *outBlock) const
	{
		T::CorrectEndianess((word32 *)outBlock, (word32 *)inBlock, BLOCKSIZE);
		T::Transform((word32 *)outBlock, key);
		T::CorrectEndianess((word32 *)outBlock, (word32 *)outBlock, BLOCKSIZE);
	}

private:
	SecBlock<word32> key;
};

NAMESPACE_END

#endif
