#include "NetworkPlayer.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num)
{
	this->game = game;
	playerNum = num;
	defaultPlayerName = "Player " + std::to_string(num + 2);
	playerName = defaultPlayerName;
	connected = true;
	isHost = false;
	isOnHost = false;
}

void NetworkPlayer::Update(float dt)
{
	if (isOnHost)
	{
		PlayerObject::Update(dt);		
	}
	else
	{
		Vector3 q = Quaternion(physicsObject->GetPXActor()->getGlobalPose().q).ToEuler() + Vector3(0, 180, 0);
		transform.SetOrientation(PhysxConversions::GetQuaternion(Quaternion::EulerAnglesToQuaternion(q.x, q.y, q.z)));
		transform.SetPosition(physicsObject->GetPXActor()->getGlobalPose().p);
		transform.UpdateMatrix();
		timeAlive += dt;
	}

}

void NetworkPlayer::FixedUpdate(float dt)
{
	UIState ui = GameManager::GetRenderer()->GetUIState();

	if (isHost && (ui == UIState::INGAME || ui == UIState::SCOREBOARD))
	{
		PlayerObject::FixedUpdate(dt);
	}
}

void NetworkPlayer::OnCollisionBegin(GameObject* otherObject)
{
	isColliding = true;

	if (dynamic_cast<NetworkPlayer*>(otherObject))
	{
		game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
	}
}