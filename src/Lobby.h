#pragma once

#include <SDL2/SDL_render.h>

#include "Novalis/Scene.h"

#include "NetworkUtil.h"

enum LobbyUpdate {
	StartingGame,
	PacifistJoining,
	VirusJoining,
	LaserJoining,
	OracleJoining,
	None
};

std::vector<nv::TextureObject> runLobby(Socket& sock, nv::TextureObject& alien, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap);