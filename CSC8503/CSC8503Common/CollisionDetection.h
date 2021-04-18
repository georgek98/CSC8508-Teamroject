/*			  Created By Rich Davison
*			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Collision Detection Definition		 */
#pragma once

#include "../../Common/Camera.h"
#include "PhysxConversions.h"
#include "../../Common/Vector2.h"
#include "../../Common/Window.h"
#include "../../Common/Maths.h"
#include "Debug.h"
using NCL::Camera;
using namespace NCL::Maths;
using namespace NCL::CSC8503;
namespace NCL {
	class CollisionDetection
	{
	public:
		static PxVec3 GetMouseDirection(const Camera& c);
		static Vector3 Unproject(const Vector3& screenPos, const Camera& cam);
		static Matrix4 GenerateInverseView(const Camera& c);
		static Matrix4 GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane);
	private:
		CollisionDetection() {}
		~CollisionDetection() {}
	};
}
