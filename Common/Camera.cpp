/*         Created By Rich Davison,
		Edited by Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Camera Implementation			 */
#include "Camera.h"
#include "Window.h"
#include "../CSC8503/GameTech/GameManager.h"
#include <algorithm>

using namespace NCL;

/* Polls the camera for keyboard / mouse movement.
Should be done once per frame! Pass it the msec since
last frame (default value is for simplicities sake...) */
void Camera::UpdateCamera(float dt)
{
	if (currentState == CameraState::FREE)
	{		// Only update camera if in free mode
			//Update the mouse by how much
		pitch -= (Window::GetMouse()->GetRelativePosition().y);
		yaw -= (Window::GetMouse()->GetRelativePosition().x);

		//Bounds check the pitch, to be between straight up and straight down ;)
		pitch = std::min(pitch, 90.0f);
		pitch = std::max(pitch, -90.0f);

		if (yaw < 0)
			yaw += 360.0f;
		if (yaw > 360.0f)
			yaw -= 360.0f;

		float frameSpeed = 100 * dt;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W))
			position += Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S))
			position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A))
			position += Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D))
			position -= Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE))
			position.y += frameSpeed;
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT))
			position.y -= frameSpeed;
	}
}

void Camera::RotateCameraWithObject(float dt, NCL::CSC8503::GameObject* o)
{
	UIState ui = GameManager::GetRenderer()->GetUIState();
	if (ui == UIState::INGAME || ui == UIState::FINISH || ui == UIState::SCOREBOARD)
	{
		pitch -= (Window::GetMouse()->GetRelativePosition().y) * rotateSpeed * dt;
		yaw -= (Window::GetMouse()->GetRelativePosition().x) * rotateSpeed * dt;

		pitch = std::min(pitch, 05.0f);
		pitch = std::max(pitch, -20.0f);

		if (yaw < 0)
			yaw += 360.0f;
		if (yaw > 360.0f)
			yaw -= 360.0f;

	}


	/* Make sure our cameras position stays within our camera sphere */
	float radius = 24;
	float scale = (radius * radius) / 90;
	float xPos = (scale * yaw) / radius;
	xPos = fmod(xPos, radius);

	if (yaw < 90)
	{
		lockedOffset.x = xPos;
		lockedOffset.z = radius - xPos;
	}
	else if (yaw < 180)
	{
		lockedOffset.x = radius - xPos;
		lockedOffset.z = -xPos;
	}
	else if (yaw < 270)
	{
		lockedOffset.x = -xPos;
		lockedOffset.z = -radius + xPos;
	}
	else
	{
		lockedOffset.x = -radius + xPos;
		lockedOffset.z = xPos;
	}
}
/* Lock the camera to an object */
void Camera::UpdateCameraWithObject(float dt, NCL::CSC8503::GameObject* o)
{
	/* Set our play camera */
	if (currentState == CameraState::TOPDOWN)
	{
		PxTransform pose = o->GetPhysicsObject()->GetPXActor()->getGlobalPose();
		position = Vector3(0, 100, 0) + pose.p;
		pitch = -90;
	}
	else if (currentState == CameraState::THIRDPERSON)
	{
		PxRigidDynamic* actor = (PxRigidDynamic*)o->GetPhysicsObject()->GetPXActor();
		PxTransform pose = actor->getGlobalPose();
		float mag = abs(actor->getLinearVelocity().magnitude());

		if (mag >= 50)
		{
			tween = tweeny::from(currentDist).to(56.f).during(40).onStep([this](float outDist)
				{
					this->setCurrentDist(outDist);
					return false;
				});
		}
		else if (mag < 50 && mag > 20)
		{
			tween = tweeny::from(currentDist).to(46.f).during(40).onStep([this](float outDist)
				{
					this->setCurrentDist(outDist);
					return false;
				});
		}
		else
		{
			tween = tweeny::from(currentDist).to(40.f).during(40).onStep([this](float outDist)
				{
					this->setCurrentDist(outDist);
					return false;
				});
		}
		if (tween.progress() < 1.0f)
		{
			tween.step(1);
		}
		SetPosition(Quaternion(pose.q) * Vector3(0, 25, currentDist) + pose.p);
	}
}


/* Generates a view matrix for the camera's viewpoint. This matrix can be sent
straight to the shader...it's already an 'inverse camera' matrix. */
Matrix4 Camera::BuildViewMatrix() const
{
	//Why do a complicated matrix inversion, when we can just generate the matrix
	//using the negative values ;). The matrix multiplication order is important!
	return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Translation(-position);
};

Matrix4 Camera::BuildProjectionMatrix(float currentAspect) const
{
	if (camType == CameraType::Orthographic)
	{
		return Matrix4::Orthographic(nearPlane, farPlane, right, left, top, bottom);
	}
	return Matrix4::Perspective(nearPlane, farPlane, currentAspect, fov);
}

Camera Camera::BuildPerspectiveCamera(const Vector3& pos, float pitch, float yaw, float fov, float nearPlane, float farPlane)
{
	Camera c;
	c.camType = CameraType::Perspective;
	c.position = pos;
	c.pitch = pitch;
	c.yaw = yaw;
	c.nearPlane = nearPlane;
	c.farPlane = farPlane;

	c.fov = fov;

	return c;
}

Camera Camera::BuildOrthoCamera(const Vector3& pos, float pitch, float yaw, float left, float right, float top, float bottom, float nearPlane, float farPlane)
{
	Camera c;
	c.camType = CameraType::Orthographic;
	c.position = pos;
	c.pitch = pitch;
	c.yaw = yaw;
	c.nearPlane = nearPlane;
	c.farPlane = farPlane;

	c.left = left;
	c.right = right;
	c.top = top;
	c.bottom = bottom;

	return c;
}
