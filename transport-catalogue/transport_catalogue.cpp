#include "transport_catalogue.h"

#include <algorithm>
#include <set>

namespace tc {

template <typename T>
size_t ComputeUnique(const std::vector<T>& vec) {
    std::set<T> counter(vec.cbegin(), vec.cend());

    return counter.size();
}

Stop* TransportCatalogue::AddStop(Stop&& stop) {
    if (Stop* found_stop{FindStop(stop.name)}; nullptr != found_stop) {
        if (auto& coord{found_stop->coordinates};
            (0 == coord.lat && 0 == coord.lng)) {
            coord = std::move(stop.coordinates);
        }

        return found_stop;
    }

    stops_.push_back(std::move(stop));
    Stop* added_stop = &stops_.back();

    stopname_to_stop_[added_stop->name] = added_stop;

    return added_stop;
}

Stop* TransportCatalogue::FindStop(std::string_view stop_name) const noexcept {
    auto it{stopname_to_stop_.find(stop_name)};

    return (it != stopname_to_stop_.cend() ? it->second : nullptr);
}

Bus* TransportCatalogue::AddBus(Bus&& bus) {
    if (Bus* found_bus{FindBus(bus.name)}; nullptr != found_bus) {
        return found_bus;
    }

    buses_.push_back(std::move(bus));
    Bus* added_bus = &buses_.back();

    busname_to_bus_[added_bus->name] = added_bus;

    return added_bus;
}

Bus* TransportCatalogue::FindBus(std::string_view bus_name) const noexcept {
    auto it{busname_to_bus_.find(bus_name)};

    return (it != busname_to_bus_.cend() ? it->second : nullptr);
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
    const Bus* found_bus{FindBus(bus_name)};

    if (nullptr == found_bus) {
        return {};
    }

    const size_t kStopCount = found_bus->route.size();
    const size_t kUniqueStopsCount = ComputeUnique(found_bus->route);

    double length = 0.0;

    for (int i = 1; i < kStopCount; ++i) {
        length += ComputeDistance(found_bus->route[i - 1]->coordinates,
                                  found_bus->route[i]->coordinates);
    }

    return {.name{found_bus->name},
            .stops = kStopCount,
            .unique_stops = kUniqueStopsCount,
            .route_length = length};
}

StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
    const Stop* found_stop{FindStop(stop_name)};

    if (nullptr == found_stop) {
        return {};
    }

    std::set<std::string_view> buses_throught_stop;
    for (auto it{buses_.cbegin()}; it != buses_.cend(); ++it) {
        auto stop_in_route{std::find_if(it->route.cbegin(), it->route.cend(),
                                        [&](const Stop* stop) -> bool {
                                            return found_stop->name ==
                                                   stop->name;
                                        })};

        if (stop_in_route != it->route.cend()) {
            buses_throught_stop.insert(it->name);
        }
    }

    return {.name{found_stop->name}, .buses{std::move(buses_throught_stop)}};
}

}  // namespace tc