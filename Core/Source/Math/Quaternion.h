//
// Quaternion operations with DirectXMath.
//

#pragma once

#include "Vector.h"

namespace Math
{
	class Quaternion
	{
	public:
		INLINE Quaternion() { m_vec = DirectX::XMQuaternionIdentity(); }
		INLINE Quaternion(const Vector3& axis, const Scalar& angle) { m_vec = DirectX::XMQuaternionRotationAxis(axis, angle); }
		INLINE Quaternion(float pitch, float yaw, float roll) { m_vec = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll); }
		INLINE explicit Quaternion(const DirectX::XMMATRIX& matrix) { m_vec = DirectX::XMQuaternionRotationMatrix(matrix); }
		INLINE explicit Quaternion(DirectX::FXMVECTOR vec) { m_vec = vec; }
		INLINE explicit Quaternion(EIdentityTag) { m_vec = DirectX::XMQuaternionIdentity(); }

		INLINE operator DirectX::XMVECTOR() const { return m_vec; }

		INLINE Quaternion operator~ (void) const { return Quaternion(DirectX::XMQuaternionConjugate(m_vec)); }
		INLINE Quaternion operator- (void) const { return Quaternion(DirectX::XMVectorNegate(m_vec)); }

		INLINE Quaternion operator* (Quaternion rhs) const { return Quaternion(DirectX::XMQuaternionMultiply(rhs, m_vec)); }
		INLINE Vector3 operator* (Vector3 rhs) const { return Vector3(DirectX::XMVector3Rotate(rhs, m_vec)); }

		INLINE Quaternion& operator= (Quaternion rhs) { m_vec = rhs; return *this; }
		INLINE Quaternion& operator*= (Quaternion rhs) { *this = *this * rhs; return *this; }

	protected:
		DirectX::XMVECTOR m_vec;
	};

	// Inline methods.
	INLINE Quaternion Normalize(Quaternion q) { return Quaternion(DirectX::XMQuaternionNormalize(q)); }

}	// namespace Math
