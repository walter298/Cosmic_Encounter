#pragma once

#include <concepts>

namespace nv {
	template<std::signed_integral T, T scale>
	class Scaled {
	private:
		T m_value;
	public:
		operator T() {
			return m_value;
		}

		bool operator==(const T& other) const noexcept {
			return m_value < other.value;
		}

		Scaled(T other) noexcept 
			: m_value{ other * scale }
		{
		}
		Scaled(const Scaled& other) noexcept 
			: m_value{ other.value }
		{
		}

		void operator=(const T& other) noexcept {
			m_value = other.value * scale;
		}
		void operator=(Scaled other) noexcept {
			m_value = other.m_value; //other.value is already scaled
		}

		void operator++() noexcept {
			m_value++;
		}
		void operator--() noexcept {
			m_value--;
		}

		void operator+=(T t) noexcept {
			m_value += (t * scale);
		}
		void operator-=(T t) noexcept {
			m_value -= (t * scale);
		}
		void operator+=(Scaled scaled) noexcept {
			m_value += scaled.m_value;
		}
		void operator-=(Scaled scaled) noexcept {
			m_value -= scaled.m_value;
		}
	};

	template<std::signed_integral T>
	using ScaledX = Scaled<T, 2560 / NV_SCREEN_WIDTH>;

	template<std::signed_integral T>
	using ScaledY = Scaled<T, 1920 / NV_SCREEN_HEIGHT>;
}
