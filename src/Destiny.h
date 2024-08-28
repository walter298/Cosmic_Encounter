#pragma once

#include "novalis/Scene.h"

#include "NetworkUtil.h"
#include "RenderData.h"

struct DestinyDrawInfo {
	Color drawnColor{};
	bool allowedToKeepDrawing = false;
	bool mustKeepDrawing = false;
};

enum DestinyResponseFromTurnTaker {
	AcceptedColor,
	DecidedToKeepDrawing
};

inline constexpr int ENDING_DESTINY_PHASE = 2;

class NonTurnTakingDestiny {
private:
	Socket& m_sock;
	nv::Scene m_scene;
	std::mutex m_mutex;
	std::vector<Color> m_drawnColors;
	bool m_wasFinalColorSent = false;

	asio::awaitable<void> asyncReadColorsFromServer();
public:
	NonTurnTakingDestiny(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap, const ColorMap& colorMap);

	void operator()();
};

class TurnTakingDestiny {
private:
	Socket& m_sock;
	nv::Scene m_scene;
	bool m_wasColorDrawn = false;
	Color m_currColor{};

	void readDrawnColor(nv::Rect& acceptButtonRect, nv::Text& acceptButtonText, nv::Rect& keepDrawingButtonRect, 
		nv::Text& keepDrawingButtonText, const ColorMap& colorMap);

	void toggleButton(nv::Rect& rect, nv::Text& text, bool showing);
public:
	TurnTakingDestiny(Socket& sock, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap, const ColorMap& colorMap);
	Color operator()();
};