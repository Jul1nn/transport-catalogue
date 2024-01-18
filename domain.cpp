#include "domain.h"

namespace domain {

	double GetRouteGeoLength(const Bus* bus) {
		using namespace geo;
		double L_G = 0.0;
		Coordinates prev_stop_coord = { bus->route[0]->latitude, bus->route[0]->longitude };
		for (size_t i = 1; i < bus->route.size(); ++i) {
			Coordinates current_stop_coord = { bus->route[i]->latitude, bus->route[i]->longitude };
			L_G += ComputeDistance(prev_stop_coord, current_stop_coord);
			prev_stop_coord = current_stop_coord;
		}
		if (!bus->is_round) {
			L_G *= 2.0;
		}
		return L_G;
	}

} //namespace domain