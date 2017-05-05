#ifndef CRYPTOPP_HEX_H
#define CRYPTOPP_HEX_H

#include "cryptlib.h"
#include "filters.h"

NAMESPACE_BEGIN(CryptoPP)

//! Converts given data to base 16
class HexEncoder : public Filter
{
public:
	HexEncoder(BufferedTransformation *outQueue = NULL, bool uppercase = true);

	void Put(byte inByte)
	{
		AttachedTransformation()->Put(m_vec[inByte >> 4]);
		AttachedTransformation()->Put(m_vec[inByte & 0x0F]);
	}

	void Put(const byte *inString, unsigned int length);

private:
	const byte *m_vec;
};

//! Decode 16 bit data back to bytes
class HexDecoder : public Filter
{
public:
	HexDecoder(BufferedTransformation *outQueue = NULL)
		: Filter(outQueue) {last = -1;}

	void Put(byte inByte)
	{
		int i=ConvToNumber(inByte);
		if (i >= 0)
		{
			if (last >= 0)
			{
				AttachedTransformation()->Put((last << 4) | i);
				last = -1;
			}
			else
				last = i;
		}
	}

	void Put(const byte *inString, unsigned int length);

	void MessageEnd(int propagate=-1);

private:
	static int ConvToNumber(byte inByte)
	{
		if (inByte >= '0' && inByte <= '9')
			return inByte - '0';
		if (inByte >= 'A' && inByte <= 'F')
			return inByte - 'A' + 10;
		if (inByte >= 'a' && inByte <= 'f')
			return inByte - 'a' + 10;
		return -1;
	}

	int last;
};

NAMESPACE_END

#endif
