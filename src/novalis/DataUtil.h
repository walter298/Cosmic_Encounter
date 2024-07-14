#ifndef DATA_UTIL_H
#define DATA_UTIL_H

#include <algorithm> //equal_range
#include <chrono>
#include <deque>
#include <filesystem>
#include <ranges> //viewable_range
#include <string>
#include <type_traits>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/optional.hpp>
#include <boost/pfr.hpp>

#include <plf_hive.h>

#include <SDL2/SDL.h>

#include <nlohmann/json.hpp>

void to_json(nlohmann::json& j, const SDL_Color& c);
void from_json(const nlohmann::json& j, SDL_Color& c);

void to_json(nlohmann::json& j, const SDL_Rect& c);
void from_json(const nlohmann::json& j, SDL_Rect& c);

void to_json(nlohmann::json& j, const SDL_Point& p);
void from_json(const nlohmann::json& j, SDL_Point& p);

//json (de)serialization for chrono types
namespace std {
	namespace chrono {
		template<typename Rep, typename Period>
		void from_json(const nlohmann::json& j, duration<Rep, Period>& time) {
			time = duration<Rep, Period>{ j.at("time").get<Rep>() };
		}
		template<typename Rep, typename Period>
		void to_json(nlohmann::json& j, const duration<Rep, Period>& time) {
			j["time"] = time.count();
		}
	}
}

namespace nv {
	namespace chrono = std::chrono;
	namespace ranges = std::ranges;
	namespace views = std::views;
	namespace pfr = boost::pfr;
	namespace boost_con = boost::container;

	//class aliases
	using nlohmann::json;

	//literals
	using namespace std::literals;

	template<typename T>
	concept Aggregate = std::is_aggregate_v<T>;

	namespace detail {
		template<Aggregate Aggr, size_t... Idxs>
		void assignEachAggrMember(const json& j, Aggr& aggr, std::index_sequence<Idxs...> idxs) {
			using ParsedAggr = std::tuple<pfr::tuple_element_t<Idxs, Aggr>...>;
			auto parsedTuple = j.get<ParsedAggr>();
			((pfr::get<Idxs>(aggr) = std::move(std::get<Idxs>(parsedTuple))), ...);
		}
	}

	template<Aggregate Aggr>
	void from_json(const json& j, Aggr& aggr) {
		detail::assignEachAggrMember(j, aggr, std::make_index_sequence<pfr::tuple_size_v<Aggr>>());
	}

	namespace detail {
		template<Aggregate Aggr, size_t... Idxs>
		auto feedJsonAggregate(json& j, const Aggr& aggr, std::index_sequence<Idxs...> idxs) {
			j = std::tuple{ pfr::get<Idxs>(aggr)... };
		}
	}

	template<Aggregate Aggr>
	void to_json(json& j, const Aggr& aggr) {
		detail::feedJsonAggregate(j, aggr, std::make_index_sequence<pfr::tuple_size_v<Aggr>>());
	}

	const std::string& workingDirectory();

	inline std::string objectPath(std::string relativePath) {
		return workingDirectory() + std::string("static_objects/") + relativePath;
	}

	inline std::string imagePath(std::string relativePath) {
		return workingDirectory() + std::string("images/") + relativePath;
	}

	//returns the path relative to the working directory
	inline std::string relativePath(std::string_view relativePath) {
		return workingDirectory() + relativePath.data();
	}

	std::optional<std::string> fileExtension(const std::string& fileName);
	std::string_view fileName(std::string_view filePath);

	template<typename T, typename U>
	using FlatOrderedMap = boost::container::flat_map<T, U>;
}

//boost::container::flat_map json (de)serialization
namespace boost {
	namespace container {
		template<typename Key, typename Value>
		void to_json(nlohmann::json& j, const flat_map<Key, Value>& bmap) {
			std::vector<std::pair<Key, Value>> vec;
			for (const auto& [key, value] : bmap) {
				vec.emplace_back(key, value);
			}
			j = std::move(vec);
		}
		template<typename Key, typename Value>
		void from_json(const nlohmann::json& j, flat_map<Key, Value>& bmap) {
			auto keyValuePairs = j.get<std::vector<std::pair<Key, Value>>>();
			for (auto& [key, value] : keyValuePairs) {
				bmap.emplace(std::move(key), std::move(value));
			}
		}
	}
}

namespace nv {
	//convenience routine for plf::hive
	template<typename T>
	decltype(auto) getBack(T& container) {
		return *(std::prev(container.end()));
	}

