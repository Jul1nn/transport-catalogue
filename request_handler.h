#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_catalogue::catalogue::TransportCatalogue& db);

    RequestHandler(const transport_catalogue::catalogue::TransportCatalogue& db, MapRenderer* render, RouterBuilder* router, Serializator* serializator);

    // Возвращает информацию о маршруте (запрос Bus)
    transport_catalogue::catalogue::TransportCatalogue::BusInfo GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через остановку
    const std::set<std::string_view>& GetBusesByStop(const std::string_view& stop_name) const;

    void InitializeRenderer(MapRenderer*);

    void InitializeRouter(RouterBuilder*);

    void InitializeSerializator(Serializator*);

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

    //Возвращает информацию об оптимальном маршруте
    OptimalRouteInfo FindOptimalRoute(std::string_view, std::string_view) const;

    void Serialize();

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::catalogue::TransportCatalogue& db_;
    MapRenderer* renderer_;
    RouterBuilder* router_;
    Serializator* serializator_;
};
