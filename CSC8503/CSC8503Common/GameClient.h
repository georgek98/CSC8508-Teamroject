#pragma once
#include "NetworkBase.h"
#include <stdint.h>
//#include <thread>
//#include <atomic>

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class GameClient : public NetworkBase {
		public:
			GameClient();
			~GameClient();

			bool Connect(std::string ip, int portNum);
			bool Disconnect();

			void SendPacket(GamePacket& payload);

			void UpdateClient(float dt);

			//void ThreadedUpdate();

			int lastPacketID = 0;

		protected:
			ENetPeer* netPeer;
			/*std::atomic<bool> threadAlive;
			std::thread	updateThread;*/
		};
	}
}

