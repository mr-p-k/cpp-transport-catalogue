#include "request_handler.h"

namespace request_handler {

RequestHandler::RequestHandler(const TransportCatalogue& db,
                               const MapRenderer& renderer,
                               const TransportRouter& router)
    : db_(db), renderer_(renderer), router_(router) {
}

std::optional<BusInfo> RequestHandler::GetBusInfo(
    std::string_view bus_name) const {
    //
    return db_.GetBusInfo(bus_name);
}

const std::set<std::string_view>& RequestHandler::GetBusesByStop(
    std::string_view stop_name) const {
    //
    return db_.GetBusesByStop(stop_name);
}

bool RequestHandler::IsStopExists(std::string_view name) const {
    return nullptr != db_.FindStop(name);
}

std::vector<const Bus*> RequestHandler::GetAllBusesWithStops() const {
    return db_.GetAllBusesWithStops();
}

std::vector<const Stop*> RequestHandler::GetAllStopsInRoutes() const {
    return db_.GetAllStopsInRoutes();
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render(GetAllBusesWithStops(), GetAllStopsInRoutes());
}

std::optional<RouteResult> RequestHandler::FindRoute(
    std::string_view from, std::string_view to) const {
    return router_.FindRoute(from, to);
}

}  // namespace request_handler