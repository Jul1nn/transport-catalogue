#include "request_handler.h"

#include <stdexcept>

using namespace std;

RequestHandler::RequestHandler(const transport_catalogue::catalogue::TransportCatalogue& db)
    :db_(db) {}

RequestHandler::RequestHandler(const transport_catalogue::catalogue::TransportCatalogue& db, MapRenderer* render, RouterBuilder* router, 
    Serializator* serializator)
    :db_(db), renderer_(render), router_(router), serializator_(serializator) {}

transport_catalogue::catalogue::TransportCatalogue::BusInfo RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    return db_.GetBusInfo(bus_name);
}

const std::set<std::string_view>& RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return db_.GetStopInfo(stop_name);
}

void RequestHandler::InitializeRenderer(MapRenderer* renderer) {
    renderer_ = renderer;
    renderer_->IdentifyBusMapping(db_.GetBusesList());
    renderer_->IdentifyStopMapping(db_.GetStopsList());
}

svg::Document RequestHandler::RenderMap() const {
    if (!renderer_) {
        throw invalid_argument("Renderer is not initialized"s);
    }
    return renderer_->MakeMap();
}

void RequestHandler::InitializeRouter(RouterBuilder* router) {    
    router_ = router;
    router_->BuildRouter(db_);
}

OptimalRouteInfo RequestHandler::FindOptimalRoute(string_view from, string_view to) const
{    
    if (!router_) {
        throw invalid_argument("Router is not initialized"s);
    }
    return router_->BuildOptimalRoute(db_.FindStop(from), db_.FindStop(to));
}

void RequestHandler::InitializeSerializator(Serializator* serie) {
    serializator_ = serie;
}

void RequestHandler::Serialize() {
    if (!serializator_) {
        throw invalid_argument("Serializator is not initialized"s);
    }
    serializator_->Serialize(db_, *renderer_, *router_);
}
