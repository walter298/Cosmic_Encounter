#pragma once

#include <algorithm>
#include <cassert>

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

	template<typename Object>
	void eraseMultipleIterators(std::vector<Object>& vec, std::vector<typename std::vector<Object>::iterator>& iterators) {
		assert(!vec.empty() && !iterators.empty());

		std::ptrdiff_t elemsPastDoomedIt = 1;
		auto doomedIndirectIt = iterators.begin();
		auto currEnd = *doomedIndirectIt;

		while (std::next(doomedIndirectIt) != iterators.end()) {
			auto moveRangeBegin = currEnd + elemsPastDoomedIt; //vec::iterator
			auto moveRangeEnd = *std::next(doomedIndirectIt); //vec::iterator

			if (ranges::distance(moveRangeBegin, moveRangeEnd) > 0) {
				ranges::move(moveRangeBegin, moveRangeEnd, currEnd);
				currEnd += ranges::distance(moveRangeBegin, moveRangeEnd);
			}
			doomedIndirectIt++;
			elemsPastDoomedIt++;
		}
		ranges::move(std::next(iterators.back()), vec.end(), currEnd);
		vec.erase(vec.begin() + (vec.size() - iterators.size()), vec.end());
		iterators.clear();
	}
}