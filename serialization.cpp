#include "serialization.h"

#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

Serializator::Serializator(string path)
	:filename_(path) {}

void Serializator::Serialize(const transport_catalogue::catalogue::TransportCatalogue& cat, const MapRenderer& rend, const RouterBuilder& rout) {
	SerializeCatalogue(cat);
	SerializeRenderer(cat, rend);
	SerializeRouter(cat, rout);
	ofstream out(filename_, std::ios::binary);
	db_serialization_.SerializeToOstream(&out);
}

Serializator::DeserializationResult Serializator::Deserialize() {
	ifstream in(filename_, std::ios::binary);
	db_serialization_.ParseFromIstream(&in);
	transport_catalogue::catalogue::TransportCatalogue catalogue = DeserializeCatalogue();
	MapRenderer renderer = DeserializeRenderer(catalogue);
	RouterBuilder router = DeserializeRouter(catalogue);
	return { move(catalogue), move(renderer), move(router) };
}

void Serializator::SerializeCatalogue(const transport_catalogue::catalogue::TransportCatalogue& cat) {
	const deque<domain::Stop>& stops = cat.GetStopsList();
	for (const domain::Stop& stop : stops) {
		transport_catalogue_serialize::Stop* st = db_serialization_.add_stops();
		st->set_name(stop.name);
		st->set_latitude(stop.latitude);
		st->set_longitude(stop.longitude);
	}
	const deque<domain::Bus>& buses = cat.GetBusesList();
	for (const domain::Bus& bus : buses) {
		transport_catalogue_serialize::Bus* bs = db_serialization_.add_buses();
		bs->set_name(bus.name);
		bs->set_is_round(bus.is_round);
		for (auto stop : bus.route) {
			auto it = find_if(stops.begin(), stops.end(), [stop](const domain::Stop& obj) { return stop == &obj; });
			int stop_num = it - stops.begin();
			bs->add_stops(stop_num);
		}
	}
	const auto& distances = cat.GetDistances();
	for (const auto& [stops_pair, distance] : distances) {
		auto it_from = find_if(stops.begin(), stops.end(), [stops_pair](const domain::Stop& obj) { return stops_pair.first == &obj; });
		int num_from = it_from - stops.begin();
		auto it_to = find_if(stops.begin(), stops.end(), [stops_pair](const domain::Stop& obj) { return stops_pair.second == &obj; });
		int num_to = it_to - stops.begin();
		transport_catalogue_serialize::Distance* dist = db_serialization_.add_distances();
		dist->set_stop_from(num_from);
		dist->set_stop_to(num_to);
		dist->set_distance(distance);
	}
}

void Serializator::SerializeRenderer(const transport_catalogue::catalogue::TransportCatalogue& cat, const MapRenderer& rend) {
	map_renderer_serialize::MapRenderer* renderer_s = db_serialization_.mutable_renderer();

	const RenderSettings& settings = rend.GetRenderSettings();
	map_renderer_serialize::RenderSettings* sets_s = renderer_s->mutable_settings();
	sets_s->set_width(settings.width);
	sets_s->set_height(settings.height);
	sets_s->set_padding(settings.padding);
	sets_s->set_line_width(settings.line_width);
	sets_s->set_stop_radius(settings.stop_radius);
	sets_s->set_bus_label_font_size(settings.bus_label_font_size);
	sets_s->mutable_bus_label_offset()->set_x(settings.bus_label_offset.x);
	sets_s->mutable_bus_label_offset()->set_y(settings.bus_label_offset.y);
	sets_s->set_stop_label_font_size(settings.stop_label_font_size);
	sets_s->mutable_stop_label_offset()->set_x(settings.stop_label_offset.x);
	sets_s->mutable_stop_label_offset()->set_y(settings.stop_label_offset.y);

	SerializeColor(sets_s->mutable_underlayer_color(), settings.underlayer_color);

	sets_s->set_underlayer_width(settings.underlayer_width);

	for (const auto& color : settings.color_palette) {
		SerializeColor(sets_s->add_color_palette(), color);
	}

	const deque<domain::Stop>& stops = cat.GetStopsList();
	const deque<domain::Bus>& buses = cat.GetBusesList();

	const vector<const domain::Stop*> stops_sorted = rend.GetStopsSortedList();
	const unordered_map<const domain::Stop*, svg::Point>& stops_with_coordinates = rend.GetStopsWithCoordinates();
	for (auto stop : stops_sorted) {
		auto it = find_if(stops.begin(), stops.end(), [stop](const domain::Stop& obj) { return stop == &obj; });
		int stop_num = it - stops.begin();
		svg_serialize::Point point_s;
		point_s.set_x(stops_with_coordinates.at(stop).x);
		point_s.set_y(stops_with_coordinates.at(stop).y);
		renderer_s->add_stops_sorted_list(stop_num);
		(*renderer_s->mutable_stops_coordinates())[stop_num] = point_s;
	}

	const vector<const domain::Bus*>& buses_sorted = rend.GetBusesSortedList();
	for (auto bus : buses_sorted) {
		auto it = find_if(buses.begin(), buses.end(), [bus](const domain::Bus& obj) { return bus == &obj; });
		int bus_num = it - buses.begin();
		renderer_s->add_buses_sorted_list(bus_num);
	}
}

