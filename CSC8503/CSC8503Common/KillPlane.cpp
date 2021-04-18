#include "KillPlane.h"
#include "../GameTech/GameManager.h"

using namespace NCL;
using namespace CSC8503;
using namespace physx;

void KillPlane::Music() {
	GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/player_death.wav");
}