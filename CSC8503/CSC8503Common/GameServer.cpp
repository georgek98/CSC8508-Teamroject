#include "GameServer.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients, LevelState level)
{
	port = onPort;
	clientMax = maxClients;
	clientCount = 0;
	netHandle = nullptr;
	//threadAlive = false;
	this->level = level;

	Initialise();
}

GameServer::~GameServer()
{
	Shutdown();
}

void GameServer::Shutdown()
{
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	UpdateServer(0);

	//threadAlive = false;
	//updateThread.join();

	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise()
{
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	netHandle = enet_host_create(&address, clientMax, 1, 0, 0);

	if (!netHandle)
	{
		//std::cout << __FUNCTION__ << " failed to create network handle!" << std::endl;
		return false;
	}
	//threadAlive		= true;
	//updateThread	= std::thread(&GameServer::ThreadedUpdate, this);

	return true;
}

bool GameServer::SendGlobalPacket(int msgID)
{
	GamePacket packet;
	packet.type = msgID;

	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet)
{
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

void GameServer::UpdateServer(float dt)
{
	if (!netHandle)
	{
		return;
	}

	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		int type = event.type;
		ENetPeer* p = event.peer;

		int peer = p->incomingPeerID;

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT)
		{
			//std::cout << "Server: New client connected" << std::endl;
			NewPlayerPacket player(peer, (int)level);
			SendGlobalPacket(player);
			players.insert(std::pair<int, ENetPeer*>(peer, p));
			clientCount++;
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT)
		{
			//std::cout << "Server: A client has disconnected" << std::endl;
			PlayerDisconnectPacket player(peer);
			SendGlobalPacket(player);

			std::map<int, ENetPeer*>::iterator it;
			it = players.find(peer);

			if (it != players.end())
			{
				players.erase(it);
			}

			clientCount--;
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE)
		{
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(dt, packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}

bool GameServer::SendPacketToPeer(int peerID, GamePacket& packet)
{
	std::map<int, ENetPeer*>::iterator it;
	it = players.find(peerID);

	if (it != players.end())
	{
		ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
		enet_peer_send(it->second, 0, dataPacket);

		return true;
	}

	return false;
}

//void GameServer::ThreadedUpdate() {
//	while (threadAlive) {
//		UpdateServer(0);
//	}
//}