#include "transport_catalogue.h"

#include <set>
#include <utility>

namespace tc {

void TransportCatalogue::AddStop(const Stop& stop) {
    if (const Stop* found_stop{FindStop(stop.name)}; nullptr != found_stop) {
        return;
    }

    stops_.push_back(stop);
    const Stop* added_stop = &stops_.back();

    stopname_to_stop_[added_stop->name] = added_stop;
}

const Stop* TransportCatalogue::FindStop(
    std::string_view stop_name) const noexcept {
    auto it{stopname_to_stop_.find(stop_name)};

    return (it != stopname_to_stop_.cend() ? it->second : nullptr);
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}

const std::unordered_map<std::pair<const Stop*, const Stop*>, int,
                         StopPtrHasher>&
TransportCatalogue::GetDistances() const {
    return distances_;
}

void TransportCatalogue::AddBus(std::string_view name,
                                const std::vector<std::string_view>& route,
                                bool is_roundtrip) {
    if (const Bus* found_bus{FindBus(name)}; nullptr != found_bus) {
        return;
    }

    buses_.push_back(Bus{.name{name}, .route{}, .is_roundtrip = is_roundtrip});
    Bus& added_bus{buses_.back()};
    busname_to_bus_[added_bus.name] = &added_bus;

    for (const auto& r : route) {
        const Stop* found_stop{FindStop(r)};
        if (nullptr == found_stop) {
            continue;
        }

        buses_by_stop_[found_stop->name].insert(added_bus.name);

        added_bus.route.push_back(found_stop);
    }
}

const Bus* TransportCatalogue::FindBus(
    std::string_view bus_name) const noexcept {
    auto it{busname_to_bus_.find(bus_name)};

    return (it != busname_to_bus_.cend() ? it->second : nullptr);
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(
    std::string_view bus_name) const {
    //
    const Bus* found_bus{FindBus(bus_name)};
    if (nullptr == found_bus) {
        return std::nullopt;
    }

    const size_t kRouteSize{found_bus->route.size()};

    const size_t kStopCount =
        (found_bus->is_roundtrip ? kRouteSize : (2 * kRouteSize) - 1);
    const size_t kUniqueStopsCount = ComputeUnique(found_bus->route);

    double geo_length = 0.0;
    int route_length = 0;

    for (auto it = found_bus->route.cbegin(); it != found_bus->route.cend();
         ++it) {
        //
        if (auto next_it{std::next(it, 1)};
            next_it != found_bus->route.cend()) {
            //
            geo_length += geo::ComputeDistance((*it)->coordinates,
                                               (*next_it)->coordinates);

            route_length += GetDistance(*it, *next_it).value_or(0);
        }
    }

    if (!found_bus->is_roundtrip) {
        for (auto it = found_bus->route.crbegin();
             it != found_bus->route.crend(); ++it) {
            //
            if (auto next_it{std::next(it, 1)};
                next_it != found_bus->route.crend()) {
                //
                geo_length += geo::ComputeDistance((*it)->coordinates,
                                                   (*next_it)->coordinates);

                route_length += GetDistance(*it, *next_it).value_or(0);
            }
        }
    }

    return BusInfo{
        .name{found_bus->name},
        .stops = kStopCount,
        .unique_stops = kUniqueStopsCount,
        .route_length = route_length,
        .curvature = static_cast<double>(route_length) / geo_length,
    };
}

const std::set<std::string_view>& TransportCatalogue::GetBusesByStop(
    std::string_view stop_name) const {
    //
    static const std::set<std::string_view> kBlank{};

    auto it = buses_by_stop_.find(stop_name);

    return (it != buses_by_stop_.cend() ? it->second : kBlank);
}

void TransportCatalogue::SetDistance(std::string_view stop_a,
                                     std::string_view stop_b, int dist) {
    const Stop* stop_a_ptr{FindStop(stop_a)};
    if (nullptr == stop_a_ptr) {
        return;
    }

    const Stop* stop_b_ptr{FindStop(stop_b)};
    if (nullptr == stop_b_ptr) {
        return;
    }

    distances_.insert({{stop_a_ptr, stop_b_ptr}, dist});
}

std::optional<int> TransportCatalogue::GetDistance(
    const Stop* stop_a_ptr, const Stop* stop_b_ptr) const noexcept {
    auto found_a_to_b{distances_.find({stop_a_ptr, stop_b_ptr})};

    if (found_a_to_b == distances_.cend()) {
        auto found_b_to_a{distances_.find({stop_b_ptr, stop_a_ptr})};

        return (found_b_to_a != distances_.cend()) ? found_b_to_a->second : 0;
    }

    return found_a_to_b->second;
}

std::vector<const Bus*> TransportCatalogue::GetAllBusesWithStops() const {
    std::vector<const Bus*> buses_with_stops;

    for (const auto& bus : buses_) {
        if (!bus.route.empty()) {
            buses_with_stops.emplace_back(&bus);
        }
    }

    return buses_with_stops;
}

std::vector<const Stop*> TransportCatalogue::GetAllStopsInRoutes() const {
    std::vector<const Stop*> stops;

    for (const auto& [stop_name, route] : buses_by_stop_) {
        if (route.empty()) {
            continue;
        }

        const Stop* stop_ptr{FindStop(stop_name)};
        if (nullptr == stop_ptr) {
            continue;
        }

        stops.emplace_back(stop_ptr);
    }

    return stops;
}

}  // namespace tc