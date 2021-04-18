#pragma once
#include "LevelCreator.h"
#include "../CSC8503Common/NetworkObject.h"
#include "../CSC8503Common/NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;

		class NetworkedGame : public LevelCreator, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			void ResetWorld();

			void StartAsServer(LevelState state, string playerName);
			void StartAsClient(string playerName, string ip);

			void Update(float dt) override;

			void ReceivePacket(float dt, int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			std::vector<NetworkObject*> GetNetworkObjects() const {
				return networkObjects;
			}

			int GetLevelNetworkObjectsCount() const {
				return levelNetworkObjectsCount;
			}

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			void UpdatePlayer(NetworkPlayer* player, float dt);
			NetworkPlayer* SpawnPlayer(int playerNum);
			void AddLevelNetworkObject(GameObject* object);

			void InitWorld(LevelState state);
			void InitGameExamples(LevelState state);
			void InitGameObstacles(LevelState state);

			// Server-side
			std::map<int, int> stateIDs;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;


			// Client-side
			std::vector<NetworkObject*> networkObjects;

			// Server-side
			std::map<int, NetworkPlayer*> serverPlayers;

			NetworkPlayer* localPlayer = nullptr;
			string localPlayerName = "";

			int levelNetworkObjectsCount = 0;
			bool initialising = true;
		};
	}
}

