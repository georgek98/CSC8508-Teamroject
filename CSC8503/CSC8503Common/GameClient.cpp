#include "GameClient.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;

GameClient::GameClient() {
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient() {
	//threadAlive = false;
	//updateThread.join();
	enet_host_destroy(netHandle);
}

bool GameClient::Connect(std::string ip, int portNum) {
	ENetAddress address;
	address.port = portNum;

	//address.host = (d << 24) | (c << 16) | (b << 8) | (a);

	enet_address_set_host(&address, ip.c_str());

	netPeer = enet_host_connect(netHandle, &address, 2, 0);

	/*if (netPeer != nullptr) {
		threadAlive = true;
		updateThread = std::thread(&GameClient::ThreadedUpdate, this);
	}*/

	return netPeer != nullptr;
}

bool GameClient::Disconnect() {
	if (netPeer != nullptr) {
		enet_peer_disconnect_now(netPeer, 0);

		return true;
	}
	else {
		return false;
	}
}

void GameClient::UpdateClient(float dt) {
	if (netHandle == nullptr)
	{
		return;
	}
	//Handle all incoming packets & send any packets awaiting dispatch
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			//std::cout << "Client: Connected to server!" << std::endl;
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			//std::cout << "Client: Packet received..." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(dt, packet);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameClient::SendPacket(GamePacket& payload) {
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);

	int test = enet_peer_send(netPeer, 0, dataPacket);
}

//void GameClient::ThreadedUpdate() {
//	while (threadAlive) {
//		UpdateClient(0);
//	}
//}
