#ifndef CRYPTOPP_EPRECOMP_H
#define CRYPTOPP_EPRECOMP_H

#include "integer.h"
#include "algebra.h"
#include <vector>

NAMESPACE_BEGIN(CryptoPP)

/**
   Please do not directly use the following class.  It should be
   considered a private class for the library.  The following
   classes are public and use ExponentiationPrecomputation internally. <br><br>
 
   ModExpPrecomputation; <br>
   EcPrecomputation<EC2N>; <br>
   EcPrecomputation<ECP>;
*/
template <class T> class ExponentiationPrecomputation
{
public:
	typedef T Element;
	typedef AbstractGroup<T> Group;

	ExponentiationPrecomputation() : m_group(NULL) {}

	void SetGroupAndBase(const Group &group, const Element &base);
	void Precompute(unsigned int maxExpBits, unsigned int storage);
	void PrepareCascade(std::vector<BaseAndExponent<Element> > &eb, const Integer &exponent) const;
	Element Exponentiate(const Integer &exponent) const;
	Element CascadeExponentiate(const Integer &exponent, const ExponentiationPrecomputation<T> &pc2, const Integer &exponent2) const;

	const Group *m_group;
	unsigned int m_windowSize;
	Integer m_exponentBase;			// what base to represent the exponent in
	std::vector<Element> m_bases;	// precalculated bases
};

NAMESPACE_END

#endif
