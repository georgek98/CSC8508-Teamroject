#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket
{
	short playerID;
	short messageID;

	MessagePacket()
	{
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame()
{
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket = 0.0f;
	packetsToSnapshot = 0;
	isSinglePlayer = true;

	GameManager::GetRenderer()->SetNetworkedGame(this);
}

NetworkedGame::~NetworkedGame()
{
	delete thisServer;
	delete thisClient;
	NetworkBase::Destroy();
}

void NetworkedGame::ResetWorld()
{
	networkObjects.clear();
	serverPlayers.clear();
	stateIDs.clear();

	delete thisServer;
	thisServer = nullptr;

	if (thisClient) {
		thisClient->Disconnect();
	}

	timeToNextPacket = 0.0f;
	packetsToSnapshot = 0;
	localPlayer = nullptr;
	localPlayerName = "";
	levelNetworkObjectsCount = 0;
	initialising = true;

	LevelCreator::ResetWorld();
}

void NetworkedGame::StartAsServer(LevelState state, string playerName)
{
	isSinglePlayer = false;
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4, state);

	thisServer->RegisterPacketHandler(Received_State, this);

	//std::cout << "Starting as server." << std::endl;

	InitWorld(state);
	localPlayer = SpawnPlayer(-1);
	localPlayer->SetPlayerName(playerName);
	localPlayer->SetHost();

	GameManager::SetPlayer(localPlayer);
	GameManager::SetSelectionObject(localPlayer);
	GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::THIRDPERSON);
}

void NetworkedGame::StartAsClient(string playerName, string ip)
{
	bool clientExists = true;

	if (!thisClient)
	{
		thisClient = new GameClient();
		clientExists = false;
	}

	thisClient->Connect(ip, NetworkBase::GetDefaultPort());

	if (!clientExists)
	{
		thisClient->RegisterPacketHandler(Delta_State, this);
		thisClient->RegisterPacketHandler(Full_State, this);
		thisClient->RegisterPacketHandler(Player_Connected, this);
		thisClient->RegisterPacketHandler(Player_Disconnected, this);
		thisClient->RegisterPacketHandler(Shutdown, this);
	}

	//std::cout << "Starting as client." << std::endl;

	localPlayerName = playerName;
}

void NetworkedGame::Update(float dt)
{
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0)
	{
		if (thisServer)
		{
			UpdateAsServer(dt);
		}
		else if (thisClient)
		{
			UpdateAsClient(dt);
		}
		timeToNextPacket += 1.0f / 120.0f; //120hz server/client update
	}

	UpdateCamera(dt);
	GameManager::GetWorld()->UpdateWorld(dt);
	UpdateLevel(dt);

	if (thisServer)
	{
		UpdateTimeStep(dt);
	}

	GameManager::GetAudioManager()->UpdateAudio(GameManager::GetWorld()->GetMainCamera()->GetPosition());
	GameManager::GetRenderer()->Update(dt);
	GameManager::GetRenderer()->Render();
	Debug::FlushRenderables(dt);
}

