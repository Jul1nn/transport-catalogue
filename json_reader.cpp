#include "json_reader.h"
#include "json_builder.h"

#include <sstream>

using namespace std;

JSONReader::JSONReader(std::istream& input)
	:document_ (json::Load(input))
{	
}

void JSONReader::PrintRequestJSON(ostream& output, const RequestHandler& request_handler) const {
	using namespace json;

	const Array& requests = document_.GetRoot().AsDict().at("stat_requests"s).AsArray();
	Builder result_build;
	result_build.StartArray();
	for (const auto& request : requests) {
		if (request.AsDict().at("type"s).AsString() == "Stop"s) {
			result_build.StartDict().Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
			try {
				const set<string_view>& buses_list = request_handler.GetBusesByStop((request.AsDict().at("name"s).AsString()));
				result_build.Key("buses"s).StartArray();
				for (const auto& bus : buses_list) {
					result_build.Value(string(bus));
				}
				result_build.EndArray().EndDict();
			}
			catch (const invalid_argument& e) {
				result_build.Key("error_message"s).Value("not found"s).EndDict();
			}
			catch (const string& s) {
				result_build.Key("buses"s).StartArray().EndArray().EndDict();
			}
		}
		if (request.AsDict().at("type"s).AsString() == "Bus"s) {
			result_build.StartDict().Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
			try {
				const transport_catalogue::catalogue::TransportCatalogue::BusInfo& bus_info = request_handler.GetBusStat((request.AsDict().at("name"s).AsString()));
				result_build.Key("curvature"s).Value(bus_info.route_curvature).Key("stop_count"s).Value(bus_info.stops_on_route).
					Key("route_length"s).Value(bus_info.route_length).Key("unique_stop_count"s).Value(bus_info.unique_stops).
					EndDict();
			}
			catch (const invalid_argument& e) {
				result_build.Key("error_message"s).Value("not found"s).EndDict();
			}
		}
		if (request.AsDict().at("type"s).AsString() == "Map") {
			result_build.StartDict().Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
			ostringstream out;
			request_handler.RenderMap().Render(out);
			result_build.Key("map"s).Value(out.str()).EndDict();
		}
		if (request.AsDict().at("type"s).AsString() == "Route"s) {
			result_build.StartDict().Key("request_id"s).Value(request.AsDict().at("id"s).AsInt());
			OptimalRouteInfo optimal_route = request_handler.FindOptimalRoute(request.AsDict().at("from"s).AsString(), request.AsDict().at("to"s).AsString());
			if (optimal_route.sum_weight == nullopt) {
				result_build.Key("error_message"s).Value("not found"s).EndDict();
			}
			else {
				result_build.Key("total_time"s).Value(optimal_route.sum_weight.value());
				result_build.Key("items"s).StartArray();
				for (const auto& route_edge : optimal_route.edges_info) {
					result_build.StartDict().Key("type"s).Value("Wait"s).Key("stop_name"s).Value(route_edge.start->name).Key("time"s).Value(route_edge.item_wait).EndDict();
					result_build.StartDict().Key("type"s).Value("Bus"s).Key("bus"s).Value(route_edge.bus->name).
						Key("span_count"s).Value(route_edge.spans).Key("time"s).Value(route_edge.item_bus).EndDict();
				}
				result_build.EndArray().EndDict();
			}
		}
	}
	Print(Document(result_build.EndArray().Build()), output);
}

transport_catalogue::catalogue::TransportCatalogue JSONReader::BrowseCatalogue() const {
	transport_catalogue::catalogue::TransportCatalogue catalogue;
	const json::Array& requests = document_.GetRoot().AsDict().at("base_requests"s).AsArray();
	for (const auto& request : requests) {
		if (request.AsDict().at("type"s).AsString() == "Stop"s) {
			catalogue.AddStop(request.AsDict().at("name"s).AsString(), request.AsDict().at("latitude"s).AsDouble(), request.AsDict().at("longitude"s).AsDouble());
		}
	}
	for (const auto& request : requests) {
		if (request.AsDict().at("type"s).AsString() == "Stop"s) {
			for (const auto& [name, distance] : request.AsDict().at("road_distances"s).AsDict()) {
				catalogue.AddDistance(request.AsDict().at("name"s).AsString(), name, distance.AsInt());
			}
		}
	}
	for (const auto& request : requests) {
		if (request.AsDict().at("type"s).AsString() == "Bus"s) {
			vector<string_view> stops;
			for (const auto& stop_name : request.AsDict().at("stops"s).AsArray()) {
				stops.push_back(string_view(stop_name.AsString()));
			}
			catalogue.AddBus(request.AsDict().at("name"s).AsString(), stops, request.AsDict().at("is_roundtrip"s).AsBool());
		}
	}
	return catalogue;
}

