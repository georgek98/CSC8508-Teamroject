#pragma once
#include"../Common/Quaternion.h"
#include"../Common/Vector3.h"
#include "../../include/PxPhysicsAPI.h"

using namespace NCL::Maths;

 class PhysxConversions
{
public:
	static PxVec3 GetVector3(Vector3 v) 
	{
		return PxVec3(v.x, v.y, v.z);
	}

	static PxQuat GetQuaternion(Quaternion q)
	{
		return PxQuat(q.x, q.y, q.z, q.w);
	}
};