#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue {

	namespace input {

		catalogue::TransportCatalogue ReadRequest(std::istream& stream);

	}

}