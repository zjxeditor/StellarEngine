//
// Scalar operations with DirectXMath.
//

#pragma once

#include "Common.h"

namespace Math
{
	class Scalar
	{
	public:
		INLINE Scalar() {}
		INLINE Scalar(const Scalar& s) { m_vec = s; }
		INLINE Scalar(float f) { m_vec = DirectX::XMVectorReplicate(f); }
		INLINE explicit Scalar(DirectX::FXMVECTOR vec) { m_vec = vec; }
		INLINE explicit Scalar(EZeroTag) { m_vec = SplatZero(); }
		INLINE explicit Scalar(EIdentityTag) { m_vec = SplatOne(); }

		INLINE operator DirectX::XMVECTOR() const { return m_vec; }
		INLINE operator float() const { return DirectX::XMVectorGetX(m_vec); }

	private:
		DirectX::XMVECTOR m_vec;
	};

	// Inline methods.
	INLINE Scalar operator- (Scalar s) { return Scalar(DirectX::XMVectorNegate(s)); }
	INLINE Scalar operator+ (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorAdd(s1, s2)); }
	INLINE Scalar operator- (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorSubtract(s1, s2)); }
	INLINE Scalar operator* (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorMultiply(s1, s2)); }
	INLINE Scalar operator/ (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorDivide(s1, s2)); }
	INLINE Scalar operator+ (Scalar s1, float s2) { return s1 + Scalar(s2); }
	INLINE Scalar operator- (Scalar s1, float s2) { return s1 - Scalar(s2); }
	INLINE Scalar operator* (Scalar s1, float s2) { return s1 * Scalar(s2); }
	INLINE Scalar operator/ (Scalar s1, float s2) { return s1 / Scalar(s2); }
	INLINE Scalar operator+ (float s1, Scalar s2) { return Scalar(s1) + s2; }
	INLINE Scalar operator- (float s1, Scalar s2) { return Scalar(s1) - s2; }
	INLINE Scalar operator* (float s1, Scalar s2) { return Scalar(s1) * s2; }
	INLINE Scalar operator/ (float s1, Scalar s2) { return Scalar(s1) / s2; }

	// To allow floats to implicitly construct Scalars, we need to clarify these operators and suppress upconversion.
	INLINE bool operator<  (Scalar lhs, float rhs) { return (float)lhs <  rhs; }
	INLINE bool operator<= (Scalar lhs, float rhs) { return (float)lhs <= rhs; }
	INLINE bool operator>  (Scalar lhs, float rhs) { return (float)lhs >  rhs; }
	INLINE bool operator>= (Scalar lhs, float rhs) { return (float)lhs >= rhs; }
	INLINE bool operator== (Scalar lhs, float rhs) { return (float)lhs == rhs; }
	INLINE bool operator<  (float lhs, Scalar rhs) { return lhs <  (float)rhs; }
	INLINE bool operator<= (float lhs, Scalar rhs) { return lhs <= (float)rhs; }
	INLINE bool operator>  (float lhs, Scalar rhs) { return lhs >  (float)rhs; }
	INLINE bool operator>= (float lhs, Scalar rhs) { return lhs >= (float)rhs; }
	INLINE bool operator== (float lhs, Scalar rhs) { return lhs == (float)rhs; }

}	// namespace Math
