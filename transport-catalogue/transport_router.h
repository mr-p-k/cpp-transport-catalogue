#pragma once

#include <memory>
#include <vector>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace router {

struct Time;

using DirectedWeightedGraph = graph::DirectedWeightedGraph<Time>;

using Router = graph::Router<Time>;

using Stop = domain::Stop;

using TransportCatalogue = tc::TransportCatalogue;

using VertexId = graph::VertexId;

struct RoutingSettings {
    int bus_wait_time;

    int bus_velocity;
};

struct Time {
    double time = 0.0;

    constexpr explicit Time(int t) : time(static_cast<double>(t)) {
    }

    constexpr explicit Time(double t) : time(t) {
    }

    constexpr Time() = default;
};

constexpr Time operator+(Time lhs, Time rhs) {
    return Time{lhs.time + rhs.time};
}

constexpr auto operator<=>(Time lhs, Time rhs) {
    return lhs.time <=> rhs.time;
}

constexpr bool operator==(Time lhs, Time rhs) {
    return lhs.time == rhs.time;
}

struct EdgeInfo {
    std::string type;
    std::string_view name;
    Time time;
    int span_count = 0;
};

struct RouteResult {
    double total_time = 0.0;
    std::vector<EdgeInfo> items;
};

class TransportRouter {
private:
    std::optional<size_t> GetStopIndex(const Stop* stop) const;

    template <typename InputIt>
    void AddBusEdges(InputIt first, InputIt last, std::string_view bus_name);

    template <typename InputIt>
    void AddWaitEdges(InputIt first, InputIt last);

public:
    explicit TransportRouter(RoutingSettings settings,
                             const TransportCatalogue& tc);

    void MakeGraph();

    std::optional<RouteResult> FindRoute(std::string_view from,
                                         std::string_view to) const;

private:
    RoutingSettings settings_;

    const TransportCatalogue& tc_;

    std::unordered_map<const Stop*, size_t> stop_to_index_;

    DirectedWeightedGraph graph_;

    std::unique_ptr<Router> router_;

    std::vector<EdgeInfo> edge_descriptions_;
};

template <typename InputIt>
void TransportRouter::AddBusEdges(InputIt first, InputIt last,
                                  std::string_view bus_name) {
    const auto kSize = std::distance(first, last);

    if (kSize < 2) {
        return;
    }

    for (auto it_i = first; it_i != std::prev(last); ++it_i) {
        double accumulated_time = 0.0;

        for (auto it_j = std::next(it_i); it_j != last; ++it_j) {
            const Stop* from_ptr = *std::prev(it_j);
            const Stop* to_ptr = *it_j;

            if (nullptr == from_ptr || nullptr == to_ptr) {
                continue;
            }

            const auto kDistance = tc_.GetDistance(from_ptr, to_ptr);

            accumulated_time +=
                static_cast<double>(kDistance.value_or(0.0)) /
                (settings_.bus_velocity * 50.0 / 3.0);  // convert to m/min

            const auto kStopIdxI = GetStopIndex(*it_i);
            const auto kStopIdxJ = GetStopIndex(*it_j);

            if (!kStopIdxI.has_value() || !kStopIdxJ.has_value()) {
                continue;
            }

            if (kStopIdxI.value() == kStopIdxJ.value() &&
                0.0 == accumulated_time) {
                continue;
            }

            const VertexId kFromVertexId = (kStopIdxI.value() * 2) + 1;
            const VertexId kToVertexId = kStopIdxJ.value() * 2;

            graph_.AddEdge({.from = kFromVertexId,
                            .to = kToVertexId,
                            .weight = Time{accumulated_time}});

            using std::string_literals::operator""s;
            edge_descriptions_.push_back(EdgeInfo{
                .type = "Bus"s,
                .name = bus_name,
                .time = Time{accumulated_time},
                .span_count = static_cast<int>(std::distance(it_i, it_j))});
        }
    }
}

template <typename InputIt>
void TransportRouter::AddWaitEdges(InputIt first, InputIt last) {
    using std::string_literals::operator""s;

    for (auto it_i = first; it_i != last; ++it_i) {
        const auto kIdx = std::distance(first, it_i);

        stop_to_index_[std::addressof(*it_i)] = kIdx;

        const VertexId kWaitStart = kIdx * 2;      // arrive
        const VertexId kWaitEnd = (kIdx * 2) + 1;  // depart

        graph_.AddEdge({.from = kWaitStart,
                        .to = kWaitEnd,
                        .weight = Time{settings_.bus_wait_time}});

        edge_descriptions_.push_back(
            EdgeInfo{.type = "Wait"s,
                     .name = it_i->name,
                     .time = Time{settings_.bus_wait_time},
                     .span_count = 0});
    }
}

}  // namespace router