#pragma once
#include "GameObject.h"
#include "GameWorld.h"
namespace NCL {
	namespace CSC8503 {
		class FallingTile : public GameObject {

		public:

			FallingTile(string newName, float newTime = 3, float newResetY = 0, PxVec3 newOriginalPos = PxVec3(0,200,0)) {
				
				name = newName;
				originalTime = newTime;
				timeAlive = 0;
				resetY = newResetY;
				originalPos = newOriginalPos;
			}
			void ResetTile(){
				
				timeActive = 0;
				started = false;
				PxRigidDynamic* actor = (PxRigidDynamic*)physicsObject->GetPXActor();
				GetPhysicsObject()->GetPXActor()->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				actor->setLinearVelocity(PxVec3(0, 0, 0));
				actor->setAngularVelocity(PxVec3(0, 0, 0));		
				SetPosition(originalPos);
			}

			void Update(float dt) override {
				GameObject::Update(dt);

				if (started) {
					timeActive += dt;
					if (timeActive >= originalTime) {

						PxRigidDynamic* actor = (PxRigidDynamic*)physicsObject->GetPXActor();


						actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
						actor->setForceAndTorque(PxVec3(0, -100, 0), PxVec3(0,0,0), PxForceMode::eIMPULSE);
					}
				}
				
				if (this->GetTransform().GetPosition().y < resetY) {
					ResetTile();
				}
				
			}

			void OnCollisionBegin(GameObject* otherObject) override {

				if (otherObject->GetName()._Equal("Player")) {
					started = true;
				}

	
				
			}
			void OnCollisionEnd(GameObject* otherObject) override {

	

			}

		protected:
			bool started;
			float originalTime;
			float timeActive;
			float resetY;
			PxVec3 originalPos;
		};
	}
}


