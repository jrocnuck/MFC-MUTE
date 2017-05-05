/*
 * Modification History
 *
 * 2002-August-8   Jason Rohrer
 * Hacked to work on Mac OS X.
 */


// iterhash.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

template <class T>
IteratedHashBase<T>::IteratedHashBase(unsigned int blockSize, unsigned int digestSize)
	: blockSize(blockSize), countLo(0), countHi(0)
	, data(blockSize/sizeof(T)), digest(digestSize/sizeof(T))
{
}

template <class T> void IteratedHashBase<T>::Update(const byte *input, unsigned int len)
{
	HashWordType tmp = countLo;
	if ((countLo = tmp + ((word32)len << 3)) < tmp)
		countHi++;             // Carry from low to high
	countHi += len >> (8*sizeof(HashWordType)-3);

	assert((blockSize & (blockSize-1)) == 0);	// blockSize is a power of 2
	unsigned int num = (unsigned int)(tmp >> 3) & (blockSize-1);

	if (num != 0)
	{
		if ((num+len) >= blockSize)
		{
			memcpy((byte *)data.ptr+num, input, blockSize-num);
			HashBlock(data);
			input += (blockSize-num);
			len-=(blockSize - num);
			num=0;
			// drop through and do the rest
		}
		else
		{
			memcpy((byte *)data.ptr+num, input, len);
			return;
		}
	}

	// we now can process the input data in blocks of blockSize
	// chars and save the leftovers to this->data.
	if (len >= blockSize)
	{
		if (IsAligned<T>(input))
		{
			unsigned int leftOver = HashMultipleBlocks((T *)input, len);
			input += (len - leftOver);
			len = leftOver;
		}
		else
			do
			{   // copy input first if it's not aligned correctly
				memcpy(data, input, blockSize);
				HashBlock(data);
				input+=blockSize;
				len-=blockSize;
			} while (len >= blockSize);
	}

	memcpy(data, input, len);
}

template <class T> unsigned int IteratedHashBase<T>::HashMultipleBlocks(const T *input, unsigned int length)
{
	do
	{
		HashBlock(input);
		input += blockSize/sizeof(T);
		length -= blockSize;
	}
	while (length >= blockSize);
	return length;
}

template <class T> void IteratedHashBase<T>::PadLastBlock(unsigned int lastBlockSize, byte padFirst)
{
	unsigned int num = (unsigned int)(countLo >> 3) & (blockSize-1);
	assert(num < blockSize);
	((byte *)data.ptr)[num++]=padFirst;
	if (num <= lastBlockSize)
		memset((byte *)data.ptr+num, 0, lastBlockSize-num);
	else
	{
		memset((byte *)data.ptr+num, 0, blockSize-num);
		HashBlock(data);
		memset(data, 0, lastBlockSize);
	}
}

template <class T> void IteratedHashBase<T>::Reinit()
{
	countLo = countHi = 0;
	Init();
}

// provide empty definitions to avoid instantiation warnings
template <class T> void IteratedHashBase<T>::Init() {}
template <class T> void IteratedHashBase<T>::HashBlock(const T *input) {}

#ifdef WORD64_AVAILABLE
//template class IteratedHashBase<word64>;
#endif

//template class IteratedHashBase<word32>;





template <>
IteratedHashBase<word32>::IteratedHashBase(unsigned int blockSize, unsigned int digestSize)
	: blockSize(blockSize), countLo(0), countHi(0)
	, data(blockSize/sizeof(word32)), digest(digestSize/sizeof(word32))
{
}

// provide empty definitions to avoid instantiation warnings
template <> void IteratedHashBase<word32>::HashBlock(const word32 *input) {}
template <> void IteratedHashBase<word32>::Init() {}


template <> unsigned int IteratedHashBase<word32>::HashMultipleBlocks(const word32 *input, unsigned int length)
{
	do
	{
		HashBlock(input);
		input += blockSize/sizeof(word32);
		length -= blockSize;
	}
	while (length >= blockSize);
	return length;
}


