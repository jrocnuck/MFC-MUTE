// rsa.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "rsa.h"
#include "asn.h"
#include "oids.h"
#include "nbtheory.h"
#include "sha.h"

#include "pubkey.cpp"
#include "oaep.cpp"

NAMESPACE_BEGIN(CryptoPP)

INSTANTIATE_PUBKEY_TEMPLATES_MACRO(PKCS_EncryptionPaddingScheme, PKCS_SignaturePaddingScheme, RSAFunction, InvertibleRSAFunction)
template class OAEP<SHA>;
INSTANTIATE_PUBKEY_CRYPTO_TEMPLATES_MACRO(OAEP<SHA>, RSAFunction, InvertibleRSAFunction)

RSAFunction::RSAFunction(BufferedTransformation &bt)
{
	BERSequenceDecoder subjectPublicKeyInfo(bt);
	if (subjectPublicKeyInfo.PeekByte() == INTEGER)
	{
		// for backwards compatibility
		n.BERDecode(subjectPublicKeyInfo);
		e.BERDecode(subjectPublicKeyInfo);
	}
	else
	{
		BERSequenceDecoder algorithm(subjectPublicKeyInfo);
			ASN1::rsaEncryption().BERDecodeAndCheck(algorithm);
			BERDecodeNull(algorithm);
		algorithm.MessageEnd();

		BERSequenceDecoder subjectPublicKey(subjectPublicKeyInfo, BIT_STRING);
			subjectPublicKey.CheckByte(0);	// unused bits
			BERSequenceDecoder seq(subjectPublicKey);
				n.BERDecode(seq);
				e.BERDecode(seq);
			seq.MessageEnd();
		subjectPublicKey.MessageEnd();
	}
	subjectPublicKeyInfo.MessageEnd();
}

void RSAFunction::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder subjectPublicKeyInfo(bt);

		DERSequenceEncoder algorithm(subjectPublicKeyInfo);
			ASN1::rsaEncryption().DEREncode(algorithm);
			DEREncodeNull(algorithm);
		algorithm.MessageEnd();

		DERGeneralEncoder subjectPublicKey(subjectPublicKeyInfo, BIT_STRING);
			subjectPublicKey.Put(0);	// unused bits
			DERSequenceEncoder seq(subjectPublicKey);
				n.DEREncode(seq);
				e.DEREncode(seq);
			seq.MessageEnd();
		subjectPublicKey.MessageEnd();

	subjectPublicKeyInfo.MessageEnd();
}

Integer RSAFunction::ApplyFunction(const Integer &x) const
{
	return a_exp_b_mod_c(x, e, n);
}

// *****************************************************************************

InvertibleRSAFunction::InvertibleRSAFunction(const Integer &n, const Integer &e, const Integer &d,
	const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u)
		: RSAFunction(n, e), d(d), p(p), q(q), dp(dp), dq(dq), u(u)
{
	assert(p*q==n);
	assert(d*e%LCM(p-1, q-1)==1);
	assert(dp==d%(p-1));
	assert(dq==d%(q-1));
	assert(u*q%p==1);
}

// generate a random private key
InvertibleRSAFunction::InvertibleRSAFunction(RandomNumberGenerator &rng, unsigned int keybits, const Integer &eStart)
{
	assert(keybits >= 16);
	// generate 2 random primes of suitable size
	if (keybits%2==0)
	{
		const Integer minP = Integer(182) << (keybits/2-8);
		const Integer maxP = Integer::Power2(keybits/2)-1;
		p.Randomize(rng, minP, maxP, Integer::PRIME);
		q.Randomize(rng, minP, maxP, Integer::PRIME);
	}
	else
	{
		const Integer minP = Integer::Power2((keybits-1)/2);
		const Integer maxP = Integer(181) << ((keybits+1)/2-8);
		p.Randomize(rng, minP, maxP, Integer::PRIME);
		q.Randomize(rng, minP, maxP, Integer::PRIME);
	}

	// pre-calculate some other data for faster speed
	const Integer lcm = LCM(p-1, q-1);
	// make sure e starts odd
	for (e = eStart+(1-eStart%2); GCD(e, lcm)!=1; ++e, ++e)
		;
	d = EuclideanMultiplicativeInverse(e, lcm);
	dp = d % (p-1);
	dq = d % (q-1);
	u = EuclideanMultiplicativeInverse(q, p);
	n = p * q;
	assert(n.BitCount() == keybits);
}

InvertibleRSAFunction::InvertibleRSAFunction(BufferedTransformation &bt)
{
	BERSequenceDecoder privateKeyInfo(bt);
	word32 version;
	BERDecodeUnsigned<word32>(privateKeyInfo, version, INTEGER, 0, 0);	// check version

	if (privateKeyInfo.PeekByte() == INTEGER)
	{
		// for backwards compatibility
		n.BERDecode(privateKeyInfo);
		e.BERDecode(privateKeyInfo);
		d.BERDecode(privateKeyInfo);
		p.BERDecode(privateKeyInfo);
		q.BERDecode(privateKeyInfo);
		dp.BERDecode(privateKeyInfo);
		dq.BERDecode(privateKeyInfo);
		u.BERDecode(privateKeyInfo);
	}
	else
	{
		BERSequenceDecoder algorithm(privateKeyInfo);
			ASN1::rsaEncryption().BERDecodeAndCheck(algorithm);
			BERDecodeNull(algorithm);
		algorithm.MessageEnd();

		BERGeneralDecoder octetString(privateKeyInfo, OCTET_STRING);
			BERSequenceDecoder privateKey(octetString);
				BERDecodeUnsigned<word32>(privateKey, version, INTEGER, 0, 0);	// check version
				n.BERDecode(privateKey);
				e.BERDecode(privateKey);
				d.BERDecode(privateKey);
				p.BERDecode(privateKey);
				q.BERDecode(privateKey);
				dp.BERDecode(privateKey);
				dq.BERDecode(privateKey);
				u.BERDecode(privateKey);
			privateKey.MessageEnd();
		octetString.MessageEnd();
	}
	privateKeyInfo.MessageEnd();
}

void InvertibleRSAFunction::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder privateKeyInfo(bt);
		DEREncodeUnsigned<word32>(privateKeyInfo, 0);	// version

		DERSequenceEncoder algorithm(privateKeyInfo);
			ASN1::rsaEncryption().DEREncode(algorithm);
			DEREncodeNull(algorithm);
		algorithm.MessageEnd();

		DERGeneralEncoder octetString(privateKeyInfo, OCTET_STRING);
			DERSequenceEncoder privateKey(octetString);
				DEREncodeUnsigned<word32>(privateKey, 0);	// version
				n.DEREncode(privateKey);
				e.DEREncode(privateKey);
				d.DEREncode(privateKey);
				p.DEREncode(privateKey);
				q.DEREncode(privateKey);
				dp.DEREncode(privateKey);
				dq.DEREncode(privateKey);
				u.DEREncode(privateKey);
			privateKey.MessageEnd();
		octetString.MessageEnd();

	privateKeyInfo.MessageEnd();
}

Integer InvertibleRSAFunction::CalculateInverse(const Integer &x) const 
{
	// here we follow the notation of PKCS #1 and let u=q inverse mod p
	// but in ModRoot, u=p inverse mod q, so we reverse the order of p and q
	return ModularRoot(x, dq, dp, q, p, u);
}

NAMESPACE_END
