#ifndef CRYPTOPP_SHA_H
#define CRYPTOPP_SHA_H

#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/md.html#SHA-1">SHA-1</a>
class SHA : public IteratedHash<word32, true, 64>
{
public:
	enum {DIGESTSIZE = 20};
	SHA() : IteratedHash<word32, true, 64>(DIGESTSIZE) {Init();}
	static void Transform(word32 *digest, const word32 *data);

protected:
	void Init();
	void vTransform(const word32 *data) {Transform(digest, data);}
};

typedef SHA SHA1;

//! implements the SHA-256 standard
class SHA256 : public IteratedHash<word32, true, 64>
{
public:
	enum {DIGESTSIZE = 32};
	SHA256() : IteratedHash<word32, true, 64>(DIGESTSIZE) {Init();}
	static void Transform(word32 *digest, const word32 *data);

protected:
	void Init();
	void vTransform(const word32 *data) {Transform(digest, data);}

	const static word32 K[64];
};

#ifdef WORD64_AVAILABLE

//! implements the SHA-512 standard
class SHA512 : public IteratedHash<word64, true, 128>
{
public:
	enum {DIGESTSIZE = 64};
	SHA512() : IteratedHash<word64, true, 128>(DIGESTSIZE) {Init();}
	static void Transform(word64 *digest, const word64 *data);

protected:
	void Init();
	void vTransform(const word64 *data) {Transform(digest, data);}

	const static word64 K[80];
};

//! implements the SHA-384 standard
class SHA384 : public IteratedHash<word64, true, 128>
{
public:
	enum {DIGESTSIZE = 48};
	SHA384() : IteratedHash<word64, true, 128>(64) {Init();}
	unsigned int DigestSize() const {return DIGESTSIZE;};

protected:
	void Init();
	void vTransform(const word64 *data) {SHA512::Transform(digest, data);}
};

#endif

NAMESPACE_END

#endif
