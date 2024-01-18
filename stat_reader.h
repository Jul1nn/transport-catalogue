#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue {

	namespace output {

		void PrintRequest(std::istream& input, const catalogue::TransportCatalogue& catalogue, std::ostream& output);

	}

}