#ifndef SOUND_H
#define SOUND_H

#include <string>
#include <exception>
#include <iostream>

#include <SDL2/SDL_mixer.h>

namespace nv {
	class Music {
		Mix_Chunk* m_music = nullptr;
		bool m_valid = false;
	public:
		explicit Music(std::string absPath) noexcept;
		Music(const Music&)  = delete;
		Music(Music&&)       = delete;

		~Music();

		void play(int loopCount) noexcept;

		static void stop() noexcept;

		inline bool valid() const noexcept {
			return m_valid;
		}
	};
}

#endif