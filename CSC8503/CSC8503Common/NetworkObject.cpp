#include "NetworkObject.h"
#include "PhysXConversions.h"
#include "GameObject.h"
#include "../GameTech/NetworkPlayer.h"

using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o), networkID(id) {
	deltaErrors = 0;
	fullErrors = 0;
	o.SetNetworkObject(this);
}

NetworkObject::~NetworkObject() {
}

bool NetworkObject::ReadPacket(GamePacket& p) {
	if (p.type == Delta_State) {
		return ReadDeltaPacket((DeltaPacket&)p);
	}
	if (p.type == Full_State) {
		return ReadFullPacket((FullPacket&)p);
	}
	return false; //this isn't a packet we care about!
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
		else {
			return true;
		}
	}
	return WriteFullPacket(p);
}
//Client objects receive these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket& p) {
	if (p.fullID != lastFullState.stateID) {
		deltaErrors++; //can't delta this frame
		return false;
	}

	UpdateStateHistory(p.fullID);

	Vector3		fullPos = lastFullState.position;
	Quaternion  fullOrientation = lastFullState.orientation;

	fullPos.x += p.pos[0];
	fullPos.y += p.pos[1];
	fullPos.z += p.pos[2];

	fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
	fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
	fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
	fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

	object.GetPhysicsObject()->GetPXActor()->setGlobalPose(PxTransform(PhysxConversions::GetVector3(fullPos), PhysxConversions::GetQuaternion(fullOrientation)));

	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket& p) {
	if (p.fullState.stateID < lastFullState.stateID) {
		return false; // received an 'old' packet, ignore!
	}
	lastFullState = p.fullState;

	object.GetPhysicsObject()->GetPXActor()->setGlobalPose(PxTransform(PhysxConversions::GetVector3(lastFullState.position), PhysxConversions::GetQuaternion(lastFullState.orientation)));

	if (dynamic_cast<NetworkPlayer*>(&object)) {
		NetworkPlayer& player = (NetworkPlayer&)object;

		if (p.finishTime > 0)
		{
			player.SetFinishTime(p.finishTime);
			player.SetFinished(true);
		}

		if (p.playerName != player.GetDefaultPlayerName()) {
			player.SetPlayerName(p.playerName);
		}

		player.SetPowerUpState((PowerUpState)p.powerUpState);
		player.SetCoinsCollected(p.coins);

		((PxRigidDynamic*)player.GetPhysicsObject()->GetPXActor())->setLinearVelocity(PhysxConversions::GetVector3(p.playerVel));
	}

	stateHistory.emplace_back(lastFullState);

	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket** p, int stateID) {
	DeltaPacket* dp = new DeltaPacket();

	dp->objectID = networkID;

	NetworkState state;
	if (!GetNetworkState(stateID, state)) {
		return false; //can't delta!
	}

	dp->fullID = stateID;

	PxTransform pose = object.GetPhysicsObject()->GetPXActor()->getGlobalPose();
	Vector3	currentPos = pose.p;
	Quaternion currentOrientation = pose.q;

	currentPos -= state.position;
	currentOrientation -= state.orientation;

	dp->pos[0] = (char)currentPos.x;
	dp->pos[1] = (char)currentPos.y;
	dp->pos[2] = (char)currentPos.z;

	dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
	dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
	dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
	dp->orientation[3] = (char)(currentOrientation.w * 127.0f);

	*p = dp;
	return true;
}

bool NetworkObject::WriteFullPacket(GamePacket** p) {
	FullPacket* fp = new FullPacket();

	fp->objectID = networkID;
	PxTransform pose = object.GetPhysicsObject()->GetPXActor()->getGlobalPose();
	fp->fullState.position = pose.p;
	fp->fullState.orientation = pose.q;
	fp->fullState.stateID = lastFullState.stateID + 1;

	if (dynamic_cast<NetworkPlayer*>(&object)) {
		NetworkPlayer& player = (NetworkPlayer&)object;
		fp->score = player.GetScore();
		fp->playerName = player.GetPlayerName();

		if (player.HasFinished()) {
			fp->finishTime = player.GetFinishTime();
		}

		fp->powerUpState = (int)player.GetPowerUpState();
		fp->coins = player.GetCoinsCollected();

		fp->playerVel = ((PxRigidDynamic*)player.GetPhysicsObject()->GetPXActor())->getLinearVelocity();
	}

	*p = fp;
	return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() {
	return lastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ++i) {
		if ((*i).stateID == stateID) {
			state = (*i);
			return true;
		}
	}
	return false;
}

void NetworkObject::UpdateStateHistory(int minID) {
	for (auto i = stateHistory.begin(); i < stateHistory.end(); ) {
		if ((*i).stateID < minID) {
			i = stateHistory.erase(i);
		}
		else {
			++i;
		}
	}
}