#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>

namespace domain {

	struct Stop {
		std::string name;
		double latitude;
		double longitude;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> route;
		bool is_round;
	};

	double GetRouteGeoLength(const Bus* bus);

}