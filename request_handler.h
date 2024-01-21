#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

class RequestHandler {
public:

    RequestHandler(const transport_catalogue::catalogue::TransportCatalogue& db);

    RequestHandler(const transport_catalogue::catalogue::TransportCatalogue& db, MapRenderer* render, RouterBuilder* router, Serializator* serializator);

    transport_catalogue::catalogue::TransportCatalogue::BusInfo GetBusStat(const std::string_view& bus_name) const;

    const std::set<std::string_view>& GetBusesByStop(const std::string_view& stop_name) const;

    void InitializeRenderer(MapRenderer*);

    void InitializeRouter(RouterBuilder*);

    void InitializeSerializator(Serializator*);

    svg::Document RenderMap() const;

    OptimalRouteInfo FindOptimalRoute(std::string_view, std::string_view) const;

    void Serialize();

private:

    const transport_catalogue::catalogue::TransportCatalogue& db_;
    MapRenderer* renderer_;
    RouterBuilder* router_;
    Serializator* serializator_;
};
