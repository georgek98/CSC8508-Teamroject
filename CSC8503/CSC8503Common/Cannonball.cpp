#include "Cannonball.h"
#include "../GameTech/GameManager.h"

using namespace NCL;
using namespace CSC8503;
using namespace physx;

void Cannonball::Disable()
{
	isActive = false;
	SetPosition(PxVec3(50000.0f, -50000.0f, 50000.0f));
	//timeLeft = 10.f;
}
void Cannonball::ResetBall(const PxVec3& newPos, const PxVec3& force)
{
	isActive = true;
	timeLeft = refTimeLeft;
	PxRigidDynamic* actor = (PxRigidDynamic*)physicsObject->GetPXActor();
	actor->setLinearVelocity(PxVec3(0, 0, 0));
	actor->setAngularVelocity(PxVec3(0, 0, 0));
	actor->setGlobalPose(PxTransform(newPos));
	actor->addForce(force, PxForceMode::eVELOCITY_CHANGE);
	GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/CannonFire02.wav", actor->getGlobalPose(), false);
}

void Cannonball::Update(float dt)
{
	GameObject::Update(dt);
	if (!isActive) return;
	timeLeft -= dt;
	if (timeLeft <= 0.f)
	{
		Disable();
	}
}
