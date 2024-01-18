#include "transport_router.h"

#include <deque>
#include <algorithm>

using namespace std;
using namespace domain;

RouterBuilder::RouterBuilder(RoutingSettings set)
	: settings_(set) {}

void RouterBuilder::InitializeGraph(graph::DirectedWeightedGraph<double>* graph)
{
	graph_ptr_ = unique_ptr<graph::DirectedWeightedGraph<double>>(graph);
}

void RouterBuilder::InitializeRouter(graph::Router<double>* router)
{
	router_ptr_ = unique_ptr<graph::Router<double>>(router);
}

const RoutingSettings& RouterBuilder::GetSettings() const
{
	return settings_;
}

const graph::Router<double>& RouterBuilder::GetRouter() const
{
	return *router_ptr_;
}

const graph::DirectedWeightedGraph<double>& RouterBuilder::GetGraph() {
	return *graph_ptr_;
}

const std::unordered_map<size_t, RouteEdgeInfo>& RouterBuilder::GetEdgesInfo() const
{
	return edges_to_buses_;
}

void RouterBuilder::AddEdgeInfo(size_t edge, RouteEdgeInfo info)
{
	edges_to_buses_[edge] = move(info);
}

void RouterBuilder::AddStopVertice(const domain::Stop* stop, size_t vertice)
{
	stops_to_vertices_[stop] = vertice;
}

void RouterBuilder::BuildRouter(const transport_catalogue::catalogue::TransportCatalogue& catalogue) {
	graph_ptr_ = make_unique<graph::DirectedWeightedGraph<double>>(catalogue.GetStopsList().size());
	size_t i = 0;
	for (const Stop& stop : catalogue.GetStopsList()) {
		stops_to_vertices_[&stop] = i;
		++i;
	}
	const deque<Bus>& buses_list = catalogue.GetBusesList();
	constexpr double velocity_coef = 1000.0 / 60;
	for (const Bus& bus : buses_list) {
		for (size_t from = 0; from < bus.route.size() - 1; ++from) {
			int dist = 0;
			int spans = 0;
			for (size_t to = from + 1; to < bus.route.size(); ++to) {				
				if (bus.route[from] == bus.route[to]) {
					continue;
				}
				dist += catalogue.FindDistance(bus.route[to - 1], bus.route[to]);
				spans += 1;
				double weight = settings_.wait_time + dist / (settings_.velocity * velocity_coef);				
				size_t edge_id = graph_ptr_->AddEdge({ stops_to_vertices_.at(bus.route[from]),
					stops_to_vertices_.at(bus.route[to]), weight });
				edges_to_buses_[edge_id] = { bus.route[from], &bus, spans,
					settings_.wait_time, dist / (settings_.velocity * velocity_coef) };
			}
		}
		if (!bus.is_round) {
			for (size_t from = 0; from < bus.route.size() - 1; ++from) {
				int dist_reverse = 0;
				int spans = 0;
				for (size_t to = from + 1; to < bus.route.size(); ++to) {
					if (bus.route[from] == bus.route[to]) {
						continue;
					}
					dist_reverse += catalogue.FindDistance(bus.route[to], bus.route[to - 1]);
					spans += 1;
					double weight_reverse = settings_.wait_time + dist_reverse / (settings_.velocity * velocity_coef);
					size_t reverse_edge_id = graph_ptr_->AddEdge({ stops_to_vertices_.at(bus.route[to]),
						stops_to_vertices_.at(bus.route[from]), weight_reverse });
					edges_to_buses_[reverse_edge_id] = { bus.route[to], &bus, spans,
						settings_.wait_time, dist_reverse / (settings_.velocity * velocity_coef) };
				}
			}
		}
	}
	router_ptr_ = make_unique<graph::Router<double>>(*graph_ptr_);
}

OptimalRouteInfo RouterBuilder::BuildOptimalRoute(const domain::Stop* from, const domain::Stop* to) const
{
	if (from == to) {
		return { 0, {} };
	}
	optional<graph::Router<double>::RouteInfo> route_info = router_ptr_->BuildRoute(stops_to_vertices_.at(from), stops_to_vertices_.at(to));
	if (route_info == nullopt) {
		return { nullopt, {} };
	}
	vector<RouteEdgeInfo> edges_info;
	edges_info.reserve(route_info->edges.size());
	for (const auto& edge : route_info->edges) {
		edges_info.push_back(edges_to_buses_.at(edge));
	}
	return { route_info->weight, move(edges_info) };
}
