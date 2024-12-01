#pragma once

#include <chrono>
#include <vector>

#include <boost/pfr.hpp>
#include <boost/container/flat_map.hpp>

#include <plf_hive.h>

#include <nlohmann/json.hpp>

#include <SDL2/SDL_rect.h>

#include "BasicConcepts.h"

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

namespace plf {
	template<typename T>
	void to_json(nlohmann::json& j, const hive<T>& hive) {
		std::vector<T> vec;
		vec.append_range(hive);
		j = vec;
	}
	template<typename T>
	void from_json(const nlohmann::json& j, hive<T>& hive) {
		auto vec = j.get<std::vector<T>>();
		hive.insert(vec.begin(), vec.end());
	}
}

void to_json(nlohmann::json& j, const SDL_Color& c);
void from_json(const nlohmann::json& j, SDL_Color& c);

void to_json(nlohmann::json& j, const SDL_Rect& c);
void from_json(const nlohmann::json& j, SDL_Rect& c);

void to_json(nlohmann::json& j, const SDL_Point& p);
void from_json(const nlohmann::json& j, SDL_Point& p);

namespace nv {
	namespace chrono = std::chrono;
	namespace pfr = boost::pfr;
	namespace boost_con = boost::container;

	//class aliases
	using nlohmann::json;

	//literals
	using namespace std::literals;

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

	json parseJson(std::string_view filename);

	void printJsonKeys(const json& json);
}