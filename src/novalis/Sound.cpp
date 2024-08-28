#include "Sound.h"

nv::Music::Music(std::string absPath) noexcept
{
	m_music = Mix_LoadWAV(absPath.c_str());
	
	if (m_music == nullptr) {
		m_valid = false;
	} else {
		m_valid = true;
	}
}

nv::Music::~Music()
{
	stop();
	Mix_FreeChunk(m_music);
}

void nv::Music::play(int loopCount) noexcept {
	constexpr int NEXT_AVAILABLE_CHANNEL = -1;
	if (m_valid) {
		Mix_PlayChannel(NEXT_AVAILABLE_CHANNEL, m_music, loopCount);
	}
}

void nv::Music::stop() noexcept {
	Mix_HaltMusic();
}