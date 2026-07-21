#include "request_handler.h"


namespace request_handler {

RequestHandler::RequestHandler(const tc::TransportCatalogue& db,
                               const renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) {
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

}  // namespace request_handler