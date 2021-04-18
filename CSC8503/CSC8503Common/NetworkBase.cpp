#include "NetworkBase.h"
#include <enet/enet.h>
#include <iostream>

NetworkBase::NetworkBase() {
	netHandle = nullptr;
}

NetworkBase::~NetworkBase() {
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

bool NetworkBase::ProcessPacket(float dt, GamePacket* packet, int peerID) {
	PacketHandlerIterator firstHandler;
	PacketHandlerIterator lastHandler;

	bool canHandle = GetPacketHandlers(packet->type, firstHandler, lastHandler);

	if (canHandle) {
		if (firstHandler == lastHandler) {
			lastHandler++;
		}

		for (auto i = firstHandler; i != lastHandler; ++i) {
			i->second->ReceivePacket(dt, packet->type, packet, peerID);
		}
		return true;
	}
	//std::cout << __FUNCTION__ << " no handler for packet type " << packet->type << std::endl;
	return false;
}