/*         Created By Rich Davison,
		Edited by Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Camera Definition			 */
#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "../CSC8503/CSC8503Common/GameObject.h"

#include <tweeny/tween.h>
#include <tweeny/tweeny.h>

namespace NCL
{
	using namespace NCL::Maths;
	enum class CameraType
	{
		Orthographic,
		Perspective
	};
	enum class CameraState { FREE, TOPDOWN, THIRDPERSON };
	class Camera
	{
	public:
		Camera(void)
		{
			left = 0;
			right = 0;
			top = 0;
			bottom = 0;

			pitch = 0.0f;
			yaw = 0.0f;

			fov = 45.0f;
			nearPlane = 1.0f;
			farPlane = 100.0f;
			lockedOffset = Vector3(0, 0, 0);
			camType = CameraType::Perspective;
			currentState = CameraState::FREE;
			currentDist = 50;
			rotateSpeed = 50;
		};


		Camera(float pitch, float yaw, const Vector3& position) : Camera()
		{
			this->pitch = pitch;
			this->yaw = yaw;
			this->position = position;

			this->fov = 45.0f;
			this->nearPlane = 1.0f;
			this->farPlane = 100.0f;

			this->camType = CameraType::Perspective;
		}

		~Camera(void) {};

		void UpdateCamera(float dt);
		void RotateCameraWithObject(float dt, NCL::CSC8503::GameObject* o);
		void UpdateCameraWithObject(float dt, NCL::CSC8503::GameObject* o);

		float GetFieldOfVision() const
		{
			return fov;
		}

		float GetNearPlane() const
		{
			return nearPlane;
		}

		float GetFarPlane() const
		{
			return farPlane;
		}

		void SetNearPlane(float val)
		{
			nearPlane = val;
		}

		void SetFarPlane(float val)
		{
			farPlane = val;
		}
		void SetState(CameraState val)
		{
			currentState = val;
		}
		CameraState GetState() const
		{
			return currentState;
		}
		//Builds a view matrix for the current camera variables, suitable for sending straight
		//to a vertex shader (i.e it's already an 'inverse camera matrix').
		Matrix4 BuildViewMatrix() const;

		Matrix4 BuildProjectionMatrix(float currentAspect = 1.0f) const;

		//Gets position in world space
		Vector3 GetPosition() const { return position; }
		//Sets position in world space
		void	SetPosition(const Vector3& val) { position = val; }

		//Gets yaw, in degrees
		float	GetYaw()   const { return yaw; }
		//Sets yaw, in degrees
		void	SetYaw(float y) { yaw = y; }

		//Gets pitch, in degrees
		float	GetPitch() const { return pitch; }
		//Sets pitch, in degrees
		void	SetPitch(float p) { pitch = p; }

		static Camera BuildPerspectiveCamera(const Vector3& pos, float pitch, float yaw, float fov, float near, float far);
		static Camera BuildOrthoCamera(const Vector3& pos, float pitch, float yaw, float left, float right, float top, float bottom, float near, float far);

		void setCurrentDist(float d) { currentDist = d; }


	protected:
		CameraType camType;
		float rotateSpeed;
		float currentDist;
		float	nearPlane;
		float	farPlane;
		float	left;
		float	right;
		float	top;
		float	bottom;

		float	fov;
		float	yaw;
		float	pitch;
		Vector3 position;
		Vector3 lockedOffset;

		bool updatingclose = false;
		CameraState currentState;


		tweeny::tween<float> tween;
	};
}