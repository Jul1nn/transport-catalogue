#pragma once

#include "domain.h"

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <string_view>
#include <set>

namespace transport_catalogue {

	namespace catalogue {

		using namespace domain;

		class TransportCatalogue {
		public:

			struct BusInfo {
				int stops_on_route;
				int unique_stops;
				double route_length;
				double route_curvature;
			};

			struct StopHasher {
				size_t operator() (std::pair<const Stop*, const Stop*> stops) const {
					size_t h_1 = stop_hasher(stops.first);
					size_t h_2 = stop_hasher(stops.second);
					return h_1 * 37 + h_2 * 37 * 37;
				}

			private:
				std::hash<const void*> stop_hasher;
			};

		public:
			void AddStop(const std::string& name, double latitude, double longitude);
			const Stop* FindStop(std::string_view stop_name) const;
			void AddBus(const std::string& name, const std::vector<std::string_view>& stops, bool is_round);
			const Bus* FindBus(std::string_view bus_name) const;
			void AddDistance(std::string_view stop1, std::string_view stop2, int distance);
			int FindDistance(const Stop* stop1, const Stop* stop2) const;
			BusInfo GetBusInfo(std::string_view bus_name) const;
			const std::set<std::string_view>& GetStopInfo(std::string_view stop_name) const;
			const std::deque<Bus>& GetBusesList() const;
			const std::deque<Stop>& GetStopsList() const;
			const std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher>& GetDistances() const;

		private:

			std::deque<Stop> stops_;
			std::unordered_map<std::string_view, const Stop*> stops_search_;
			std::deque<Bus> buses_;
			std::unordered_map<std::string_view, const Bus*> buses_search_;
			std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;
			std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher> distances_;
		};

	}

}