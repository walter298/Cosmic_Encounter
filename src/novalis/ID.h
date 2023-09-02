#pragma once

#include <type_traits>

namespace nv {
	template<typename T>
	class ID {
	private:
		static int IDCount;
		int m_ID = 0;

		void setUniqueID() {
			IDCount++;
			m_ID = IDCount;
		}
	public:
		ID() noexcept {
			setUniqueID();
		}
		ID(const ID& other) noexcept {
			setUniqueID();
		}
		ID& operator=(const ID& other) noexcept {
			setUniqueID();
			return *this;
		}
		ID(ID&& other) noexcept {
			m_ID = std::move(other.m_ID);
		}
		ID& operator=(ID&& other) noexcept {
			m_ID = std::move(other.m_ID);
			return *this;
		}
		bool operator==(const ID& other) const noexcept {
			return (m_ID == other.m_ID);
		}
		inline int raw() const noexcept {
			return m_ID;
		}
	};

	template<typename T>
	int ID<T>::IDCount = 0;

	template<typename Container, typename T>
	void removeIfHasID(Container& con, const ID<T>& ID) noexcept {
		std::erase_if(con, [&ID](const auto& obj) { return std::invoke(&T::getID, obj) == ID; });
	}
}