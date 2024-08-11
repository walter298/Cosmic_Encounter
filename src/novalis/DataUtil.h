#ifndef DATA_UTIL_H
#define DATA_UTIL_H

#include <algorithm> //equal_range
#include <chrono>
#include <deque>
#include <filesystem>
#include <ranges> //viewable_range
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/optional.hpp>
#include <boost/pfr.hpp>

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

	//returns the path relative to the working directory
	const std::string& relativePath(std::string_view relativePath);

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
	template<typename Object>
	concept RenderObject = requires(Object& obj) {
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

	template<typename... Ts>
	using ObjectLayers = boost_con::flat_map<int, std::tuple<std::vector<Ts>...>>;

	//get that works with tuples AND aggregates
	template<size_t Idx, typename T>
	constexpr decltype(auto) powerGet(T&& t) {
		if constexpr (std::is_aggregate_v<std::remove_cvref_t<T>>) {
			return pfr::get<Idx>(std::forward<T>(t)); //aggregate case
		} else {
			return std::get<Idx>(std::forward<T>(t)); //tuple case
		}
	}
	
	namespace detail {
		template<bool B, size_t Idx, typename T>
		struct GetTypeImpl {
			using type = std::tuple_element_t<Idx, std::remove_cvref_t<T>>;
		};

		template<size_t Idx, Aggregate T>
		struct GetTypeImpl<true, Idx, T> {
			using type = pfr::tuple_element_t<Idx, std::remove_cvref_t<T>>;
		};
	}

	template<size_t Idx, typename T>
	using GetType = typename detail::GetTypeImpl<std::is_aggregate_v<std::remove_cvref<T>>, Idx, T>::type;

	//tuple_size_v that works with tuples and aggregates
	template<typename T>
	consteval size_t memberCount() {
		using Plain = std::remove_cvref_t<T>;
		if constexpr (std::is_aggregate_v<Plain>) {
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
			auto tiedMembers = std::tie(powerGet<MemberIdx>(powerGet<StructIdxs>(std::forward<TiedStructs>(tiedStructs)))...);
			return std::apply(f, tiedMembers);
		}

		template<typename Func, typename TiedStructs, size_t... MemberIdxs>
		constexpr bool iterateStructsImpl(Func f, TiedStructs tiedStructs, std::index_sequence<MemberIdxs...>) {
			return ((iterateStructMembers<MemberIdxs>(f, tiedStructs, std::make_index_sequence<memberCount<TiedStructs>()>())) || ...);
		}
	}

	template<typename Func, typename FirstStruct, typename... Structs> //FirstStruct param lets us extract member count of each tuple (assumes all tuples have same count of members)
	constexpr bool forEachDataMember(Func f, FirstStruct& firstStruct, Structs&... structs) {
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

	template<ranges::viewable_range Range>
	constexpr auto binaryFind(Range& range, typename Range::value_type& value) {
		return ranges::lower_bound(range, value);
	}

	template<typename T, typename U>
	concept SameAsDecayed = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
	
	template<ranges::viewable_range Range, typename Value, typename PMD>
	constexpr auto binaryFind(Range& range, const Value& value, PMD pmd) 
		requires(std::is_member_pointer_v<PMD> && SameAsDecayed<Value, decltype(std::declval<typename Range::value_type>().*pmd)>)
	{
		return ranges::lower_bound(range, value, [](const Value& v1, const Value& v2) {
			return v1 < v2;
		}, [&](const auto& elem) { return elem.*pmd; });
	}

	template<size_t N, typename... Ts>
	using PackIndex = GetType<N, std::tuple<Ts...>>;

	template<typename T>
	struct FunctionTraits { //primary template assumes function call operator
		using args = FunctionTraits<decltype(&T::operator())>::args;
	};

	template<typename R, typename... Args>
	struct FunctionTraits<R(Args...)> { //specialization for functions that haven't decayed
		using args = std::tuple<Args...>;
	};

	template<typename R, typename... Args>
	struct FunctionTraits<R(*)(Args...)> { //specialization for function pointers
		using args = std::tuple<Args...>;
	};

	template<typename C, typename R, typename... Args>
	struct FunctionTraits<R(C::*)(Args...) const> { //specialization for const member functions
		using args = std::tuple<Args...>;
	};

	template<typename C, typename R, typename... Args>
	struct FunctionTraits<R(C::*)(Args...)> { //specialization for mutable member functions
		using args = std::tuple<Args...>;
	};

	template<typename Func>
	using ResultOfNonOverloaded = decltype(
		std::apply(std::declval<std::decay_t<Func>>(), std::declval<typename FunctionTraits<Func>::args>())
	);

	json parseJson(const std::string& path);

	template<typename Value, typename... Keys>
	class TypeMap {
	private:
		std::array<Value, sizeof...(Keys)> m_values;

		template<typename KeyTarget, size_t I, typename IthKey, typename... OtherKeys>
		static consteval size_t getTypeIndex() {
			if constexpr (std::same_as<KeyTarget, IthKey>) {
				return I;
			} else {
				return getTypeIndex<KeyTarget, I + 1, OtherKeys...>();
			}
		}
	public:
		constexpr TypeMap() {
			ranges::fill(m_values, Value{});
		}
		template<std::same_as<Value>... Values>
		constexpr TypeMap(Values&&... values) requires(sizeof...(Values) > 0)
			: m_values{ std::forward<Values>(values)... } 
		{
		}

		template<typename Key>
		constexpr decltype(auto) get(this auto&& self) noexcept {
			static constexpr auto typeIdx = getTypeIndex<Key, 0, Keys...>();
			return self.m_values[typeIdx];
		}
	};

	namespace detail {
		template<size_t TypeIdx, bool Negation, template<typename> typename Pred, typename FilteredTuple, typename Tuple>
		constexpr auto filterDataMembersImpl(FilteredTuple filteredTuple, Tuple& tuple) {
			if constexpr (TypeIdx == memberCount<Tuple>()) {
				return filteredTuple;
			} else if constexpr (!Negation && Pred<GetType<TypeIdx, Tuple>>::value) {
				return filterDataMembersImpl<TypeIdx + 1, Negation, Pred>(
					std::tuple_cat(filteredTuple, std::tie(powerGet<TypeIdx>(tuple))), tuple
				);
			} else if constexpr (Negation && !Pred<GetType<TypeIdx, Tuple>>::value) {
				return filterDataMembersImpl<TypeIdx + 1, Negation, Pred>(
					std::tuple_cat(filteredTuple, std::tie(powerGet<TypeIdx>(tuple))), tuple
				);
			} else {
				return filterDataMembersImpl<TypeIdx + 1, Negation, Pred>(filteredTuple, tuple);
			}
		}
	}

	template<template<typename> typename Pred, bool Negation = false, typename T>
	constexpr auto filterDataMembers(T& t) {
		return detail::filterDataMembersImpl<0, Negation, Pred>(std::tuple{}, t);
	}

	template<typename Member, typename Transform, typename WhenFound, typename Pred, typename Tuple>
	constexpr void findMember(Transform getMember, WhenFound whenFound, Pred pred, Tuple& tuple) {
		forEachDataMember([&](auto& obj) {
			if constexpr (pred(obj)) {
				whenFound(obj);
				return BREAK_FROM_LOOP;
			} else {
				return STAY_IN_LOOP;
			}
		}, tuple);
	}

	namespace detail {
		template<typename Comp, typename Transform, typename WhenFound, typename FirstArg, typename SecondArg, typename... Args>
		void findCompImpl(Comp comp, Transform transform, WhenFound whenFound, FirstArg& first, SecondArg& second, Args&... args) {
			using ComparedType = decltype(std::invoke(transform, first));

			auto v1 = std::invoke(transform, first);
			auto v2 = std::invoke(transform, second);

			auto cont = [&](auto& bigger) {
				if constexpr (sizeof...(Args) == 0) {
					std::invoke(whenFound, bigger);
				} else {
					findCompImpl(comp, transform, whenFound, bigger, args...);
				}
			};
			if (v1 > v2) {
				cont(first);
			} else {
				cont(second);
			}
		}
	}

	template<typename Transform, typename WhenFound, typename FirstArg, typename SecondArg, typename... Args>
	void findMin(Transform transform, WhenFound whenFound, FirstArg& first, SecondArg& second, Args&... args) {
		auto comp = [](const auto& x, const auto& y) { return x < y; };
		detail::findCompImpl(comp, transform, whenFound, first, second, args...);
	}
	template<typename Transform, typename WhenFound, typename FirstArg, typename SecondArg, typename... Args>
	void findMax(Transform transform, WhenFound whenFound, FirstArg& first, SecondArg& second, Args&... args) {
		auto comp = [](const auto& x, const auto& y) { return x > y; };
		detail::findCompImpl(comp, transform, whenFound, first, second, args...);
	}

	template<typename T>
	struct IsReferenceWrapper : public std::false_type {};

	template<typename T>
	struct IsReferenceWrapper<std::reference_wrapper<T>> : public std::true_type {};

	template<typename T>
	auto& unrefwrap(T& t) {
		if constexpr (IsReferenceWrapper<std::remove_cvref_t<T>>::value) {
			return t.get();
		} else {
			return t;
		}
	}

	template<typename Range>
	using ValueType = typename std::remove_cvref_t<Range>::value_type;

	std::string& convertFullToRegularPath(std::string& path);
	std::string convertFullToRegularPath(std::string_view path);
};

#endif