#pragma once
#include "Collectable.h"
#include "PlayerObject.h"
namespace NCL
{
	namespace CSC8503
	{
		class Coin : public Collectable
		{
		public:
			Coin() {
				Collectable::Collectable(); 
				name = "Coin";
			}
			void OnCollisionBegin(GameObject* otherObject) override {
				Collectable::OnCollisionBegin(otherObject);
				if (otherObject->GetName() == "Player") {
					((PlayerObject*)otherObject)->CollectCoin();
					GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/GetCoin.wav");
				}
			}
		};
	}
}
