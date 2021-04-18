#pragma once

#include "Collectable.h"
#include "PlayerObject.h"
namespace NCL
{
	namespace CSC8503
	{
		class LongJump : public Collectable
		{
		public:
			LongJump() {
				Collectable::Collectable();
				name = "LongJump";
			}
			void OnCollisionBegin(GameObject* otherObject) override {
				Collectable::OnCollisionBegin(otherObject);
				if (otherObject->GetName() == "Player") {
					((PlayerObject*)otherObject)->LongJumpCollection();
					GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/long_jump.wav");
				}
			}
		};
	}
}
