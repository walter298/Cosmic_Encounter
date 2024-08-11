#pragma once

namespace nv {
	class ID {
	private:
		int m_ID = 0;
	public:
		ID() noexcept;
		operator int() const noexcept;
	};
 
	template<typename T>
	struct SharedIDObject : public T {
	private:
		ID m_ID;
	public:
		ID getID() const noexcept {
			return m_ID;
		}
	};
}