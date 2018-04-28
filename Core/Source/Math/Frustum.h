//
// Frustum operations, usually to deal with culling related problems.
// Always looks into the positive z axis to construct the standard frustum. And remind we use right-handed system.
//

#pragma once

#include "BoundingPlane.h"
#include "BoundingSphere.h"

namespace Math {

enum class CornerID {
	kNearLowerLeft, kNearUpperLeft, kNearLowerRight, kNearUpperRight,
	kFarLowerLeft, kFarUpperLeft, kFarLowerRight, kFarUpperRight
};

enum class PlaneID {
	kNearPlane, kFarPlane, kLeftPlane, kRightPlane, kTopPlane, kBottomPlane
};

class Frustum {
public:
	Frustum() {}
	Frustum(const Matrix4& ProjectionMatrix);

	Vector3 GetFrustumCorner(CornerID id) const { return m_FrustumCorners[(int)id]; }
	BoundingPlane GetFrustumPlane(PlaneID id) const { return m_FrustumPlanes[(int)id]; }

	// Test whether the bounding sphere intersects the frustum. Intersection is defined as either being
	// fully contained in the frustum, or by intersecting one or more of the planes.
	bool IntersectSphere(BoundingSphere sphere) const;

	// We don't officially have a BoundingBox class yet, but let's assume it's forthcoming. (There is a
	// simple struct in the Model project.)
	bool IntersectBoundingBox(const Vector3 minBound, const Vector3 maxBound) const;

	friend Frustum  operator* (const OrthogonalTransform& xform, const Frustum& frustum);	// Fast
	friend Frustum  operator* (const AffineTransform& xform, const Frustum& frustum);		// Slow
	friend Frustum  operator* (const Matrix4& xform, const Frustum& frustum);				// Slowest (and most general)

private:
	// Perspective frustum constructor (for pyramid-shaped frusta). Note: HTan and VTan both are according to half angle.
	void ConstructPerspectiveFrustum(float HTan, float VTan, float NearClip, float FarClip);

	// Orthographic frustum constructor (for box-shaped frusta).
	void ConstructOrthographicFrustum(float Left, float Right, float Top, float Bottom, float NearClip, float FarClip);

	Vector3 m_FrustumCorners[8];		// the corners of the frustum
	BoundingPlane m_FrustumPlanes[6];	// the bounding planes
};

//
// Inline methods.
//

inline bool Frustum::IntersectSphere(BoundingSphere sphere) const {
	float radius = sphere.GetRadius();
	for (int i = 0; i < 6; ++i) {
		// Note: the normal of bounding planes of frustum point inside.
		if (m_FrustumPlanes[i].DistanceFromPoint(sphere.GetCenter()) + radius < 0.0f)
			return false;
	}
	return true;
}

inline bool Frustum::IntersectBoundingBox(const Vector3 minBound, const Vector3 maxBound) const {
	for (int i = 0; i < 6; ++i) {
		BoundingPlane p = m_FrustumPlanes[i];
		Vector3 farCorner = Select(minBound, maxBound, p.GetNormal() > Vector3(EZeroTag::kZero));
		if (p.DistanceFromPoint(farCorner) < 0.0f)
			return false;
	}

	return true;
}

inline Frustum operator* (const OrthogonalTransform& xform, const Frustum& frustum) {
	Frustum result;

	for (int i = 0; i < 8; ++i)
		result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

	for (int i = 0; i < 6; ++i)
		result.m_FrustumPlanes[i] = xform * frustum.m_FrustumPlanes[i];

	return result;
}

inline Frustum operator* (const AffineTransform& xform, const Frustum& frustum) {
	Frustum result;

	for (int i = 0; i < 8; ++i)
		result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

	Matrix4 XForm = Transpose(Invert(Matrix4(xform)));

	for (int i = 0; i < 6; ++i)
		result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

	return result;
}

inline Frustum operator* (const Matrix4& mtx, const Frustum& frustum) {
	Frustum result;

	for (int i = 0; i < 8; ++i)
		result.m_FrustumCorners[i] = Vector3(mtx * frustum.m_FrustumCorners[i]);

	Matrix4 XForm = Transpose(Invert(mtx));

	for (int i = 0; i < 6; ++i)
		result.m_FrustumPlanes[i] = BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

	return result;
}

}	// namespace Math