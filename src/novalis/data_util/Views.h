#pragma once

#include "BasicConcepts.h"

namespace nv {
	template<typename Container, std::integral... ExcludedIdxs> requires(sizeof...(ExcludedIdxs) > 0)
	class ExcludeIndices {
	private:
		Container& m_container;

		using ExcludedIndicesArray = std::array<size_t, sizeof...(ExcludedIdxs)>;
		ExcludedIndicesArray m_excludedIdxs;
	public:
		ExcludeIndices(Container& con, ExcludedIdxs... excludedIdxs)
			: m_container{ con }, m_excludedIdxs{ static_cast<size_t>(excludedIdxs)... }
		{
		}

		class Iterator {
		private:
			using ContainerIterator = typename Container::iterator;
			ContainerIterator m_it;
			size_t m_containerIdx;

			ExcludedIndicesArray& m_excludedIdxs;
			size_t m_excludedIdxArrIdx = 0;
		public:
			Iterator(ContainerIterator it, size_t index, ExcludedIndicesArray& excludedIdxs)
				: m_it{ it }, m_excludedIdxs{ excludedIdxs }, m_containerIdx{ index }
			{
			}

			decltype(auto) operator*(this auto&& self) {
				return *self.m_it;
			}

			auto& operator++() {
				m_it++;
				m_containerIdx++;
				while (m_excludedIdxArrIdx < sizeof...(ExcludedIdxs) && m_excludedIdxs[m_excludedIdxArrIdx] < m_containerIdx) {
					m_excludedIdxArrIdx++;
				}
				while (m_excludedIdxArrIdx < sizeof...(ExcludedIdxs) && m_excludedIdxs[m_excludedIdxArrIdx] == m_containerIdx) {
					m_it++;
					m_containerIdx++;
					m_excludedIdxArrIdx++;
				}
				return *this;
			}

			auto& operator++(int) {
				return this->operator++();
			}


			bool operator==(const Iterator& other) const {
				return m_it == other.m_it;
			}
			bool operator!=(const Iterator& other) const {
				return m_it != other.m_it;
			}
		};

		Iterator begin() {
			auto beginIt = ranges::begin(m_container);
			Iterator it{
				beginIt,
				0,
				m_excludedIdxs
			};
			if (0 == m_excludedIdxs[0]) {
				it++;
			}
			return it;
		}
		Iterator end() {
			return Iterator{
				ranges::end(m_container),
				ranges::size(m_container),
				m_excludedIdxs
			};
		}
	};

	template<typename Container, typename Comp>
	class UniqueElements {
	private:
		using It = typename Container::iterator;

		static consteval auto getComp() {
			if constexpr (Map<Container>) {
				return [](const It& it1, const It& it2) {
					return std::cmp_less(it1->first, it2->first);
				};
			} else {
				return [](const It& v1, const It& v2) {
					return std::cmp_less(*v1, *v2);
				};
			}
		}

		Container& m_container;
		decltype(getComp()) m_compLess;
	public:
		UniqueElements(Container& container) : m_container{ container }, m_compLess{ getComp() }
		{
		}

		class Iterator {
		private:
			It m_it;
		public:
			explicit Iterator(It it) : m_it{ it }
			{
			}
			
			Iterator operator++() noexcept {
				auto equalRange = ranges::equal_range(m_it, ranges::end(m_container), m_compLess);
				return Iterator{ equalRange.second };
			}
			Iterator operator++(int) noexcept {
				return this->operator++();
			}

			bool operator==(const Iterator& other) const {
				return other.m_it == m_it;
			}
			bool operator!=(const Iterator& other) const {
				return other.m_it != m_it;
			}
		};
	};
}