#pragma once

#include <tuple>

#include <boost/functional/hash.hpp>
#include <boost/pfr.hpp>

#include "BasicConcepts.h"

namespace nv {
	namespace pfr = boost::pfr;

	//get that works with tuples AND aggregates
	template<size_t Idx, typename T>
	constexpr decltype(auto) powerGet(T&& t) {
		if constexpr (Aggregate<T>) {
			return pfr::get<Idx>(std::forward<T>(t)); //aggregate case
		}
		else {
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

	template<typename T>
	consteval size_t memberCount() {
		using Plain = std::remove_cvref_t<T>;
		if constexpr (Aggregate<Plain>) {
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

	template<size_t N, typename... Ts>
	using PackIndex = GetType<N, std::tuple<Ts...>>;

	template<typename T>
	struct FunctionTraits { //primary template assumes function call operator
		using Args = typename FunctionTraits<decltype(&T::operator())>::Args;
		using Ret  = typename FunctionTraits<decltype(&T::operator())>::Ret;
		using Sig  = typename FunctionTraits<decltype(&T::operator())>::Sig;
	};

	template<typename R, typename... Ts>
	struct FunctionTraits<R(Ts...)> { //specialization for functions that haven't decayed
		using Args = std::tuple<Ts...>;
		using Ret  = R;
		using Sig  = std::tuple<R, Ts...>;
	};

	template<typename R, typename... Ts>
	struct FunctionTraits<R(*)(Ts...)> { //specialization for function pointers
		using Args = std::tuple<Ts...>;
		using Ret  = R;
		using Sig = std::tuple<R, Ts...>;
	};

	template<typename C, typename R, typename... Ts>
	struct FunctionTraits<R(C::*)(Ts...) const> { //specialization for const member functions
		using Args = std::tuple<Ts...>;
		using Ret  = R;
		using Sig = std::tuple<R, Ts...>;
	};

	template<typename C, typename R, typename... Ts>
	struct FunctionTraits<R(C::*)(Ts...)> { //specialization for mutable member functions
		using Args = std::tuple<Ts...>;
		using Ret  = R;
		using Sig = std::tuple<R, Ts...>;
	};

	template<typename Value, typename... Keys>
	class TypeMap {
	private:
		std::array<Value, sizeof...(Keys)> m_values;

		template<typename KeyTarget, size_t I, typename IthKey, typename... OtherKeys>
		static consteval size_t getTypeIndex() {
			if constexpr (std::same_as<KeyTarget, IthKey>) {
				return I;
			}
			else {
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
			}
			else {
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

	template<template<typename...> typename ClassTemplate, typename>
	struct IsClassTemplate : public std::false_type {};

	template<template<typename...> typename ClassTemplate, typename... Ts>
	struct IsClassTemplate<ClassTemplate, ClassTemplate<Ts...>> : public std::true_type {};

	template<typename T>
	constexpr auto& unrefwrap(T& t) {
		if constexpr (IsClassTemplate<std::reference_wrapper, std::remove_cvref_t<T>>::value) {
			return t.get();
		} else {
			return t;
		}
	}

	template<typename T>
	constexpr auto& unptrwrap(T& t) {
		if constexpr (IsClassTemplate<std::unique_ptr, T>::value) {
			return *t;
		} else {
			return t;
		}
	}

	template<template<typename... Ts> typename T, typename... Ts>
	struct GetParameterizedTypeFromTuple {};

	template<template<typename... Ts> typename Parameterized, typename... Ts2>
	struct GetParameterizedTypeFromTuple<Parameterized, std::tuple<Ts2...>> {
		using type = Parameterized<Ts2...>;
	};

	template<typename Parameterized>
	struct GetTemplateTypes {};

	template<template<typename... GTs> typename Parameterized, typename... Ts>
	struct GetTemplateTypes<Parameterized<Ts...>> {
		using Types = std::tuple<Ts...>;
	};

	namespace detail {
		//this is moronic: compiler is too stupid to deduce an nttp pack of 0 length in an integer sequence
		template<size_t currentTypeIdx, typename BuiltUpType, typename T>
		constexpr auto excludeMembers(BuiltUpType builtUp, T&& t, std::integer_sequence<size_t>) {
			if constexpr (currentTypeIdx == memberCount<T>()) {
				return builtUp;
			} else {
				return excludeMembers<currentTypeIdx + 1>(std::tuple_cat(builtUp, std::tie(std::get<currentTypeIdx>(t))), t, std::integer_sequence<size_t>{});
			}
		}

		template<size_t currentTypeIdx, typename BuiltUpType, typename T, size_t currentExcludedTypeIdx, size_t... excludedTypeIdxs>
		constexpr auto excludeMembers(BuiltUpType builtUp, T&& t, std::integer_sequence<size_t, currentExcludedTypeIdx, excludedTypeIdxs...>) {
			if constexpr (currentTypeIdx == memberCount<T>()) {
				return builtUp;
			} else if constexpr (currentExcludedTypeIdx == currentTypeIdx) {
				return excludeMembers<currentTypeIdx + 1>(
					builtUp,
					t,
					std::integer_sequence<size_t, excludedTypeIdxs...>{}
				);
			} else {
				return excludeMembers<currentTypeIdx + 1>(
					std::tuple_cat(builtUp, std::tie(std::get<currentTypeIdx>(t))),
					t,
					std::integer_sequence<size_t, currentExcludedTypeIdx, excludedTypeIdxs...>{}
				);
			}
		}
	}

	template<typename T, size_t... excludedTypeIdxs>
	constexpr auto excludeMembers(T&& t, std::integer_sequence<size_t, excludedTypeIdxs...>) {
		return detail::excludeMembers<0>(std::tuple{}, t, std::integer_sequence<size_t, excludedTypeIdxs...>{});
	}

	struct HashAggregate {
		template<Aggregate Aggr>
		size_t operator()(const Aggr& aggr) const noexcept {
			size_t hashCode = 0;
			pfr::for_each_field(aggr, [&](const auto& member) {
				boost::hash_combine(hashCode, member);
			});
			return hashCode;
		}
	};

	struct CompareAggregates {
		template<Aggregate Aggr1, Aggregate Aggr2>
		bool operator()(const Aggr1 a, const Aggr2& b) const noexcept {
			bool ret = true;
			forEachDataMember([&](const auto& field1, const auto& field2) {
				if (field1 == field2) {
					ret = false;
					return BREAK_FROM_LOOP;
				}
				return STAY_IN_LOOP;
			}, a , b);
			return ret;
		}
	};
}