template <> void IteratedHashBase<word32>::Update(const byte *input, unsigned int len)
{
	HashWordType tmp = countLo;
	if ((countLo = tmp + ((word32)len << 3)) < tmp)
		countHi++;             // Carry from low to high
	countHi += len >> (8*sizeof(HashWordType)-3);

	assert((blockSize & (blockSize-1)) == 0);	// blockSize is a power of 2
	unsigned int num = (unsigned int)(tmp >> 3) & (blockSize-1);

	if (num != 0)
	{
		if ((num+len) >= blockSize)
		{
			memcpy((byte *)data.ptr+num, input, blockSize-num);
			HashBlock(data);
			input += (blockSize-num);
			len-=(blockSize - num);
			num=0;
			// drop through and do the rest
		}
		else
		{
			memcpy((byte *)data.ptr+num, input, len);
			return;
		}
	}

	// we now can process the input data in blocks of blockSize
	// chars and save the leftovers to this->data.
	if (len >= blockSize)
	{
		if (IsAligned<word32>(input))
		{
			unsigned int leftOver = HashMultipleBlocks((word32 *)input, len);
			input += (len - leftOver);
			len = leftOver;
		}
		else
			do
			{   // copy input first if it's not aligned correctly
				memcpy(data, input, blockSize);
				HashBlock(data);
				input+=blockSize;
				len-=blockSize;
			} while (len >= blockSize);
	}

	memcpy(data, input, len);
}



template <> void IteratedHashBase<word32>::PadLastBlock(unsigned int lastBlockSize, byte padFirst)
{
	unsigned int num = (unsigned int)(countLo >> 3) & (blockSize-1);
	assert(num < blockSize);
	((byte *)data.ptr)[num++]=padFirst;
	if (num <= lastBlockSize)
		memset((byte *)data.ptr+num, 0, lastBlockSize-num);
	else
	{
		memset((byte *)data.ptr+num, 0, blockSize-num);
		HashBlock(data);
		memset(data, 0, lastBlockSize);
	}
}



template <> void IteratedHashBase<word32>::Reinit()
{
	countLo = countHi = 0;
	Init();
}










template <>
IteratedHashBase<word64>::IteratedHashBase(unsigned int blockSize, unsigned int digestSize)
	: blockSize(blockSize), countLo(0), countHi(0)
	, data(blockSize/sizeof(word64)), digest(digestSize/sizeof(word64))
{
}

// provide empty definitions to avoid instantiation warnings
template <> void IteratedHashBase<word64>::Init() {}
template <> void IteratedHashBase<word64>::HashBlock(const word64 *input) {}


template <> unsigned int IteratedHashBase<word64>::HashMultipleBlocks(const word64 *input, unsigned int length)
{
	do
	{
		HashBlock(input);
		input += blockSize/sizeof(word64);
		length -= blockSize;
	}
	while (length >= blockSize);
	return length;
}



template <> void IteratedHashBase<word64>::Update(const byte *input, unsigned int len)
{
	HashWordType tmp = countLo;
	if ((countLo = tmp + ((word32)len << 3)) < tmp)
		countHi++;             // Carry from low to high
	countHi += len >> (8*sizeof(HashWordType)-3);

	assert((blockSize & (blockSize-1)) == 0);	// blockSize is a power of 2
	unsigned int num = (unsigned int)(tmp >> 3) & (blockSize-1);

	if (num != 0)
	{
		if ((num+len) >= blockSize)
		{
			memcpy((byte *)data.ptr+num, input, blockSize-num);
			HashBlock(data);
			input += (blockSize-num);
			len-=(blockSize - num);
			num=0;
			// drop through and do the rest
		}
		else
		{
			memcpy((byte *)data.ptr+num, input, len);
			return;
		}
	}

	// we now can process the input data in blocks of blockSize
	// chars and save the leftovers to this->data.
	if (len >= blockSize)
	{
		if (IsAligned<word64>(input))
		{
			unsigned int leftOver = HashMultipleBlocks((word64 *)input, len);
			input += (len - leftOver);
			len = leftOver;
		}
		else
			do
			{   // copy input first if it's not aligned correctly
				memcpy(data, input, blockSize);
				HashBlock(data);
				input+=blockSize;
				len-=blockSize;
			} while (len >= blockSize);
	}

	memcpy(data, input, len);
}



template <> void IteratedHashBase<word64>::PadLastBlock(unsigned int lastBlockSize, byte padFirst)
{
	unsigned int num = (unsigned int)(countLo >> 3) & (blockSize-1);
	assert(num < blockSize);
	((byte *)data.ptr)[num++]=padFirst;
	if (num <= lastBlockSize)
		memset((byte *)data.ptr+num, 0, lastBlockSize-num);
	else
	{
		memset((byte *)data.ptr+num, 0, blockSize-num);
		HashBlock(data);
		memset(data, 0, lastBlockSize);
	}
}

template <> void IteratedHashBase<word64>::Reinit()
{
	countLo = countHi = 0;
	Init();
}





NAMESPACE_END