void NetworkedGame::UpdateAsServer(float dt)
{
	thisServer->UpdateServer(dt);

	if (serverPlayers.size() < thisServer->players.size())
	{
		for (auto i : thisServer->players)
		{
			std::map<int, NetworkPlayer*>::iterator it;
			it = serverPlayers.find(i.first);

			if (it == serverPlayers.end())
			{
				NetworkPlayer* player = SpawnPlayer(i.first);
				player->SetOnHost();
				serverPlayers.insert(std::pair<int, NetworkPlayer*>(i.first, player));
				stateIDs.insert(std::pair<int, int>(i.first, 0));
			}
		}
	}
	else if (serverPlayers.size() > thisServer->players.size())
	{
		for (auto i = serverPlayers.begin(); i != serverPlayers.end(); )
		{
			std::map<int, ENetPeer*>::iterator it;
			it = thisServer->players.find((*i).first);

			if (it == thisServer->players.end())
			{
				(*i).second->Disconnect();
				stateIDs.erase((*i).first);
				i = serverPlayers.erase(i);
			}
			else
			{
				i++;
			}
		}
	}

	if (stateIDs.size() > 0)
	{
		UpdateMinimumState();
	}

	packetsToSnapshot--;
	if (packetsToSnapshot < 0)
	{
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else
	{
		BroadcastSnapshot(true);
	}
}

void NetworkedGame::UpdateAsClient(float dt)
{
	ClientPacket newPacket;
	thisClient->UpdateClient(dt);

	Camera* c = GameManager::GetWorld()->GetMainCamera();
	newPacket.cameraYaw = c->GetYaw();

	if (localPlayer)
	{
		newPacket.fwdAxis = Quaternion(localPlayer->GetTransform().GetOrientation()) * Vector3(0, 0, 1);
		newPacket.playerName = localPlayer->GetPlayerName();

		if (localPlayer->HasFinished())
		{
			newPacket.finishTime = localPlayer->GetFinishTime();
		}

		UIState ui = GameManager::GetRenderer()->GetUIState();

		if (ui == UIState::INGAME || ui == UIState::SCOREBOARD)
		{
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W))
				newPacket.buttonstates[0] = 1;
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A))
				newPacket.buttonstates[1] = 1;
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S))
				newPacket.buttonstates[2] = 1;
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D))
				newPacket.buttonstates[3] = 1;
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE))
				newPacket.buttonstates[4] = 1;
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT))
				newPacket.buttonstates[5] = 1;
		}
	}

	newPacket.lastID = thisClient->lastPacketID;
	thisClient->SendPacket(newPacket);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame)
{
	for (auto p : serverPlayers)
	{
		for (auto o : networkObjects)
		{
			int playerState = stateIDs.at(p.first);
			GamePacket* newPacket = nullptr;
			if (o->WritePacket(&newPacket, deltaFrame, playerState))
			{
				thisServer->SendPacketToPeer(p.first, *newPacket);
				delete newPacket;
			}
		}
	}
}

void NetworkedGame::UpdateMinimumState()
{
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs)
	{
		minID = min(minID, i.second);
		maxID = max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	GameManager::GetWorld()->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o)
		{
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}

	//std::cout << "Server min state: " << minID << std::endl;
}

void NetworkedGame::UpdatePlayer(NetworkPlayer* player, float dt)
{
	if (player->GetRaycastTimer() <= 0.0f)
	{
		PxVec3 pos = PhysxConversions::GetVector3(player->GetTransform().GetPosition() + PxVec3(0, 3, 0));
		PxVec3 dir = PxVec3(0, -1, 0);
		float distance = 4.0f;
		const PxU32 bufferSize = 4;        // [in] size of 'hitBuffer'
		PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
		PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here

		GameManager::GetPhysicsSystem()->GetGScene()->raycast(pos, dir, distance, buf);
		player->SetIsGrounded(buf.getNbTouches() > 1);

		player->SetRaycastTimer(0.1f);
	}
}

NetworkPlayer* NetworkedGame::SpawnPlayer(int playerNum)
{
	PxVec3 pos;
	LevelState state = GameManager::GetLevelState();

	switch (state) {
	case LevelState::LEVEL3:
		pos = PxVec3(0, -100, 100);
		break;
	case LevelState::LEVEL2:
		pos = PxVec3(0, 10, 0);
		break;
	case LevelState::LEVEL1:
		pos = PxVec3(0, 180, 150);
		break;
	}

	pos.x = 10 * playerNum;
	NetworkPlayer* p = GameManager::AddPxNetworkPlayerToWorld(PxTransform(pos), 1, this, playerNum);
	NetworkObject* n = new NetworkObject(*p, levelNetworkObjectsCount + playerNum + 1);
	networkObjects.emplace_back(n);

	return p;
}

void NetworkedGame::AddLevelNetworkObject(GameObject* object)
{
	networkObjects.emplace_back(new NetworkObject(*object, networkObjects.size()));
	levelNetworkObjectsCount++;
}

void NetworkedGame::InitWorld(LevelState state)
{
	InitCamera();
	InitFloors(state);
	InitGameMusic(state);
	InitGameExamples(state);
	InitGameObstacles(state);
}

