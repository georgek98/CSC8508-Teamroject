#pragma once

#include "Collectable.h"
#include "PlayerObject.h"
namespace NCL
{
	namespace CSC8503
	{
		class SpeedPower : public Collectable
		{
		public:
			SpeedPower() {
				Collectable::Collectable();
				name = "SpeedPower";
			}
			void OnCollisionBegin(GameObject* otherObject) override {
				Collectable::OnCollisionBegin(otherObject);
				if (otherObject->GetName() == "Player") {
					((PlayerObject*)otherObject)->SpeedPowerCollection();
					GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/speed_boost.wav");
				}
			}
		};
	}
}
