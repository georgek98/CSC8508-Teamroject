#pragma once
#include "../CSC8503Common/PlayerObject.h"


namespace NCL
{
	namespace CSC8503
	{
		class NetworkedGame;

		class NetworkPlayer : public PlayerObject
		{
		public:
			NetworkPlayer(NetworkedGame* game, int num);
			~NetworkPlayer() {};

			void Update(float dt) override;

			void FixedUpdate(float dt)override;

			void OnCollisionBegin(GameObject* otherObject) override;

			int GetPlayerNum() const
			{
				return playerNum;
			}

			int GetScore() const
			{
				return score;
			}

			void SetScore(int val)
			{
				score = val;
			}

			string GetPlayerName() const
			{
				return playerName;
			}

			void SetPlayerName(string s)
			{
				playerName = s;
			}

			string GetDefaultPlayerName() const
			{
				return defaultPlayerName;
			}

			void Disconnect()
			{
				connected = false;
			}

			bool IsConnected() const
			{
				return connected;
			}

			void SetHost()
			{
				isHost = true;
				isOnHost = true;
			}

			void SetOnHost() {
				isOnHost = true;
			}

		protected:
			NetworkedGame* game;
			int playerNum;
			int score;
			string playerName;
			string defaultPlayerName;
			bool connected;
			bool isHost;
			bool isOnHost;
		};
	}
}

