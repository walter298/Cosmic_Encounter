#pragma once

#include <boost/container/flat_map.hpp>

#include <plf_hive.h>

#include "BasicConcepts.h"
#include "Reflection.h"

namespace nv {
	template<typename Object>
	class StableRef {
	private:
		using PlainObject = std::remove_cvref_t<Object>;

		using ObjectHive = plf::hive<PlainObject>;
		using ObjectHiveIt = typename ObjectHive::iterator;

		std::reference_wrapper<ObjectHive> m_hive;
		ObjectHiveIt m_it;
		bool m_erased = false;
	public:
		StableRef(plf::hive<Object>& hive, ObjectHiveIt it)
			: m_hive{ hive }, m_it{ it }
		{
		}

		auto operator->(this auto&& self) {
			assert(!self.m_erased);
			return &(*self.m_it);
		}

		decltype(auto) get(this auto&& self) {
			assert(!self.m_erased);
			return unrefwrap(*self.m_it);
		}

		void erase() {
			assert(!m_erased);
			m_hive.get().erase(m_it);
			m_erased = true;
		}
	};

	template<typename T>
	class AmbiguousOwnerPtr {
	private:
		T* m_obj = nullptr;
		bool m_owning = false;
	public:
		AmbiguousOwnerPtr(T* obj, bool owning) : m_obj{ obj }, m_owning{ owning }
		{
		}
		AmbiguousOwnerPtr(const AmbiguousOwnerPtr<T>&) = delete;

		template<typename U>
		AmbiguousOwnerPtr(AmbiguousOwnerPtr<U>&& other) noexcept requires(std::convertible_to<U*, T*>)
			: m_obj{ other.m_obj }, m_owning{ other.m_owning }
		{
			other.m_owning = false;
			other.m_obj = nullptr;
		};

		auto const get(this auto&& self) {
			return self.m_obj;
		}
		auto& operator*(this auto&& self) {
			assert(self.m_obj != nullptr);
			return *self.m_obj;
		}

		~AmbiguousOwnerPtr() noexcept {
			if (m_owning) {
				delete m_obj;
			}
		}
	};

	template<typename Func>
	struct ScopeExit {
	private:
		Func m_f;
	public:
		ScopeExit(const Func& f) noexcept(std::is_nothrow_copy_constructible_v<Func>) : m_f{ f } {}
		ScopeExit(Func&& f) noexcept(std::is_nothrow_move_constructible_v<Func>) : m_f{ std::move(f) } {}
		~ScopeExit() noexcept(std::is_nothrow_invocable_v<Func>) {
			m_f();
		}
	};

	template<typename... Ts>
	struct ObjectLayers {
		boost::container::flat_map<int, std::tuple<plf::hive<Ts>...>> layers;

		decltype(auto) operator[](this auto&& self, int n) {
			return self.layers[n];
		}
		decltype(auto) at(this auto&& self, int n) {
			return self.layers.at(n);
		}
		auto find(this auto&& self, int n) {
			return self.layers.find(n);
		}
		auto begin(this auto&& self) {
			return self.layers.begin();
		}
		auto end(this auto&& self) {
			return self.layers.end();
		}
		void clear() {
			layers.clear();
		}
		void reserve(size_t n) {
			layers.reserve(n);
		}
		template<typename Callable>
		void forEach(this auto&& self, Callable callable) {
			for (auto& [layer, objHives] : self.layers) {
				forEachDataMember([layer, &callable](auto& objHive) {
					for (auto& obj : objHive) {
						if (callable(layer, obj) == BREAK_FROM_LOOP) {
							return BREAK_FROM_LOOP;
						}
					}
					return STAY_IN_LOOP;
				}, objHives);
			}
		}

		template<typename Callable>
		void forEach(this auto&& self, Callable callable, int layer) {
			forEachDataMember([layer, &callable](auto& objHive) {
				for (auto& obj : objHive) {
					if (callable(layer, obj) == BREAK_FROM_LOOP) {
						return BREAK_FROM_LOOP;
					}
				}
				return STAY_IN_LOOP;
			}, self.layers.at(layer));
		}

		template<typename Callable>
		void forEachHive(this auto&& self, Callable callable) {
			for (auto& [layer, objHives] : self.layers) {
				forEachDataMember([layer, &callable](auto& objHive) {
					if (callable(layer, objHive) == BREAK_FROM_LOOP) {
						return BREAK_FROM_LOOP;
					}
				}, objHives);
			}
		}

		template<typename Callable>
		void forEachHive(this auto&& self, Callable callable, int layer) {
			forEachDataMember([layer, &callable](auto& objHive) {
				return callable(layer, objHive) == BREAK_FROM_LOOP;
			}, self.layers[layer]);
		}
	};
}