	struct Coord {
		int x = 0;
		int y = 0;
	};

	template<typename Object>
	concept RenderObject = requires(Object obj) {
		obj.move(1, -1);
		obj.move(SDL_Point{});
		obj.scale(1, -1);
		obj.scale(SDL_Point{});
		obj.containsCoord(1, -1);
		obj.containsCoord(SDL_Point{});
		obj.render();
	};

	template<typename Object>
	concept RotatableObject = requires(Object obj) {
		{ obj } -> RenderObject;
		obj.rotate(0.0, SDL_Point{});
		obj.setRotationCenter();
	};

	template<typename Object>
	concept SizeableObject = requires(Object obj) {
		obj.setSize(500, 500);
		obj.setSize(SDL_Point{ 500, 500 });
		{ obj.getSize() } -> std::same_as<SDL_Point>;
	};

	template<typename Range>
	concept RenderObjectRange = ranges::viewable_range<Range> && RenderObject<typename Range::value_type>;

	//get that works with tuples AND aggregates
	template<size_t Idx, typename T>
	constexpr decltype(auto) get(T&& t) {
		if constexpr (std::is_aggregate_v<std::remove_cvref_t<T>>) {
			return pfr::get<Idx>(std::forward<T>(t)); //aggregate case
		} else {
			return std::get<Idx>(std::forward<T>(t)); //tuple case
		}
	}
	
	namespace detail {
		template<size_t Idx, typename T>
		struct GetType {
			using Plain = std::remove_cvref_t<T>;
			using type = std::conditional_t< 
				std::is_aggregate_v<Plain>,
				pfr::tuple_element_t<Idx, Plain>,
				std::tuple_element_t<Idx, Plain>
			>;
		};
	}
	//tuple_element_t that works with tuples and aggregates
	template<size_t Idx, typename T>
	using GetType = typename detail::GetType<Idx, T>::type;

	//tuple_size_v that works with tuples and aggregates
	template<typename T>
	consteval size_t memberCount() {
		using Plain = std::remove_cvref_t<T>;
		if constexpr (std::is_aggregate_v<std::remove_cvref_t<T>>) {
			return pfr::tuple_size_v<Plain>; //aggregate case
		} else {
			return std::tuple_size_v<Plain>; //tuple case
		}
	}

	//used for applying a trait to all the types in a tuple. Ex: tuple<int, double> -> tuple<const int* const, const double* const>
	template<typename Tuple, template<typename> typename Trait>
	struct TypeTransform {
	private:
		template<size_t... Idxs>
		static auto dummy(std::index_sequence<Idxs...>) {
			return std::tuple<typename Trait<GetType<Idxs, Tuple>>::type...>();
		}
	public:
		using type = decltype(dummy(std::make_index_sequence<memberCount<Tuple>()>()));
	};

	namespace detail {
		template<size_t MemberIdx, typename Func, typename TiedStructs, size_t... StructIdxs>
		constexpr bool iterateStructMembers(Func f, TiedStructs tiedStructs, std::index_sequence<StructIdxs...>) {
			auto tiedMembers = std::tie(get<MemberIdx>(get<StructIdxs>(std::forward<TiedStructs>(tiedStructs)))...);
			return std::apply(f, tiedMembers);
		}

		template<typename Func, typename TiedStructs, size_t... MemberIdxs>
		constexpr bool iterateStructsImpl(Func f, TiedStructs tiedStructs, std::index_sequence<MemberIdxs...>) {
			return ((iterateStructMembers<MemberIdxs>(f, tiedStructs, std::make_index_sequence<memberCount<TiedStructs>()>())) || ...);
		}
	}

	template<typename Func, typename FirstStruct, typename... Structs> //FirstStruct param lets us extract member count of each tuple (assumes all tuples have same count of members)
	constexpr bool iterateStructs(Func f, FirstStruct& firstStruct, Structs&... structs) {
		return detail::iterateStructsImpl(f, std::tie(firstStruct, structs...), std::make_index_sequence<memberCount<FirstStruct>()>());
	}

	inline constexpr bool STAY_IN_LOOP = false;
	inline constexpr bool BREAK_FROM_LOOP = true;

	std::string writeCloneID(std::string_view str);

	template<typename T>
	using Layers = boost_con::flat_map<int, std::vector<T>>;

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

	template<typename T>
	using Subrange = ranges::subrange<typename std::vector<T>::iterator>;

	class NamedObject {
	protected:
		std::string m_name;
	public:
		const std::string& getName() const noexcept;
	};

	json parseFile(std::string_view filename);
};

#endif