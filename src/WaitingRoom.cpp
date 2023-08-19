#include "WaitingRoom.h"

WaitingRoom::WaitingRoom(nv::NovalisInstance& instance, asio::io_context& context, Client& client,
	const tcp::endpoint& serverEndpoint) :
	Scene{ nv::relativePath("scenes/homescreen.txt"), instance },
	m_context{ context },
	m_client{ client },
	m_serverEndpoint{ serverEndpoint }
{
	const auto objectPath = nv::workingDirectory() + "/objects";

	//initialize connection messages
	m_connectingMsg      = instance.getObj<nv::Text>("connecting_to_game");
	m_failedToConnectMsg = instance.getObj<nv::Text>("failed_to_connect");
	
	m_connectingMsg->setRenPos(950, 750);
	m_failedToConnectMsg->setRenPos(950, 550);

	//create start button texture
	m_startBtnPic = instance.getObj<nv::Text>("start_button");
	m_startBtnPic->setRenPos(950, 750);
	addObj(m_startBtnPic);

	m_startBtn = &m_startBtnPic->backgroundRect();
	m_startBtn.onHovered(
		[this] {
			m_startBtnPic->backgroundRect().setRenderColor(107, 250, 8, 255);
		}
	);
	m_startBtn.onUnhovered(
		[this] {
			m_startBtnPic->backgroundRect().setRenderColor(185, 196, 194, 255);
		}
	);
	m_startBtn.onLeftClick(
		[this] {
			connect();
		}
	);
	addButton(&m_startBtn);

	addEvent([this] { checkConnection(); });
}

void WaitingRoom::connect() {
	removeButton(m_startBtn.getID());
	m_renderer.removeObj(m_startBtnPic->getID());
	m_renderer.addObj(m_connectingMsg);

	m_context.post(
		[this] { 
			auto connState = m_client.joinGame(m_context, m_serverEndpoint); 
			//std::cout << "Hit the connect button and connected\n";
			std::scoped_lock lk{ m_connStateMutex };
			m_connState = std::move(connState);
		}
	);
}

void WaitingRoom::checkConnection() {
	std::scoped_lock lk{ m_connStateMutex };
	switch (m_connState) {
	case ConnectionState::FailedToConnect:
		m_connState = ConnectionState::NotConnecting;
		m_renderer.removeObj(m_connectingMsg->getID());
		m_renderer.addObj(m_failedToConnectMsg);
		m_renderer.addObj(m_startBtnPic);
		addButton(&m_startBtn);
		break;
	case ConnectionState::Connected:
		endScene(EndReason::NextScene);
		break;
	}
}

