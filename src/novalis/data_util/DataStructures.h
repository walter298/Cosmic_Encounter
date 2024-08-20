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
}