/* Initialises all game objects, enemies etc */
void NetworkedGame::InitGameExamples(LevelState state)
{
	switch (state)
	{
	case LevelState::LEVEL3:
		//power ups
		//floor 1
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(-20, -92, -75)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(-40, -92, -75)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(20, -92, -75)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(40, -92, -75)), 5));

		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -75)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -35)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -95)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -115)), 5));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -135)), 5));

		//floor 2
		for (int z = 0; z < 4; ++z)
		{
			for (int x = 0; x < 5 - (z % 2); ++x)
			{
				AddLevelNetworkObject(GameManager::AddPxFallingTileToWorld(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 300, 30 - (z * 80))), PxVec3(20, 1, 20)));
				if (x % 2 == 0) {
					AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 308, 30 - (z * 80))), 5));
				}
				else {
					AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 308, 30 - (z * 80))), 5));

				}
			}
		}

		//floor 3
		for (int i = 0; i < 4; ++i) {
			AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, 708, -5 - (i * 70))), 5));
			AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(160, 708, -5 - (i * 70))), 5));
			AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(-160, 708, -5 - (i * 70))), 5));
		}
		break;
	case LevelState::LEVEL2:
		//power ups
		for (int i = 0; i < 4; ++i) {
			AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(-55 + (i * 40), 5, -50) * 2), 3));
		}

		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(-60, -85, -470) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, -85, -480) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(50, -85, -475) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-40, -85, -550) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(10, -85, -540) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(60, -85, -520) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(-65, -85, -610) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-20, -85, -620) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(20, -85, -640) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(70, -85, -600) * 2), 3));

		for (int i = 0; i < 3; ++i) {
			AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(-70 + (i * 70), -85, -800) * 2), 5));
		}
		break;
	case LevelState::LEVEL1:
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 89, 30) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(30, 99, 0) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-30, 99, 0) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(60, 129, -85) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-60, 129, -85) * 2), 3));

		//AFTER TRAMPOLINES
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, 154, -124.5) * 2), 3));

		//Powerup before jumping part with blenders
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(36, 154, -280) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(-36, 154, -280) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 154, -270) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 154, -260) * 2), 3));

		//Slope  coins

		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(22, 134, -545) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(4, 125, -575) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-15, 110, -625) * 2), 3));

		//Toilet section powerUps/coins
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(-30, 95, -720) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(-15, 95, -720) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 95, -720) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxLongJump(PxTransform(PxVec3(15, 95, -720) * 2), 3));
		AddLevelNetworkObject(GameManager::AddPxSpeedPower(PxTransform(PxVec3(30, 95, -720) * 2), 3));

		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(30, 95, -800) * 2), 3));

		AddLevelNetworkObject(GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-30, 95, -850) * 2), 3));
		break;
	}
}

