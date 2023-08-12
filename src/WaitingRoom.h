#pragma once

#include <atomic>

#include "novalis/Scene.h"
#include "novalis/Sound.h"

#include "Player.h"

class WaitingRoom : public nv::Scene {
private:
	asio::io_context& m_context;
	Player& m_client;
	const tcp::endpoint& m_serverEndpoint;

	ConnectionState m_connState = ConnectionState::NotConnecting;
	std::mutex m_connStateMutex;

	void connect();
	void checkConnection();

	nv::Text* m_startBtnPic;
	nv::Text* m_connectingMsg;
	nv::Text* m_failedToConnectMsg;

	nv::Event m_startBtnEvt;
	nv::Button m_startBtn;
public:
	WaitingRoom(nv::NovalisInstance& instance, asio::io_context& context, Player& client, 
		const tcp::endpoint& serverEndpoint);
};