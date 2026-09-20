#include "transport_router.h"

namespace router {

std::optional<size_t> TransportRouter::GetStopIndex(const Stop* stop) const {
    //
    if (const auto kIt = stop_to_index_.find(stop);
        kIt != stop_to_index_.cend()) {
        return kIt->second;
    }

    return std::nullopt;
}

TransportRouter::TransportRouter(RoutingSettings settings,
                                 const RequestHandler& rhandler)
    : settings_(settings), rhandler_(rhandler) {
    //
    MakeGraph();

    router_ = std::make_unique<Router>(graph_);
}

void TransportRouter::MakeGraph() {
    const auto& all_stops{rhandler_.GetAllStops()};

    // increase the storage twice, for Wait and Bus edges
    graph_ = DirectedWeightedGraph(all_stops.size() * 2);
    edge_descriptions_.reserve(all_stops.size() * 2);

    // fill wait edges
    AddWaitEdges(all_stops.cbegin(), all_stops.cend());

    // fill bus edges
    const auto& all_buses{rhandler_.GetAllBuses()};

    for (const auto& bus : all_buses) {
        const auto& bus_stops{bus.route};

        AddBusEdges(bus_stops.cbegin(), bus_stops.cend(), bus.name);

        if (!bus.is_roundtrip) {
            AddBusEdges(bus_stops.crbegin(), bus_stops.crend(), bus.name);
        }
    }
}

std::optional<RouteResult> TransportRouter::FindRoute(
    std::string_view from, std::string_view to) const {
    //
    const Stop* from_ptr = rhandler_.FindStop(from);
    if (nullptr == from_ptr) {
        return std::nullopt;
    }

    const Stop* to_ptr = rhandler_.FindStop(to);
    if (nullptr == to_ptr) {
        return std::nullopt;
    }

    const auto kFromIdx = GetStopIndex(from_ptr);
    if (!kFromIdx.has_value()) {
        return std::nullopt;
    }

    const auto kToIdx = GetStopIndex(to_ptr);
    if (!kToIdx.has_value()) {
        return std::nullopt;
    }

    const VertexId kVertexFrom = kFromIdx.value() * 2;
    const VertexId kVertexTo = kToIdx.value() * 2;

    auto route_info = router_->BuildRoute(kVertexFrom, kVertexTo);
    if (!route_info.has_value()) {
        return std::nullopt;
    }

    RouteResult result;
    result.total_time = route_info->weight.time;

    for (auto edge_id : route_info->edges) {
        result.items.emplace_back(edge_descriptions_[edge_id]);
    }

    return result;
}

}  // namespace router