#pragma once
#include "NetworkBase.h"
#include "NetworkState.h"
#include<vector>
#include<string>

namespace NCL
{
	namespace CSC8503
	{
		struct FullPacket : public GamePacket
		{
			int		objectID = -1;
			NetworkState fullState;
			int score = -1;
			std::string playerName = "";
			float finishTime = 0.0f;
			Vector3 playerVel;
			int powerUpState = 0;
			int coins = 0;

			FullPacket()
			{
				type = Full_State;
				size = sizeof(FullPacket) - sizeof(GamePacket);
			}
		};

		struct DeltaPacket : public GamePacket
		{
			int		fullID = -1;
			int		objectID = -1;
			char	pos[3];
			char	orientation[4];

			DeltaPacket()
			{
				type = Delta_State;
				size = sizeof(DeltaPacket) - sizeof(GamePacket);
			}
		};

		struct ClientPacket : public GamePacket
		{
			int		lastID = -1;
			char	buttonstates[6];
			Vector3 fwdAxis;
			int cameraYaw;
			std::string playerName;
			float finishTime = 0.0f;

			ClientPacket()
			{
				type = Received_State;
				size = sizeof(ClientPacket);
			}
		};

		class NetworkObject
		{
		public:
			NetworkObject(GameObject& o, int id);
			virtual ~NetworkObject();

			//Called by clients
			virtual bool ReadPacket(GamePacket& p);
			//Called by servers
			virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

			void UpdateStateHistory(int minID);

			GameObject* GetGameObject() const
			{
				return &object;
			}

		protected:
			NetworkState& GetLatestNetworkState();
			bool GetNetworkState(int frameID, NetworkState& state);

			virtual bool ReadDeltaPacket(DeltaPacket& p);
			virtual bool ReadFullPacket(FullPacket& p);

			virtual bool WriteDeltaPacket(GamePacket** p, int stateID);
			virtual bool WriteFullPacket(GamePacket** p);

			GameObject& object;

			NetworkState lastFullState;

			std::vector<NetworkState> stateHistory;

			int deltaErrors;
			int fullErrors;

			int networkID;
		};
	}
}

