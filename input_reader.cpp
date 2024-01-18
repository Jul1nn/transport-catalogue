#include "input_reader.h"
#include <string>
#include <istream>
#include <map>

using namespace std;

namespace transport_catalogue::input {

	using namespace transport_catalogue::catalogue;

	TransportCatalogue ReadRequest(std::istream& stream) {
		TransportCatalogue catalogue;
		string N;
		getline(stream, N);
		vector<string> requests;
		requests.reserve(stoi(N));
		for (int i = 0; i < stoi(N); ++i) {
			string S;
			getline(stream, S);
			requests.push_back(move(S));
		}
		map<pair<string_view, string_view>, int> prelim_distances;
		for (const string& str : requests) {
			size_t pos = str.find_first_of(' ');
			if (str.substr(0, pos) == "Stop"s) {
				size_t pos1 = str.find(':');
				size_t pos2 = str.find_first_of(',');
				size_t pos3 = str.find_first_of(',', pos2 + 1);
				string stop_name = str.substr(5, pos1 - 5);
				catalogue.AddStop(stop_name, stod(str.substr(pos1 + 2, pos2 - pos1 - 2)), stod(str.substr(pos2 + 2, pos3 - pos2 - 2)));
			}
		}
		for (const string& str : requests) {
			size_t pos = str.find_first_of(' ');
			if (str.substr(0, pos) == "Stop"s) {
				size_t pos1 = str.find(':');
				size_t pos2 = str.find_first_of(',');
				size_t pos3 = str.find_first_of(',', pos2 + 1);
				if (pos3 != string::npos) {
					string stop_name = str.substr(5, pos1 - 5);
					size_t pos4 = str.find_first_of(',', pos3 + 1);
					size_t pos5 = str.find_first_of('m', pos3);
					while (pos4 != string::npos) {
						catalogue.AddDistance(stop_name, str.substr(pos5 + 5, pos4 - pos5 - 5), stoi(str.substr(pos3 + 2, pos5 - pos3 - 2)));
						pos3 = pos4;
						pos5 = str.find_first_of('m', pos3);
						pos4 = str.find_first_of(',', pos4 + 1);
					}
					catalogue.AddDistance(stop_name, str.substr(pos5 + 5, string::npos), stoi(str.substr(pos3 + 2, pos5 - pos3 - 2)));
				}
			}
		}
		for (const string& str : requests) {
			size_t pos = str.find_first_of(' ');
			if (str.substr(0, pos) == "Bus"s) {
				vector<string_view> route;
				size_t pos1 = str.find(':');
				string bus_name = str.substr(4, pos1 - 4);
				size_t pos2 = str.find_first_of('-');
				if (pos2 != string::npos) {
					while (pos2 != string::npos) {
						route.push_back(str.substr(pos1 + 2, pos2 - pos1 - 3));
						pos1 = pos2;
						pos2 = str.find_first_of('-', pos2 + 1);
					}
					route.push_back(str.substr(pos1 + 2, string::npos));
					catalogue.AddBus(bus_name, route, true);
				}
				else {
					pos2 = str.find_first_of('>');
					while (pos2 != string::npos) {
						route.push_back(str.substr(pos1 + 2, pos2 - pos1 - 3));
						pos1 = pos2;
						pos2 = str.find_first_of('>', pos2 + 1);
					}
					route.push_back(str.substr(pos1 + 2, string::npos));
					catalogue.AddBus(bus_name, route, false);
				}
			}
		}
		return catalogue;
	}
}