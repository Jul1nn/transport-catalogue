#include "transport_catalogue.h"
#include "geo.h"

#include <stdexcept>
#include <algorithm>
#include <unordered_set>

using namespace std;

namespace transport_catalogue::catalogue {

	using namespace domain;

	void TransportCatalogue::AddStop(const string& name, double latitude, double longitude) {
		stops_.push_back({ name, latitude, longitude });
		stops_search_.insert({ stops_.back().name, &stops_.back() });
	}

	const Stop* TransportCatalogue::FindStop(string_view stop_name) const {
		if (stops_search_.count(stop_name) == 0) {
			throw invalid_argument("Stop not found"s);
		}
		return stops_search_.at(stop_name);
	}

	void TransportCatalogue::AddBus(const string& name, const vector<string_view>& stops, bool is_round) {
		vector<const Stop*> route;
		for (const auto& stop : stops) {
			route.push_back(FindStop(stop));
		}
		buses_.push_back({ name, route, is_round });
		buses_search_.insert({ buses_.back().name, &buses_.back() });
		for (const auto& stop : route) {
			stop_to_buses_[stop].insert(buses_.back().name);
		}
	}

	const Bus* TransportCatalogue::FindBus(string_view bus_name) const {
		if (buses_search_.count(bus_name) == 0) {
			throw invalid_argument("Bus not found"s);
		}
		return buses_search_.at(bus_name);
	}

	void TransportCatalogue::AddDistance(string_view stop1, string_view stop2, int distance) {
		pair<const Stop*, const Stop*> stops = { FindStop(stop1), FindStop(stop2) };
		distances_[stops] = distance;
	}

	int TransportCatalogue::FindDistance(const Stop* stop1, const Stop* stop2) const {
		pair<const Stop*, const Stop*> stops = { stop1, stop2 };
		if (distances_.count(stops) == 0) {
			pair<const Stop*, const Stop*> stops_vv = { stop2, stop1 };
			return distances_.at(stops_vv);
		}
		return distances_.at(stops);
	}

	TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(string_view bus_name) const {
		unordered_set<const Stop*> dupls;
		double L_D = 0.0;
		const Bus* bus = FindBus(bus_name);
		const Stop* prev_stop = bus->route[0];
		dupls.insert(bus->route[0]);
		for (size_t i = 1; i < bus->route.size(); ++i) {
			dupls.insert(bus->route[i]);
			L_D += FindDistance(prev_stop, bus->route[i]);
			if (!bus->is_round) {
				L_D += FindDistance(bus->route[i], prev_stop);
			}
			prev_stop = bus->route[i];
		}
		int U = dupls.size();
		int R = bus->route.size();
		if (!bus->is_round) {
			R = R * 2 - 1;
		}
		double C = 1.0 * L_D / GetRouteGeoLength(bus);
		return { R, U, L_D, C };
	}

	const set<string_view>& TransportCatalogue::GetStopInfo(string_view stop_name) const {
		const Stop* stop = FindStop(stop_name);
		string s_exception("no buses"s);
		if (stop_to_buses_.count(stop) == 0) {
			throw s_exception;
		}
		return stop_to_buses_.at(stop);
	}

	const deque<Bus>& TransportCatalogue::GetBusesList() const {
		return buses_;
	}

	const deque<Stop>& TransportCatalogue::GetStopsList() const {
		return stops_;
	}

	const unordered_map<std::pair<const Stop*, const Stop*>, int, TransportCatalogue::StopHasher>& TransportCatalogue::GetDistances() const {
		return distances_;
	}

}