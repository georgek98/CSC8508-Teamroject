#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class KillPlane :
			public GameObject {


		public:
			KillPlane(PxVec3 newRespawnCentre, Vector3 newRespawnSizeRange) {
				respawnCentre = newRespawnCentre;
				newRespawnSizeRange.x = newRespawnSizeRange.x == 0 ? 1 : newRespawnSizeRange.x;
				newRespawnSizeRange.y = newRespawnSizeRange.y == 0 ? 1 : newRespawnSizeRange.y;
				newRespawnSizeRange.z = newRespawnSizeRange.z == 0 ? 1 : newRespawnSizeRange.z;

				respawnSizeRange = newRespawnSizeRange;
				name = "KillPlane";
			}


			//at the moment, I'm respawning randomly within range
			//obviously this won't be called for other objects which the plane just kills, rather than calling respawn for
			PxVec3 getRespawnPoint() {
				int xSize = respawnSizeRange.x;
				float x = (respawnCentre.x - (respawnSizeRange.x / 2)) + (rand() % xSize);
				int ySize = respawnSizeRange.y;
				float y = (respawnCentre.y - (respawnSizeRange.y / 2)) + (rand() % ySize);
				int zSize = respawnSizeRange.z;
				float z = (respawnCentre.z - (respawnSizeRange.z / 2)) + (rand() % zSize);
				return PxVec3(x, y, z);
			}

			void OnCollisionBegin(GameObject* otherObject) {
				if (otherObject->GetName() == "Player") {
					Music();
					PxTransform t = PxTransform(getRespawnPoint());
					otherObject->GetPhysicsObject()->GetPXActor()->setGlobalPose(t.transform(PxTransform(t.p)));
					((PxRigidBody*)otherObject->GetPhysicsObject()->GetPXActor())->setLinearVelocity(PxVec3(0, 0, 0));
				}

			}
			void Music();

		protected:
			PxVec3 respawnCentre;
			Vector3 respawnSizeRange;
		};
	}
}