transport_catalogue::catalogue::TransportCatalogue Serializator::DeserializeCatalogue() const {
	transport_catalogue::catalogue::TransportCatalogue result_cat;
	for (int i = 0; i < db_serialization_.stops_size(); ++i) {
		result_cat.AddStop(db_serialization_.stops().at(i).name(), db_serialization_.stops().at(i).latitude(), db_serialization_.stops().at(i).longitude());
	}
	for (const auto& dist : db_serialization_.distances()) {
		result_cat.AddDistance(db_serialization_.stops().at(dist.stop_from()).name(), db_serialization_.stops().at(dist.stop_to()).name(), dist.distance());
	}
	for (const transport_catalogue_serialize::Bus& bus : db_serialization_.buses()) {
		vector<string_view> stops_list;
		stops_list.reserve(bus.stops_size());
		for (int stop : bus.stops()) {
			stops_list.push_back(db_serialization_.stops().at(stop).name());
		}
		result_cat.AddBus(bus.name(), stops_list, bus.is_round());
	}
	return result_cat;

}

MapRenderer Serializator::DeserializeRenderer(const transport_catalogue::catalogue::TransportCatalogue& cat) const {
	const map_renderer_serialize::RenderSettings& r_sets_serialize = db_serialization_.renderer().settings();
	svg::Color undrlr_color = DeserializeColor(r_sets_serialize.underlayer_color());
	vector<svg::Color> palette;
	palette.reserve(r_sets_serialize.color_palette_size());
	for (const auto& color : r_sets_serialize.color_palette()) {
		palette.push_back(DeserializeColor(color));
	}
	RenderSettings r_sets = {
		r_sets_serialize.width(),
		r_sets_serialize.height(),
		r_sets_serialize.padding(),
		r_sets_serialize.line_width(),
		r_sets_serialize.stop_radius(),
		r_sets_serialize.bus_label_font_size(),
		{ r_sets_serialize.bus_label_offset().x(), r_sets_serialize.bus_label_offset().y() },
		r_sets_serialize.stop_label_font_size(),
		{ r_sets_serialize.stop_label_offset().x(), r_sets_serialize.stop_label_offset().y() },
		move(undrlr_color),
		r_sets_serialize.underlayer_width(),
		move(palette)
	};
	MapRenderer result_rend(r_sets);
	for (int bus_num : db_serialization_.renderer().buses_sorted_list()) {
		result_rend.AddBus(cat.FindBus(db_serialization_.buses().at(bus_num).name()));
	}
	for (int stop_num : db_serialization_.renderer().stops_sorted_list()) {
		result_rend.AddStop(cat.FindStop(db_serialization_.stops().at(stop_num).name()));
		result_rend.AddStopCoordinates(cat.FindStop(db_serialization_.stops().at(stop_num).name()), 
			{ db_serialization_.renderer().stops_coordinates().at(stop_num).x(), db_serialization_.renderer().stops_coordinates().at(stop_num).y() });
	}
	return result_rend;
}

