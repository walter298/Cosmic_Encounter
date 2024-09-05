#pragma once

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
}