#include "stat_reader.h"
#include <string>
#include <istream>
#include <iomanip>

using namespace std;

namespace transport_catalogue::output {

	using namespace transport_catalogue::catalogue;

	void PrintRequest(istream& input, const TransportCatalogue& catalogue, ostream& output) {
		string N;
		getline(input, N);
		for (int i = 0; i < stoi(N); ++i) {
			string S;
			getline(input, S);
			size_t pos = S.find_first_of(' ');
			if (S.substr(0, pos) == "Bus"s) {
				string bus_name = S.substr(4, string::npos);
				try {
					TransportCatalogue::BusInfo result = catalogue.GetBusInfo(bus_name);
					output << "Bus " << bus_name << ": " << result.stops_on_route << " stops on route, "
						<< result.unique_stops << " unique stops, " << std::setprecision(6)
						<< result.route_length << " route length, " << result.route_curvature
						<< " curvature" << endl;
				}
				catch (const exception& e) {
					output << "Bus " << bus_name << ": not found" << endl;
				}
			}
			if (S.substr(0, pos) == "Stop"s) {
				string stop_name = S.substr(5, string::npos);
				try {
					const set<string_view>& buses = catalogue.GetStopInfo(stop_name);
					output << "Stop " << stop_name << ": buses";
					for (auto bus : buses) {
						output << " " << bus;
					}
					output << endl;
				}
				catch (const exception& e) {
					output << "Stop " << stop_name << ": not found" << endl;
				}
				catch (const string& s) {
					output << "Stop " << stop_name << ": " << s << endl;
				}
			}
		}
	}

}