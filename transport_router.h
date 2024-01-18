#pragma once
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"
#include <memory>

#include <map>

struct RoutingSettings {
	int wait_time = 0;
	double velocity = 0.0;
};

struct RouteEdgeInfo {
	const domain::Stop* start;
	const domain::Bus* bus;
	int spans = 0;
	int item_wait = 0;
	double item_bus = 0.0;
};

struct OptimalRouteInfo {
	std::optional<double> sum_weight;
	std::vector<RouteEdgeInfo> edges_info;
};

class RouterBuilder {
public:
	RouterBuilder() = default;

	RouterBuilder(RoutingSettings);

	void InitializeGraph(graph::DirectedWeightedGraph<double>*);

	void InitializeRouter(graph::Router<double>*);

	const RoutingSettings& GetSettings() const;

	const graph::Router<double>& GetRouter() const;

	const graph::DirectedWeightedGraph<double>& GetGraph();

	const std::unordered_map<size_t, RouteEdgeInfo>& GetEdgesInfo() const;

	void AddEdgeInfo(size_t, RouteEdgeInfo);

	void AddStopVertice(const domain::Stop*, size_t);

	//Создание маршрутизатора
	void BuildRouter(const transport_catalogue::catalogue::TransportCatalogue&);

	//Построение оптимального маршрута
	OptimalRouteInfo BuildOptimalRoute(const domain::Stop*, const domain::Stop*) const;

private:
	RoutingSettings settings_;
	std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_ptr_;
	std::unique_ptr<graph::Router<double>> router_ptr_;
	std::unordered_map<const domain::Stop*, size_t> stops_to_vertices_;
	std::unordered_map<size_t, RouteEdgeInfo> edges_to_buses_;

};
