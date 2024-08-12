#pragma once

#include <algorithm>

#include "BasicConcepts.h"

namespace nv {
	template<ranges::viewable_range Range, typename Func>
	void forEachEqualRange(Range& range, Func f) {
		auto it = ranges::begin(range);
		while (it != ranges::end(range)) {
			auto equalRange = ranges::equal_range(it, ranges::end(range), *it);
			f(equalRange);
			it = ranges::end(equalRange);
		}
	}
	template<ranges::viewable_range Range, typename Func, typename SortPred>
	void forEachEqualRange(Range& range, Func f, SortPred pred) {
		auto it = ranges::begin(range);
		while (it != ranges::end(range)) {
			auto equalRange = ranges::equal_range(it, ranges::end(range), *it, pred);
			f(equalRange);
			it = ranges::end(equalRange);
		}
	}

	template<ranges::viewable_range Range>
	constexpr auto binaryFind(Range& range, typename Range::value_type& value) {
		return ranges::lower_bound(range, value);
	}

	template<ranges::viewable_range Range, typename Value, typename PM>
	constexpr auto binaryFind(Range& range, const Value& value, PM pm)
		requires((std::is_member_pointer_v<PM> || std::is_member_function_pointer_v<PM>) &&
				   SameAsDecayed<Value, decltype(std::invoke(pm, std::declval<typename Range::value_type>()))>)
	{
		return ranges::lower_bound(range, value, [](const Value& v1, const Value& v2) {
			return v1 < v2;
		}, [&](auto& elem) { return std::invoke(pm, elem); });
	}
}