void Serializator::SerializeColor(svg_serialize::Color* clr, const variant<std::monostate, string, svg::Rgb,
	svg::Rgba>& source_clr) {
	switch (source_clr.index()) {
	case 0:
		break;
	case 1:
		clr->set_str(get<1>(source_clr));
		break;
	case 2:
		clr->mutable_rgb()->set_red(static_cast<uint32_t>(get<2>(source_clr).red));
		clr->mutable_rgb()->set_green(static_cast<uint32_t>(get<2>(source_clr).green));
		clr->mutable_rgb()->set_blue(static_cast<uint32_t>(get<2>(source_clr).blue));
		break;
	case 3:
		clr->mutable_rgba()->set_red(static_cast<uint32_t>(get<3>(source_clr).red));
		clr->mutable_rgba()->set_green(static_cast<uint32_t>(get<3>(source_clr).green));
		clr->mutable_rgba()->set_blue(static_cast<uint32_t>(get<3>(source_clr).blue));
		clr->mutable_rgba()->set_opacity(get<3>(source_clr).opacity);
		break;
	}
}

variant<std::monostate, string, svg::Rgb, svg::Rgba> Serializator::DeserializeColor(const svg_serialize::Color& source_clr) const {
	switch (source_clr.type_case()) {
	case svg_serialize::Color::TypeCase::TYPE_NOT_SET :
		return {};
		break;
	case svg_serialize::Color::TypeCase::kStr :
		return source_clr.str();
		break;
	case svg_serialize::Color::TypeCase::kRgb :
		return svg::Rgb{ static_cast<uint8_t>(source_clr.rgb().red()), static_cast<uint8_t>(source_clr.rgb().green()), static_cast<uint8_t>(source_clr.rgb().blue()) };
		break;
	case svg_serialize::Color::TypeCase::kRgba :
		return svg::Rgba{ static_cast<uint8_t>(source_clr.rgba().red()), static_cast<uint8_t>(source_clr.rgba().green()), static_cast<uint8_t>(source_clr.rgba().blue()), source_clr.rgba().opacity() };
		break;
	}
	return {};
}

void Serializator::SerializeRouter(const transport_catalogue::catalogue::TransportCatalogue& cat, const RouterBuilder& rout)
{
	transport_router_serialize::TransportRouter* tr_router_s = db_serialization_.mutable_transport_router();

	tr_router_s->mutable_settings()->set_wait_time(rout.GetSettings().wait_time);
	tr_router_s->mutable_settings()->set_velocity(rout.GetSettings().velocity);

	graph_serialize::Graph* graph_s = tr_router_s->mutable_router()->mutable_graph();
	const graph::DirectedWeightedGraph<double>& gr = rout.GetRouter().GetGraph();
	for (size_t i = 0; i < gr.GetEdgeCount(); ++i) {
		graph_serialize::Edge* edge_s = graph_s->add_edges();
		edge_s->set_from(static_cast<uint32_t>(gr.GetEdge(i).from));
		edge_s->set_to(static_cast<uint32_t>(gr.GetEdge(i).to));
		edge_s->set_weight(gr.GetEdge(i).weight);
	}
	for (size_t i = 0; i < gr.GetVertexCount(); ++i) {
		graph_serialize::IncidenceList* inc_list_s = graph_s->add_incidence_lists();
		for (size_t inc_edge : gr.GetIncidentEdges(i)) {
			inc_list_s->add_incident_edges(static_cast<uint32_t>(inc_edge));
		}
	}

	const graph::Router<double>::RoutesInternalData& r_int_d = rout.GetRouter().GetRoutesData();
	for (const auto& r_int_data : r_int_d) {
		transport_router_serialize::RouteInternalDataVector* r_int_d_vect_s = tr_router_s->mutable_router()->add_routes_internal_data();
		for (const auto& r_info : r_int_data) {
			transport_router_serialize::RouteInternalDataOptional r_int_d_opt_s;
			if (r_info.has_value()) {
				transport_router_serialize::RouteInternalData r_int_d_s;
				r_int_d_s.set_weight(r_info->weight);
				if (r_info->prev_edge.has_value()) {
					r_int_d_s.set_prev_edge(static_cast<uint32_t>(r_info->prev_edge.value()));
				}
				*r_int_d_opt_s.mutable_datas() = r_int_d_s;
			}
			*r_int_d_vect_s->add_datas() = r_int_d_opt_s;
		}
	}

	const deque<domain::Stop>& stops = cat.GetStopsList();
	const deque<domain::Bus>& buses = cat.GetBusesList();
	for (const auto& [edge_id, route_info] : rout.GetEdgesInfo()) {
		transport_router_serialize::RouteEdgeInfo info_s;
		auto it_stop = find_if(stops.begin(), stops.end(), [route_info](const domain::Stop& obj) { return route_info.start == &obj; });
		int stop_num = it_stop - stops.begin();
		info_s.set_start(stop_num);
		auto it_bus = find_if(buses.begin(), buses.end(), [route_info](const domain::Bus& obj) { return route_info.bus == &obj; });
		int bus_num = it_bus - buses.begin();
		info_s.set_bus(bus_num);
		info_s.set_spans(route_info.spans);
		info_s.set_item_wait(route_info.item_wait);
		info_s.set_item_bus(route_info.item_bus);
		(*tr_router_s->mutable_edges_to_buses())[static_cast<uint32_t>(edge_id)] = move(info_s);
	}

}

