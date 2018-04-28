//
// 4x4 matrix operations with DirectXMath. Use row major matrix layout.
//

#pragma once

#include "Transform.h"

namespace Math {

__declspec(align(16)) class Matrix4 {
public:
	INLINE Matrix4() {}
	INLINE Matrix4(Vector3 x, Vector3 y, Vector3 z, Vector3 w) {
		m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
		m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
	}
	INLINE Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
	INLINE Matrix4(const Matrix4& mat) { m_mat = mat.m_mat; }
	INLINE Matrix4(const Matrix3& mat) {
		m_mat.r[0] = SetWToZero(mat.GetX());
		m_mat.r[1] = SetWToZero(mat.GetY());
		m_mat.r[2] = SetWToZero(mat.GetZ());
		m_mat.r[3] = CreateWUnitVector();
	}
	INLINE Matrix4(const Matrix3& xyz, Vector3 w) {
		m_mat.r[0] = SetWToZero(xyz.GetX());
		m_mat.r[1] = SetWToZero(xyz.GetY());
		m_mat.r[2] = SetWToZero(xyz.GetZ());
		m_mat.r[3] = SetWToOne(w);
	}
	INLINE Matrix4(const AffineTransform& xform) { *this = Matrix4(xform.GetBasis(), xform.GetTranslation()); }
	INLINE Matrix4(const OrthogonalTransform& xform) { *this = Matrix4(Matrix3(xform.GetRotation()), xform.GetTranslation()); }
	INLINE explicit Matrix4(const DirectX::XMMATRIX& mat) { m_mat = mat; }
	INLINE explicit Matrix4(EIdentityTag) { m_mat = DirectX::XMMatrixIdentity(); }
	INLINE explicit Matrix4(EZeroTag) { m_mat.r[0] = m_mat.r[1] = m_mat.r[2] = m_mat.r[3] = SplatZero(); }

	INLINE const Matrix3& Get3x3() const { return (const Matrix3&)*this; }

	INLINE Vector4 GetX() const { return Vector4(m_mat.r[0]); }
	INLINE Vector4 GetY() const { return Vector4(m_mat.r[1]); }
	INLINE Vector4 GetZ() const { return Vector4(m_mat.r[2]); }
	INLINE Vector4 GetW() const { return Vector4(m_mat.r[3]); }

	INLINE void SetX(Vector4 x) { m_mat.r[0] = x; }
	INLINE void SetY(Vector4 y) { m_mat.r[1] = y; }
	INLINE void SetZ(Vector4 z) { m_mat.r[2] = z; }
	INLINE void SetW(Vector4 w) { m_mat.r[3] = w; }

	INLINE operator DirectX::XMMATRIX() const { return m_mat; }

	INLINE Vector4 operator* (Vector3 vec) const { return Vector4(DirectX::XMVector3Transform(vec, m_mat)); }
	INLINE Vector4 operator* (Vector4 vec) const { return Vector4(DirectX::XMVector4Transform(vec, m_mat)); }
	INLINE Matrix4 operator* (const Matrix4& mat) const { return Matrix4(DirectX::XMMatrixMultiply(mat, m_mat)); }

	static INLINE Matrix4 MakeScale(float scale) { return Matrix4(DirectX::XMMatrixScaling(scale, scale, scale)); }
	static INLINE Matrix4 MakeScale(Vector3 scale) { return Matrix4(DirectX::XMMatrixScalingFromVector(scale)); }

private:
	DirectX::XMMATRIX m_mat;
};

// Inline methods.
INLINE Matrix4 Transpose(const Matrix4& mat) { return Matrix4(DirectX::XMMatrixTranspose(mat)); }
INLINE Matrix4 Invert(const Matrix4& mat) { return Matrix4(DirectX::XMMatrixInverse(nullptr, mat)); }
INLINE Matrix4 OrthoInvert(const Matrix4& xform) {
	Matrix3 basis = Transpose(xform.Get3x3());
	Vector3 translate = basis * -Vector3(xform.GetW());
	return Matrix4(basis, translate);
}

}	// namespace Math