void NetworkedGame::InitGameObstacles(LevelState state)
{
	PxVec3 translate = PxVec3(0, -59, -14) * 2;
	PxVec3 translate2 = PxVec3(0, 50, 0) * 2;
	PxQuat q;

	switch (state)
	{
	case LevelState::LEVEL3:
		//Killplane
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, -130, 0)), PxVec3(1000, 1, 1000), PxVec3(0, -50, 50), PxVec3(90, 1, 20), false);

		//Floor 1
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(0, -100, -75)), PxVec3(20, 1, 80), PxVec3(0, 2, 0), 0.5, 0.5, false));



		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-120, -100, -75)), PxVec3(20, 1, 60), PxVec3(-0.25, 0, 0), 0.5, 0.5, false));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-120, -100, -75)), PxVec3(20, 60, 1), PxVec3(-0.25, 0, 0), 0.5, 0.5, false));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(120, -100, -75)), PxVec3(20, 1, 60), PxVec3(-0.25, 0, 0), 0.5, 0.5, false));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(120, -100, -75)), PxVec3(20, 60, 1), PxVec3(-0.25, 0, 0), 0.5, 0.5, false));

		//Floor 1-2 connection
		GameManager::AddBounceSticks(PxTransform(PxVec3(-160, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-80, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(0, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(80, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(160, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(40, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-40, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 4);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-160, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-80, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(0, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(80, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(160, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);


		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(40, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-40, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-160, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-80, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(0, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(80, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(160, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(40, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-40, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		//Floor 2
		for (int z = 0; z < 4; ++z)
		{
			for (int x = 0; x < 5 - (z % 2); ++x)
			{
				AddLevelNetworkObject(GameManager::AddPxFallingTileToWorld(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 300, 30 - (z * 80))), PxVec3(20, 1, 20)));
			}
		}

		//Floor 2-3 connection
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-180, 700, 145)), PxVec3(50, 0, 0), 15, 15, PxVec3(50, 0, 0));
		for (int i = 0; i < 30; i++)
		{
			Cannonball* c = GameManager::AddPxCannonBallToWorld(PxTransform(PxVec3(-200, 700, 145) * 2), 10, new PxVec3(25, 0, 0), 37);
			AddLevelNetworkObject(c);
			GameManager::GetObstacles()->cannons.push_back(c);
		}

		//floor 3 (pendulums)
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, 30)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -40)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -110)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -180)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -250)), 10, 30, 5, true, 0.5f, 2));




		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -5)), 10, 30, 5, false, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -75)), 10, 30, 5, false, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -145)), 10, 30, 5, false, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -215)), 10, 30, 5, false, 0.5f, 2));

		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -5)), 10, 30, 5, false, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -75)), 10, 30, 5, false, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -145)), 10, 30, 5, false, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -215)), 10, 30, 5, false, 0.5f, 2));

		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, 30)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -40)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -110)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -180)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -250)), 10, 30, 5, true, 0.5f, 2));

		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, 30)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -40)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -110)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -180)), 10, 30, 5, true, 0.5f, 2));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -250)), 10, 30, 5, true, 0.5f, 2));
		break;
	case LevelState::LEVEL2:
		//HAVE COMMENTED OUT THE ORIGINAL BEAMS, WILL LEAVE IN IN CASE WE DECIDE TO GO FOR STATIC ONES
		//WorldCreator::AddPxFloorToWorld(PxTransform(PxVec3(-70, -98, -900)), PxVec3(20, 20, 200));
		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(-70, -98, -900) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))),
			20, 100, PxVec3(0, 0, 1)));


		//WorldCreator::AddPxFloorToWorld(PxTransform(PxVec3(0, -98, -900      )*2), PxVec3(20, 20, 200));
		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(0, -98, -900) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))),
			20, 100, PxVec3(0, 0, 1)));
		//WorldCreator::AddPxFloorToWorld(PxTransform(PxVec3(70, -98, -900     )*2), PxVec3(20, 20, 200));
		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(70, -98, -900) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))),
			20, 100, PxVec3(0, 0, 1)));

		//cannons																
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-150, -70, -850) * 2), PxVec3(700, -50, 0), 10, 10, PxVec3(35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-150, -70, -900) * 2), PxVec3(700, -50, 0), 10, 10, PxVec3(35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-150, -70, -950) * 2), PxVec3(700, -50, 0), 10, 10, PxVec3(35, 0, 0));

		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -825) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -875) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -925) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -975) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));

		//pegs as obstacles/hiding places for the bowling balls
		//row 1
		q = PhysxConversions::GetQuaternion(Quaternion::EulerAnglesToQuaternion(11, 45, 11));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-60, -38, -1180) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(0, -38, -1180) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(60, -38, -1180) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));

		//row 2
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-30, -23, -1235) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(30, -23, -1235) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));

		//row 3
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-60, -3, -1290) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(0, -3, -1290) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));
		AddLevelNetworkObject(GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(60, -3, -1290) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0)));



		//OBSTACLE 5 - THE BLENDER
		//basically, it's an enclosed space with a spinning arm at the bottom to randomise which player actually wins
		//it should be flush with the entrance to the podium room so that the door is reasonably difficult to access unless there's nobody else there
		//again, not sure how to create the arm, it's a moving object, might need another class for this
		//also, it's over a 100m drop to the blender floor, so pls don't put fall damage in blender blade
		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform((PxVec3(0, -90, -1720) * 2) + translate + translate2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))), 20, 80, PxVec3(0, 2, 0)));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-80, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-40, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(0, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(40, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(80, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));

		for (int i = 0; i < 30; i++)
		{
			Cannonball* c = GameManager::AddPxCannonBallToWorld(PxTransform(PxVec3(50000, 5000, 5000) * 2), 20);
			AddLevelNetworkObject(c);
			GameManager::GetObstacles()->cannons.push_back(c);
		}
		break;

	case LevelState::LEVEL1:
		//OBSTACLE 1
//Rotating pillars
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 177, -180) * 2), 10, 25, 200));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 177, -200) * 2), 10, 25, 200, false));
		AddLevelNetworkObject(GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 177, -220) * 2), 10, 25, 200));

		//OBSTACLE2 
		//Jumping platforms with blenders
		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(36.5, 152.5, -315) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0)));

		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(-36.5, 152.5, -315) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0)));

		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(0, 152.5, -360) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 24, PxVec3(0, 1, 0)));

		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(36.5, 152.5, -410) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0)));

		AddLevelNetworkObject(GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(-36.5, 152.5, -410) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0)));

		//OBSTACLE 3
		//bouncing sticks on the slide 
		for (int i = 0; i <= 7; i++)
		{
			GameManager::AddBounceSticks(PxTransform(PxVec3(-35 + (i * 10), 140, -522) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
			GameManager::AddBounceSticks(PxTransform(PxVec3(-35 + (i * 10), 116, -602) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
		}

		for (int i = 0; i <= 8; i++)
		{
			GameManager::AddBounceSticks(PxTransform(PxVec3(-40 + (i * 10), 128, -562) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
			GameManager::AddBounceSticks(PxTransform(PxVec3(-40 + (i * 10), 104, -642) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
		}

		//OBSTACLE 4
		//Running through walls
		//cubes		
		/* 1st round */
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F));

		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F));

		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F));

		/* Second Round */
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F));

		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F));


		/* Third Round */
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 93, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 95, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 97, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 99, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 101, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 93, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 95, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 97, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 99, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		AddLevelNetworkObject(GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 101, -875) * 2), PxVec3(2, 2, 2), 1.0F));
		break;
	}
}

