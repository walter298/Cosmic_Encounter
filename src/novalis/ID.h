#pragma once

#include <string>

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

	template<typename T>
	struct ObjectBase {
	protected:
		ID<T> m_ID;
		std::string m_name;
	public:
		ID<T> getID() const noexcept {
			return m_ID;
		}
		const std::string& getName() const noexcept {
			return m_name;
		}
	};
}