MapRenderer JSONReader::BrowseRenderer() const {
	RenderSettings rs;
	const json::Dict& asdict = document_.GetRoot().AsDict().at("render_settings"s).AsDict();
	rs.width = asdict.at("width"s).AsDouble();
	rs.height = asdict.at("height"s).AsDouble();
	rs.padding = asdict.at("padding"s).AsDouble();
	rs.line_width = asdict.at("line_width"s).AsDouble();
	rs.stop_radius = asdict.at("stop_radius"s).AsDouble();
	rs.bus_label_font_size = asdict.at("bus_label_font_size"s).AsInt();
	rs.bus_label_offset = { asdict.at("bus_label_offset"s).AsArray()[0].AsDouble(),
		asdict.at("bus_label_offset"s).AsArray()[1].AsDouble() };
	rs.stop_label_font_size = asdict.at("stop_label_font_size"s).AsInt();
	rs.stop_label_offset = { asdict.at("stop_label_offset"s).AsArray()[0].AsDouble(),
		asdict.at("stop_label_offset"s).AsArray()[1].AsDouble() };
	if (asdict.at("underlayer_color"s).IsArray()) {
		const vector<json::Node>& color_name = asdict.at("underlayer_color"s).AsArray();
		if (color_name.size() == 3) {
			rs.underlayer_color = { svg::Rgb(static_cast<uint8_t>(color_name[0].AsInt()), static_cast<uint8_t>(color_name[1].AsInt()),
				static_cast<uint8_t>(color_name[2].AsInt())) };
		}
		else {
			rs.underlayer_color = { svg::Rgba(static_cast<uint8_t>(color_name[0].AsInt()), static_cast<uint8_t>(color_name[1].AsInt()),
				static_cast<uint8_t>(color_name[2].AsInt()), color_name[3].AsDouble()) };
		}
	}
	else if (asdict.at("underlayer_color"s).IsString()) {
		rs.underlayer_color = { asdict.at("underlayer_color"s).AsString() };
	}
	rs.underlayer_width = asdict.at("underlayer_width"s).AsDouble();
	const json::Array& color_names = asdict.at("color_palette"s).AsArray();
	for (const auto& color : color_names) {
		if (color.IsArray()) {
			if (color.AsArray().size() == 3) {
				rs.color_palette.push_back({ svg::Rgb(static_cast<uint8_t>(color.AsArray()[0].AsInt()), static_cast<uint8_t>(color.AsArray()[1].AsInt()),
					static_cast<uint8_t>(color.AsArray()[2].AsInt())) });
			}
			else {
				rs.color_palette.push_back({ svg::Rgba(static_cast<uint8_t>(color.AsArray()[0].AsInt()), static_cast<uint8_t>(color.AsArray()[1].AsInt()),
					static_cast<uint8_t>(color.AsArray()[2].AsInt()), color.AsArray()[3].AsDouble()) });
			}
		}
		else if (color.IsString()) {
			rs.color_palette.push_back({ color.AsString() });
		}
	}
	return { rs };
}

RouterBuilder JSONReader::BrowseRouter() const {
	const json::Dict& asdict = document_.GetRoot().AsDict().at("routing_settings"s).AsDict();
	return RouterBuilder({ asdict.at("bus_wait_time"s).AsInt(), asdict.at("bus_velocity"s).AsDouble() });
}

Serializator JSONReader::BrowseSerialization() const {
	const json::Dict& asdict = document_.GetRoot().AsDict().at("serialization_settings"s).AsDict();
	return { asdict.at("file"s).AsString() };
}

void JSONReader::ReadRequestJSON(istream& input) {
	document_ = json::Load(input);
}