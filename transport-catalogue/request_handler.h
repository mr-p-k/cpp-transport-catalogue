#pragma once

#include <optional>
#include <set>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace request_handler {

using Bus = domain::Bus;

using BusInfo = domain::BusInfo;

using MapRenderer = renderer::MapRenderer;

using RouteResult = router::RouteResult;

using Stop = domain::Stop;

using TransportCatalogue = tc::TransportCatalogue;

using TransportRouter = router::TransportRouter;

class RequestHandler {
private:
    [[nodiscard]] std::vector<const Bus*> GetAllBusesWithStops() const;

    [[nodiscard]] std::vector<const Stop*> GetAllStopsInRoutes() const;

public:
    RequestHandler() = delete;

    RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer,
                   const TransportRouter& router);

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

    [[nodiscard]] svg::Document RenderMap() const;

    [[nodiscard]] std::optional<RouteResult> FindRoute(
        std::string_view from, std::string_view to) const;

private:
    const TransportCatalogue& db_;

    const MapRenderer& renderer_;

    const TransportRouter& router_;
};

}  // namespace request_handler