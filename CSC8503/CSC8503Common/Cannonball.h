#pragma once
#include "GameObject.h"
#include "GameWorld.h"

namespace NCL
{
	namespace CSC8503
	{
		class Cannonball :
			public GameObject
		{
		public:
			Cannonball(int newTime) {
				name = "Cannonball";
				isActive = false;
				timeLeft = newTime;
				refTimeLeft = newTime;
				//Disable();
			}

			bool IsActive() const {
				return isActive;
			}

			void Update(float dt) override;
			void ResetBall(const PxVec3& newPos, const PxVec3& force);

		protected:
			float refTimeLeft;
			float timeLeft;
			void Disable();
			PxVec3 initialPos;
			bool isActive;
		};
	}
}
