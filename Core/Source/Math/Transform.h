//
// Orthogonal transform and Affine transform operations with DirectXMath.
//

#pragma once

#include "Matrix3.h"

namespace Math
{
	// This transform strictly prohibits non-uniform scale. Scale itself is barely tolerated.
	__declspec(align(16)) class OrthogonalTransform
	{
	public:
		INLINE OrthogonalTransform() : m_rotation(EIdentityTag::kIdentity), m_translation(EZeroTag::kZero) {}
		INLINE OrthogonalTransform(Quaternion rotate) : m_rotation(rotate), m_translation(EZeroTag::kZero) {}
		INLINE OrthogonalTransform(Vector3 translate) : m_rotation(EIdentityTag::kIdentity), m_translation(translate) {}
		INLINE OrthogonalTransform(Quaternion rotate, Vector3 translate) : m_rotation(rotate), m_translation(translate) {}
		INLINE OrthogonalTransform(const Matrix3& mat) : m_rotation(mat), m_translation(EZeroTag::kZero) {}
		INLINE OrthogonalTransform(const Matrix3& mat, Vector3 translate) : m_rotation(mat), m_translation(translate) {}
		INLINE OrthogonalTransform(EIdentityTag) : m_rotation(EIdentityTag::kIdentity), m_translation(EZeroTag::kZero) {}
		INLINE explicit OrthogonalTransform(const DirectX::XMMATRIX& mat) { *this = OrthogonalTransform(Matrix3(mat), Vector3(mat.r[3])); }

		INLINE void SetRotation(Quaternion q) { m_rotation = q; }
		INLINE void SetTranslation(Vector3 v) { m_translation = v; }

		INLINE Quaternion GetRotation() const { return m_rotation; }
		INLINE Vector3 GetTranslation() const { return m_translation; }

		static INLINE OrthogonalTransform MakeXRotation(float angle) { return OrthogonalTransform(Quaternion(Vector3(EXUnitVector::kXUnitVector), angle)); }
		static INLINE OrthogonalTransform MakeYRotation(float angle) { return OrthogonalTransform(Quaternion(Vector3(EYUnitVector::kYUnitVector), angle)); }
		static INLINE OrthogonalTransform MakeZRotation(float angle) { return OrthogonalTransform(Quaternion(Vector3(EZUnitVector::kZUnitVector), angle)); }
		static INLINE OrthogonalTransform MakeTranslation(Vector3 translate) { return OrthogonalTransform(translate); }

		INLINE Vector3 operator* (Vector3 vec) const { return m_rotation * vec + m_translation; }
		INLINE Vector4 operator* (Vector4 vec) const 
		{
			return
				Vector4(SetWToZero(m_rotation * Vector3((DirectX::XMVECTOR)vec))) +
				Vector4(SetWToOne(m_translation)) * vec.GetW();
		}
		INLINE OrthogonalTransform operator* (const OrthogonalTransform& xform) const 
		{
			return OrthogonalTransform(m_rotation * xform.m_rotation, m_rotation * xform.m_translation + m_translation);
		}

		INLINE OrthogonalTransform operator~ () const 
		{
			Quaternion invertedRotation = ~m_rotation;
			return OrthogonalTransform(invertedRotation, invertedRotation * -m_translation);
		}

	private:
		Quaternion m_rotation;
		Vector3 m_translation;
	};

	// A AffineTransform is a 3x4 matrix with an implicit 4th row = [0,0,0,1]. This is used to perform a change of
	// basis on 3D points. An affine transformation does not have to have orthonormal basis vectors.
	__declspec(align(64)) class AffineTransform
	{
	public:
		INLINE AffineTransform() {}
		INLINE AffineTransform(Vector3 x, Vector3 y, Vector3 z, Vector3 w) : m_basis(x, y, z), m_translation(w) {}
		INLINE AffineTransform(Vector3 translate) : m_basis(EIdentityTag::kIdentity), m_translation(translate) {}
		INLINE AffineTransform(const Matrix3& mat, Vector3 translate = Vector3(EZeroTag::kZero)) : m_basis(mat), m_translation(translate) {}
		INLINE AffineTransform(Quaternion rot, Vector3 translate = Vector3(EZeroTag::kZero)) : m_basis(rot), m_translation(translate) {}
		INLINE AffineTransform(const OrthogonalTransform& xform) : m_basis(xform.GetRotation()), m_translation(xform.GetTranslation()) {}
		INLINE AffineTransform(EIdentityTag) : m_basis(EIdentityTag::kIdentity), m_translation(EZeroTag::kZero) {}
		INLINE explicit AffineTransform(const DirectX::XMMATRIX& mat) : m_basis(mat), m_translation(mat.r[3]) {}

		INLINE operator DirectX::XMMATRIX() const { return (DirectX::XMMATRIX&)*this; }

		INLINE void SetX(Vector3 x) { m_basis.SetX(x); }
		INLINE void SetY(Vector3 y) { m_basis.SetY(y); }
		INLINE void SetZ(Vector3 z) { m_basis.SetZ(z); }
		INLINE void SetTranslation(Vector3 w) { m_translation = w; }

		INLINE Vector3 GetX() const { return m_basis.GetX(); }
		INLINE Vector3 GetY() const { return m_basis.GetY(); }
		INLINE Vector3 GetZ() const { return m_basis.GetZ(); }
		INLINE Vector3 GetTranslation() const { return m_translation; }
		INLINE const Matrix3& GetBasis() const { return (const Matrix3&)*this; }

		static INLINE AffineTransform MakeXRotation(float angle) { return AffineTransform(Matrix3::MakeXRotation(angle)); }
		static INLINE AffineTransform MakeYRotation(float angle) { return AffineTransform(Matrix3::MakeYRotation(angle)); }
		static INLINE AffineTransform MakeZRotation(float angle) { return AffineTransform(Matrix3::MakeZRotation(angle)); }
		static INLINE AffineTransform MakeScale(float scale) { return AffineTransform(Matrix3::MakeScale(scale)); }
		static INLINE AffineTransform MakeScale(Vector3 scale) { return AffineTransform(Matrix3::MakeScale(scale)); }
		static INLINE AffineTransform MakeTranslation(Vector3 translate) { return AffineTransform(translate); }

		INLINE Vector3 operator* (Vector3 vec) const { return m_basis * vec + m_translation; }
		INLINE AffineTransform operator* (const AffineTransform& mat) const 
		{
			return AffineTransform(m_basis * mat.m_basis, *this * mat.GetTranslation());
		}

	private:
		Matrix3 m_basis;
		Vector3 m_translation;
	};

	// Inline methods.
	INLINE OrthogonalTransform Invert(const OrthogonalTransform& xform) { return ~xform; }
	// This specialized matrix invert assumes that the 3x3 matrix is orthogonal (and normalized).
	INLINE AffineTransform OrthoInvert(const AffineTransform& xform)
	{
		Matrix3 basis = Transpose(xform.GetBasis());
		return AffineTransform(basis, basis * -xform.GetTranslation());
	}
	// inline Transform Inverse( const Transform& mat ) { TBD }

}	// namespace Math