void NetworkedGame::ReceivePacket(float dt, int type, GamePacket* payload, int source)
{
	//std::cout << "SOME SORT OF PACKET RECEIVED" << std::endl;
	if (type == Received_State)
	{	//Server version of the game receives these from players
	//std::cout << "Server: Received packet from client " << source << "." << std::endl;
		ClientPacket* realPacket = (ClientPacket*)payload;

		std::map<int, int>::iterator it;
		it = stateIDs.find(source);

		if (it != stateIDs.end())
		{
			it->second = realPacket->lastID;
		}

		if (serverPlayers.size() >= (source + 1))
		{
			NetworkPlayer* player = serverPlayers.at(source);

			if (realPacket->playerName != "")
			{
				player->SetPlayerName(realPacket->playerName);
			}

			if (realPacket->finishTime > 0)
			{
				player->SetFinishTime(realPacket->finishTime);
				player->SetFinished(true);
			}

			UpdatePlayer(player, dt);

			if (player->GetPhysicsObject()->GetPXActor()->is<PxRigidDynamic>())
			{
				PxVec3 fwd = PhysxConversions::GetVector3(realPacket->fwdAxis);
				PxVec3 right = PhysxConversions::GetVector3(Vector3::Cross(Vector3(0, 1, 0), -fwd));

				float speed = player->IsGrounded() ? 5000.0f : 2500.0f;	// air damping
				float maxSpeed = realPacket->buttonstates[5] == 1 ? (player->GetPowerUpState() == PowerUpState::SPEEDPOWER ? 160.0f : 80.0f) : (player->GetPowerUpState() == PowerUpState::SPEEDPOWER ? 100.0f : 50.0f);

				PxRigidDynamic* body = (PxRigidDynamic*)serverPlayers.at(source)->GetPhysicsObject()->GetPXActor();

				if (realPacket->buttonstates[0] == 1)
					body->addForce(fwd * speed, PxForceMode::eIMPULSE);
				if (realPacket->buttonstates[1] == 1)
					body->addForce(-right * speed, PxForceMode::eIMPULSE);
				if (realPacket->buttonstates[2] == 1)
					body->addForce(-fwd * speed, PxForceMode::eIMPULSE);
				if (realPacket->buttonstates[3] == 1)
					body->addForce(right * speed, PxForceMode::eIMPULSE);

				PxVec3 playerVel = body->getLinearVelocity();
				if (realPacket->buttonstates[4] == 1 && player->IsGrounded())
					playerVel.y = sqrt(player->GetJumpHeight() * -2 * NCL::CSC8503::GRAVITTY);

				float linearSpeed = PxVec3(playerVel.x, 0, playerVel.z).magnitude();
				float excessSpeed = std::clamp(linearSpeed - maxSpeed, 0.0f, 0.1f);
				if (excessSpeed)
				{
					body->addForce(-playerVel * excessSpeed, PxForceMode::eVELOCITY_CHANGE);
				}
				body->setLinearVelocity(playerVel);
				body->setAngularVelocity(PxVec3(0));
				body->setGlobalPose(PxTransform(body->getGlobalPose().p, PxQuat(Maths::DegreesToRadians(realPacket->cameraYaw), { 0, 1, 0 })));
			}
		}
	}
	//CLIENT version of the game will receive these from the servers
	else if (type == Delta_State)
	{
		//std::cout << "Client: Received DeltaState packet from server." << std::endl;

		DeltaPacket* realPacket = (DeltaPacket*)payload;
		if (realPacket->objectID < (int)networkObjects.size())
		{
			networkObjects[realPacket->objectID]->ReadPacket(*realPacket);
		}
	}
	else if (type == Full_State)
	{
		//std::cout << "Client: Received FullState packet from server." << std::endl;

		FullPacket* realPacket = (FullPacket*)payload;
		if (realPacket->objectID < (int)networkObjects.size())
		{
			networkObjects[realPacket->objectID]->ReadPacket(*realPacket);

			thisClient->lastPacketID = realPacket->fullState.stateID;
		}
	}
	else if (type == Message)
	{
		MessagePacket* realPacket = (MessagePacket*)payload;

		if (realPacket->messageID == COLLISION_MSG)
		{
			//std::cout << "Client: Received collision message!" << std::endl;
		}
	}
	else if (type == Player_Connected)
	{
		NewPlayerPacket* realPacket = (NewPlayerPacket*)payload;
		//std::cout << "Client: New player connected! ID: " << realPacket->playerID << std::endl;

		if (initialising)
		{
			LevelState level = (LevelState)realPacket->serverLevel;
			GameManager::SetLevelState(level);
			InitWorld(level);

			for (int i = -1; i < realPacket->playerID; i++)
			{
				SpawnPlayer(i);
			}

			localPlayer = SpawnPlayer(realPacket->playerID);

			if (localPlayerName != "")
			{
				localPlayer->SetPlayerName(localPlayerName);
			}

			GameManager::SetPlayer(localPlayer);
			GameManager::SetSelectionObject(localPlayer);
			GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::THIRDPERSON);

			initialising = false;
		}
		else
		{
			SpawnPlayer(realPacket->playerID);
		}
	}
	else if (type == Player_Disconnected)
	{
		PlayerDisconnectPacket* realPacket = (PlayerDisconnectPacket*)payload;
		//std::cout << "Client: Player Disconnected!" << std::endl;
		GameObject* g = networkObjects[levelNetworkObjectsCount + realPacket->playerID + 1]->GetGameObject();

		if (dynamic_cast<NetworkPlayer*>(g))
		{
			NetworkPlayer* player = (NetworkPlayer*)g;
			player->Disconnect();
		}
	}
	else if (type == Shutdown)
	{
		GameManager::GetRenderer()->SetUIState(UIState::JOINLEVEL);
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b)
{
	if (thisServer)
	{ //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;

		int playerNum = a->GetPlayerNum();

		if (playerNum > -1)
		{
			newPacket.playerID = b->GetPlayerNum();
			thisServer->SendPacketToPeer(a->GetPlayerNum(), newPacket);
		}

		playerNum = b->GetPlayerNum();

		if (playerNum > -1)
		{
			newPacket.playerID = a->GetPlayerNum();
			thisServer->SendPacketToPeer(b->GetPlayerNum(), newPacket);
		}
	}
}