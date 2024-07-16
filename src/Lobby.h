#pragma once

#include "novalis/Text.h"
#include "novalis/Texture.h"

#include "NetworkUtil.h"

enum LobbyUpdate {
	StartingGame,
	PacifistJoining,
	VirusJoining,
	LaserJoining,
	OracleJoining,
	None
};

std::vector<nv::TextureObject> runLobby(Socket& sock, size_t pCount, nv::TextureObject& alien, SDL_Renderer* renderer, nv::TextureMap& texMap, nv::FontMap& fontMap);