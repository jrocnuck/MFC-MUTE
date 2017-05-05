#ifndef CRYPTOPP_RSA_H
#define CRYPTOPP_RSA_H

/** \file
	This file contains classes that implement the
	<a href="http://www.weidai.com/scan-mirror/ca.html#RSA">RSA</a>
	ciphers and signature schemes as defined in PKCS #1 v2.0.
*/

#include "pkcspad.h"
#include "oaep.h"
#include "integer.h"

NAMESPACE_BEGIN(CryptoPP)

//! .
class RSAFunction : virtual public TrapdoorFunction
{
public:
	RSAFunction(const Integer &n, const Integer &e) : n(n), e(e) {}
	RSAFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer ApplyFunction(const Integer &x) const;
	Integer PreimageBound() const {return n;}
	Integer ImageBound() const {return n;}

	const Integer& GetModulus() const {return n;}
	const Integer& GetExponent() const {return e;}

protected:
	RSAFunction() {}	// to be used only by InvertibleRSAFunction
	Integer n, e;	// these are only modified in constructors
};

//! .
class InvertibleRSAFunction : public RSAFunction, public InvertibleTrapdoorFunction
{
public:
	InvertibleRSAFunction(const Integer &n, const Integer &e, const Integer &d,
						  const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u);
	// generate a random private key
	InvertibleRSAFunction(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart=17);
	InvertibleRSAFunction(BufferedTransformation &bt);
	void DEREncode(BufferedTransformation &bt) const;

	Integer CalculateInverse(const Integer &x) const;

	const Integer& GetPrime1() const {return p;}
	const Integer& GetPrime2() const {return q;}
	const Integer& GetDecryptionExponent() const {return d;}

protected:
	Integer d, p, q, dp, dq, u;
};

//! .
template <class B>
class RSAPrivateKeyTemplate : public B
{
public:
	RSAPrivateKeyTemplate(const Integer &n, const Integer &e, const Integer &d,
				  const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u)
		: PublicKeyBaseTemplate<InvertibleRSAFunction>(
			InvertibleRSAFunction(n, e, d, p, q, dp, dq, u)) {}

	RSAPrivateKeyTemplate(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart=17)
		: PublicKeyBaseTemplate<InvertibleRSAFunction>(
			InvertibleRSAFunction(rng, keybits, eStart)) {}

	RSAPrivateKeyTemplate(const InvertibleRSAFunction &priv)
		: PublicKeyBaseTemplate<InvertibleRSAFunction>(priv) {}

	RSAPrivateKeyTemplate(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<InvertibleRSAFunction>(bt) {}
};

//! .
template <class B, class V>
class RSAPublicKeyTemplate : public B
{
public:
	RSAPublicKeyTemplate(const Integer &n, const Integer &e)
		: PublicKeyBaseTemplate<RSAFunction>(RSAFunction(n, e)) {}

	RSAPublicKeyTemplate(const V &priv)
		: PublicKeyBaseTemplate<RSAFunction>(priv.GetTrapdoorFunction()) {}

	RSAPublicKeyTemplate(const RSAFunction &pub)
		: PublicKeyBaseTemplate<RSAFunction>(pub) {}

	RSAPublicKeyTemplate(BufferedTransformation &bt)
		: PublicKeyBaseTemplate<RSAFunction>(bt) {}
};

// The two RSA encryption schemes defined in PKCS #1 v2.0
//! RSA PKCS1v15 Decryptor
typedef RSAPrivateKeyTemplate<DecryptorTemplate<PKCS_EncryptionPaddingScheme, InvertibleRSAFunction> >
	RSAES_PKCS1v15_Decryptor;
//! RSA PKCS1v15 Encryptor
typedef RSAPublicKeyTemplate<EncryptorTemplate<PKCS_EncryptionPaddingScheme, RSAFunction>, RSAES_PKCS1v15_Decryptor>
	RSAES_PKCS1v15_Encryptor;

//! RSA OAEP SHA Decryptor
typedef RSAPrivateKeyTemplate<DecryptorTemplate<OAEP<SHA>, InvertibleRSAFunction> >
	RSAES_OAEP_SHA_Decryptor;
//! RSA OAEP SHA Encryptor
typedef RSAPublicKeyTemplate<EncryptorTemplate<OAEP<SHA>, RSAFunction>, RSAES_OAEP_SHA_Decryptor>
	RSAES_OAEP_SHA_Encryptor;

// The three RSA signature schemes defined in PKCS #1 v2.0
//! RSA PKCS1v15 SHA Signer
typedef RSAPrivateKeyTemplate<SignerTemplate<DigestSignerTemplate<PKCS_SignaturePaddingScheme, InvertibleRSAFunction>, PKCS_DecoratedHashModule<SHA> > >
	RSASSA_PKCS1v15_SHA_Signer;
//! RSA PKCS1v15 SHA Verifier
typedef RSAPublicKeyTemplate<VerifierTemplate<DigestVerifierTemplate<PKCS_SignaturePaddingScheme, RSAFunction>, PKCS_DecoratedHashModule<SHA> >, RSASSA_PKCS1v15_SHA_Signer>
	RSASSA_PKCS1v15_SHA_Verifier;

//! RSA PKCS1v15 MD2 Signer
typedef RSAPrivateKeyTemplate<SignerTemplate<DigestSignerTemplate<PKCS_SignaturePaddingScheme, InvertibleRSAFunction>, PKCS_DecoratedHashModule<MD2> > >
	RSASSA_PKCS1v15_MD2_Signer;
//! RSA PKCS1v15 MD2 Verifier
typedef RSAPublicKeyTemplate<VerifierTemplate<DigestVerifierTemplate<PKCS_SignaturePaddingScheme, RSAFunction>, PKCS_DecoratedHashModule<MD2> >, RSASSA_PKCS1v15_MD2_Signer>
	RSASSA_PKCS1v15_MD2_Verifier;

//! RSA PKCS1v15 MD5 Signer
typedef RSAPrivateKeyTemplate<SignerTemplate<DigestSignerTemplate<PKCS_SignaturePaddingScheme, InvertibleRSAFunction>, PKCS_DecoratedHashModule<MD5> > >
	RSASSA_PKCS1v15_MD5_Signer;
//! RSA PKCS1v15 MD5 Verifier
typedef RSAPublicKeyTemplate<VerifierTemplate<DigestVerifierTemplate<PKCS_SignaturePaddingScheme, RSAFunction>, PKCS_DecoratedHashModule<MD5> >, RSASSA_PKCS1v15_MD5_Signer>
	RSASSA_PKCS1v15_MD5_Verifier;

NAMESPACE_END

#endif
