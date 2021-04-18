#pragma once
#include "GameObject.h"
namespace NCL
{
	namespace CSC8503
	{
		class Collectable : public GameObject
		{
		public:
			Collectable()
			{
				name = "Collectable";
				spawnTimer = 10.0f;
				initPos = transform.GetPosition();
			}
			virtual void OnCollisionBegin(GameObject* otherObject)
			{
				if (otherObject->GetName() == "Player")
				{
					SetPosition(PxVec3(5000, 5000, 5000));
					spawnTimer = 10.f;
				}
			}

			void Update(float dt) override
			{

				GameObject::Update(dt);

				Quaternion q = transform.GetOrientation();
				Vector3 eu = q.ToEuler() + Vector3(0, 100.f, 0) * dt;
				SetOrientation(PhysxConversions::GetQuaternion(Quaternion::EulerAnglesToQuaternion(eu.x, eu.y, eu.z)));
				((PxRigidDynamic*)physicsObject->GetPXActor())->setLinearVelocity(PxVec3(0, 0, 0));
				((PxRigidDynamic*)physicsObject->GetPXActor())->setAngularVelocity(PxVec3(0, 0, 0));

				if (transform.GetPosition().x != initPos.x)
				{
					spawnTimer -= dt;

					if (spawnTimer <= 0.f)
					{
						SetPosition(initPos);
						spawnTimer = 3.f;
					}
				}

			}

			void SetInitialPos(PxVec3 p)
			{
				initPos = p;
			}

		protected:
			PxVec3 initPos;
			float spawnTimer;
		};
	}
}


