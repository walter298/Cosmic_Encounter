#include "WaitingRoom.h"

WaitingRoom::WaitingRoom(nv::Instance& instance, asio::io_context& context, Client& client,
	const tcp::endpoint& serverEndpoint) :
	Scene{ nv::relativePath("scenes/homescreen.txt"), instance },
	m_context{ context },
	m_client{ client },
	m_serverEndpoint{ serverEndpoint },
	m_connectingMsg{ instance.getText("connecting_to_game") },
	m_failedToConnectMsg{ instance.getText("failed_to_connect") },
	m_startBtnPic{ instance.getText("start_button") },
	m_startBtn{ &m_startBtnPic.backgroundRect() }
{
	//set render positions of textures
	m_connectingMsg.setRenPos(950, 750);
	m_failedToConnectMsg.setRenPos(950, 550);
	m_startBtnPic.setRenPos(950, 750);

	render(&m_startBtnPic, 1);
	
	m_startBtn.onHovered([this] {
			m_startBtnPic.backgroundRect().setRenderColor(107, 250, 8, 255);
		}
	);
	m_startBtn.onUnhovered([this] {
			m_startBtnPic.backgroundRect().setRenderColor(185, 196, 194, 255);
		}
	);
	m_startBtn.onLeftClick([this] {
			connect();
		}
	);
	addButton(&m_startBtn);

	addEvent([this] { checkConnection(); });
}

void WaitingRoom::connect() {
	removeButton(m_startBtn.getID());
	stopRendering(m_startBtnPic.getID());
	removeButton(m_startBtn.getID());
	render(&m_connectingMsg, 1);

	m_context.post(
		[this] { 
			auto connState = m_client.joinGame(m_context, m_serverEndpoint); 
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
		stopRendering(m_connectingMsg.getID());
		render(&m_failedToConnectMsg, 1);
		render(&m_startBtnPic, 1);
		addButton(&m_startBtn);
		break;
	case ConnectionState::Connected:
		endScene(EndReason::NextScene);
		break;
	}
}