RouterBuilder Serializator::DeserializeRouter(const transport_catalogue::catalogue::TransportCatalogue& cat)
{
	RoutingSettings sets = { db_serialization_.transport_router().settings().wait_time(), db_serialization_.transport_router().settings().velocity() };
	RouterBuilder rout(move(sets));

	auto graph_ptr = new graph::DirectedWeightedGraph<double>(db_serialization_.transport_router().router().graph().incidence_lists_size());
	for (const graph_serialize::Edge& edge_s : db_serialization_.transport_router().router().graph().edges()) {
		graph::Edge<double> edge = { static_cast<size_t>(edge_s.from()), static_cast<size_t>(edge_s.to()), edge_s.weight() };
		graph_ptr->AddEdge(edge);
	}
	rout.InitializeGraph(graph_ptr);
	
	vector<vector<optional<graph::Router<double>::RouteInternalData>>> routes_internal_data;
	routes_internal_data.reserve(db_serialization_.transport_router().router().routes_internal_data_size());
	for (const transport_router_serialize::RouteInternalDataVector& r_int_d_vect_s : db_serialization_.transport_router().router().routes_internal_data()) {
		vector<optional<graph::Router<double>::RouteInternalData>> r_int_d_vect;
		r_int_d_vect.reserve(r_int_d_vect_s.datas_size());
		for (const transport_router_serialize::RouteInternalDataOptional& r_int_opt_s : r_int_d_vect_s.datas()) {
			optional<graph::Router<double>::RouteInternalData> route_info_opt;
			if (r_int_opt_s.datas_opt_case() == transport_router_serialize::RouteInternalDataOptional::DatasOptCase::kDatas) {
				graph::Router<double>::RouteInternalData route_info;
				route_info.weight = r_int_opt_s.datas().weight();
				if (r_int_opt_s.datas().edge_case() == transport_router_serialize::RouteInternalData::EdgeCase::kPrevEdge) {
					route_info.prev_edge = static_cast<size_t>(r_int_opt_s.datas().prev_edge());
				}
				route_info_opt = route_info;
			}
			r_int_d_vect.push_back(route_info_opt);
		}
		routes_internal_data.push_back(r_int_d_vect);
	}
	auto router_ptr = new graph::Router<double>(rout.GetGraph(), move(routes_internal_data));
	rout.InitializeRouter(router_ptr);

	for (const auto& [edge_id_s, info_s] : db_serialization_.transport_router().edges_to_buses()) {
		RouteEdgeInfo info;
		info.start = cat.FindStop(db_serialization_.stops().at(info_s.start()).name());
		info.bus = cat.FindBus(db_serialization_.buses().at(info_s.bus()).name());
		info.spans = info_s.spans();
		info.item_wait = info_s.item_wait();
		info.item_bus = info_s.item_bus();
		rout.AddEdgeInfo(static_cast<size_t>(edge_id_s), move(info));
	}

	size_t i = 0;
	for (const domain::Stop& stop : cat.GetStopsList()) {
		rout.AddStopVertice(&stop, i);
		++i;
	}

	return rout;
}


