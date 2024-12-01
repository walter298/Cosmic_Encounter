#pragma once

#include <concepts>
#include <string>

#include <boost/functional/hash.hpp>

namespace nv {
	template<typename Object>
	class ID {
	private:
		int m_ID = 0;
	public:
		ID() noexcept {
			thread_local int IDCount = 0;
			m_ID = IDCount;
			IDCount++;
		}

		operator int() const noexcept {
			return m_ID;
		}
	};

	class Sprite;
	class Texture;

	namespace detail {
		class WorldCoordinates {
		private:
			static inline int x = 0;
			static inline int y = 0;
		public:
			friend class Sprite;
			friend class Texture;
		};

		template<typename T>
		struct ObjectBase {
			ID<T> m_ID;
			std::string name;
		};
	}
}

namespace boost {
	template<typename T>
	struct hash<nv::ID<T>> {
		size_t operator()(nv::ID<T> id) const {
			return boost::hash<int>()(id.operator int());
		}
	};
}