#pragma once

#include <optional>
#include <set>

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace request_handler {

using domain::Bus;
using domain::BusInfo;
using domain::Stop;

class RequestHandler {
public:
    RequestHandler() = delete;

    RequestHandler(const tc::TransportCatalogue& db,
                   const renderer::MapRenderer& renderer);

    RequestHandler(const RequestHandler& other) = delete;

    RequestHandler(RequestHandler&& other) = delete;

    RequestHandler& operator=(const RequestHandler& other) = delete;

    RequestHandler& operator=(RequestHandler&& other) = delete;

    ~RequestHandler() = default;

    [[nodiscard]] std::optional<BusInfo> GetBusInfo(
        std::string_view bus_name) const;

    [[nodiscard]] const std::set<std::string_view>& GetBusesByStop(
        std::string_view stop_name) const;

    [[nodiscard]] bool IsStopExists(std::string_view name) const;

    [[nodiscard]] const std::deque<const Bus*> GetNonEmptyRoutes() const;

    [[nodiscard]] const std::deque<const Stop*> GetStopsWithBuses() const;

    svg::Document RenderMap() const;

private:
    std::vector<const Bus*> GetAllBusesWithStops() const;

    std::vector<const Stop*> GetAllStopsInRoutes() const;

    const tc::TransportCatalogue& db_;

    const renderer::MapRenderer& renderer_;
};

}  // namespace request_handler