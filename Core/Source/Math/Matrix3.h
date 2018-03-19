//
// Represents a 3x3 matrix while occuping a 3x4 memory footprint. Use row representation for matrix layout.
//

#pragma once

#include "Quaternion.h"

namespace Math
{
	__declspec(align(16)) class Matrix3
	{
	public:
		INLINE Matrix3() {}
		INLINE Matrix3(Vector3 x, Vector3 y, Vector3 z) { m_mat[0] = x; m_mat[1] = y; m_mat[2] = z; }
		INLINE Matrix3(const Matrix3& m) { m_mat[0] = m.m_mat[0]; m_mat[1] = m.m_mat[1]; m_mat[2] = m.m_mat[2]; }
		INLINE Matrix3(Quaternion q) { *this = Matrix3(DirectX::XMMatrixRotationQuaternion(q)); }
		INLINE explicit Matrix3(const DirectX::XMMATRIX& m) { m_mat[0] = Vector3(m.r[0]); m_mat[1] = Vector3(m.r[1]); m_mat[2] = Vector3(m.r[2]); }
		INLINE explicit Matrix3(EIdentityTag) { m_mat[0] = Vector3(EXUnitVector::kXUnitVector); m_mat[1] = Vector3(EYUnitVector::kYUnitVector); m_mat[2] = Vector3(EZUnitVector::kZUnitVector); }
		INLINE explicit Matrix3(EZeroTag) { m_mat[0] = m_mat[1] = m_mat[2] = Vector3(EZeroTag::kZero); }

		INLINE void SetX(Vector3 x) { m_mat[0] = x; }
		INLINE void SetY(Vector3 y) { m_mat[1] = y; }
		INLINE void SetZ(Vector3 z) { m_mat[2] = z; }

		INLINE Vector3 GetX() const { return m_mat[0]; }
		INLINE Vector3 GetY() const { return m_mat[1]; }
		INLINE Vector3 GetZ() const { return m_mat[2]; }

		static INLINE Matrix3 MakeXRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationX(angle)); }
		static INLINE Matrix3 MakeYRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationY(angle)); }
		static INLINE Matrix3 MakeZRotation(float angle) { return Matrix3(DirectX::XMMatrixRotationZ(angle)); }
		static INLINE Matrix3 MakeScale(float scale) { return Matrix3(DirectX::XMMatrixScaling(scale, scale, scale)); }
		static INLINE Matrix3 MakeScale(float sx, float sy, float sz) { return Matrix3(DirectX::XMMatrixScaling(sx, sy, sz)); }
		static INLINE Matrix3 MakeScale(Vector3 scale) { return Matrix3(DirectX::XMMatrixScalingFromVector(scale)); }

		INLINE operator DirectX::XMMATRIX() const { return DirectX::XMMATRIX(m_mat[0], m_mat[1], m_mat[2], CreateWUnitVector()); }

		INLINE Vector3 operator* (Vector3 vec) const { return Vector3(DirectX::XMVector3TransformNormal(vec, *this)); }
		INLINE Matrix3 operator* (const Matrix3& mat) const { return Matrix3(*this * mat.GetX(), *this * mat.GetY(), *this * mat.GetZ()); }

	private:
		Vector3 m_mat[3];
	};

	// inline functions.
	INLINE Matrix3 Transpose(const Matrix3& mat) { return Matrix3(DirectX::XMMatrixTranspose(mat)); }
	// inline Matrix3 Inverse( const Matrix3& mat ) { TBD }

}	// namespace Math