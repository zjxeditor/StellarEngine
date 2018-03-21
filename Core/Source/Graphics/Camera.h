//
// Virtual camera operations.
//

#pragma once

#include "Math/VectorMath.h"
#include "Math/Frustum.h"

namespace Graphics
{
	// Base camera class which contains geenral camera operations. Add be subclassed for specific kind of camera.
	class BaseCamera
	{
	public:
		// Call this function once per frame and after you've changed any state. This
		// regenerates all matrices. Calling it more or less than once per frame will break
		// temporal effects and cause unpredictable results.
		void Update();

		// Public functions for controlling where the camera is and its orientation
		void SetEyeAtUp(Math::Vector3 eye, Math::Vector3 at, Math::Vector3 up);
		void SetLookDirection(Math::Vector3 forward, Math::Vector3 up);
		void SetRotation(Math::Quaternion basisRotation);
		void SetPosition(Math::Vector3 worldPos);
		void SetTransform(const Math::AffineTransform& xform);
		void SetTransform(const Math::OrthogonalTransform& xform);

		const Math::Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }
		const Math::Vector3 GetRightVec() const { return m_Basis.GetX(); }
		const Math::Vector3 GetUpVec() const { return m_Basis.GetY(); }
		const Math::Vector3 GetForwardVec() const { return -m_Basis.GetZ(); }
		const Math::Vector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }

		// Accessors for reading the various matrices and frustums.
		const Math::Matrix4& GetViewMatrix() const { return m_ViewMatrix; }
		const Math::Matrix4& GetProjMatrix() const { return m_ProjMatrix; }
		const Math::Matrix4& GetViewProjMatrix() const { return m_ViewProjMatrix; }
		const Math::Matrix4& GetReprojectionMatrix() const { return m_ReprojectMatrix; }
		const Math::Frustum& GetViewSpaceFrustum() const { return m_FrustumVS; }
		const Math::Frustum& GetWorldSpaceFrustum() const { return m_FrustumWS; }

	protected:
		BaseCamera() : m_CameraToWorld(Math::EIdentityTag::kIdentity), m_Basis(Math::EIdentityTag::kIdentity) {}

		void SetProjMatrix(const Math::Matrix4& ProjMat) { m_ProjMatrix = ProjMat; }

		// Camera to world transformation. It is orthogonal.
		Math::OrthogonalTransform m_CameraToWorld;

		// Redundant data cached for faster lookups.
		Math::Matrix3 m_Basis;

		// Transforms homogeneous coordinates from world space to view space. In this case, view space is defined as +X is
		// to the right, +Y is up, and -Z is forward. This has to match what the projection matrix expects, but you might
		// also need to know what the convention is if you work in view space in a shader. Use right hand system.
		Math::Matrix4 m_ViewMatrix;

		// The projection matrix transforms view space to clip space. Once division by W has occurred, the final coordinates
		// can be transformed by the viewport matrix to screen space. The projection matrix is determined by the screen aspect 
		// and camera field of view. A projection matrix can also be orthographic. In that case, field of view would be defined
		// in linear units, not angles.
		Math::Matrix4 m_ProjMatrix;

		// A concatenation of the view and projection matrices.
		Math::Matrix4 m_ViewProjMatrix;

		// The view-projection matrix from the previous frame
		Math::Matrix4 m_PreviousViewProjMatrix;

		// Projects a clip-space coordinate to the previous frame (useful for temporal effects).
		Math::Matrix4 m_ReprojectMatrix;

		Math::Frustum m_FrustumVS;	// View-space view frustum
		Math::Frustum m_FrustumWS;	// World-space view frustum
	};

	// Inline methods.
	inline void BaseCamera::SetEyeAtUp(Math::Vector3 eye, Math::Vector3 at, Math::Vector3 up)
	{
		SetLookDirection(at - eye, up);
		SetPosition(eye);
	}
	inline void BaseCamera::SetRotation(Math::Quaternion basisRotation)
	{
		basisRotation = Normalize(basisRotation);
		m_CameraToWorld.SetRotation(basisRotation);
		m_Basis = Math::Matrix3(basisRotation);
	}
	inline void BaseCamera::SetPosition(Math::Vector3 worldPos)
	{
		m_CameraToWorld.SetTranslation(worldPos);
	}
	inline void BaseCamera::SetTransform(const Math::AffineTransform& xform)
	{
		// By using these functions, we rederive an orthogonal transform.
		SetLookDirection(-xform.GetZ(), xform.GetY());
		SetPosition(xform.GetTranslation());
	}
	inline void BaseCamera::SetTransform(const Math::OrthogonalTransform& xform)
	{
		// Directly set the rotation and translation.
		SetRotation(xform.GetRotation());
		SetPosition(xform.GetTranslation());
	}

	// Perspective camera.
	class Camera : public BaseCamera
	{
	public:
		Camera();

		// Controls the view-to-projection matrix
		void SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip);
		void SetFOV(float verticalFovInRadians) { m_VerticalFOV = verticalFovInRadians; UpdateProjMatrix(); }
		void SetAspectRatio(float heightOverWidth) { m_AspectRatio = heightOverWidth; UpdateProjMatrix(); }
		void SetZRange(float nearZ, float farZ) { m_NearClip = nearZ; m_FarClip = farZ; UpdateProjMatrix(); }
		void ReverseZ(bool enable) { m_ReverseZ = enable; UpdateProjMatrix(); }

		float GetFOV() const { return m_VerticalFOV; }
		float GetNearClip() const { return m_NearClip; }
		float GetFarClip() const { return m_FarClip; }
		float GetClearDepth() const { return m_ReverseZ ? 0.0f : 1.0f; }

	private:
		void UpdateProjMatrix();

		float m_VerticalFOV;	// Field of view angle in radians
		float m_AspectRatio;	// height / weight.
		float m_NearClip;
		float m_FarClip;
		bool m_ReverseZ;		// Invert near and far clip distances so that Z=0 is the far plane
	};

	// Inline methods.
	inline Camera::Camera() : m_ReverseZ(true)
	{
		SetPerspectiveMatrix(DirectX::XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f);
	}
	inline void Camera::SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
	{
		m_VerticalFOV = verticalFovRadians;
		m_AspectRatio = aspectHeightOverWidth;
		m_NearClip = nearZClip;
		m_FarClip = farZClip;

		UpdateProjMatrix();

		m_PreviousViewProjMatrix = m_ViewProjMatrix;
	}

}	// namespace Graphics