#include "transport_catalogue.h"

#include <set>

namespace tc {

template <typename T>
size_t ComputeUnique(const std::vector<T>& vec) {
    std::set<T> counter(vec.cbegin(), vec.cend());

    return counter.size();
}

void TransportCatalogue::AddStop(const Stop& stop) {
    if (const Stop* found_stop{FindStop(stop.name)}; nullptr != found_stop) {
        return;
    }

    stops_.push_back(std::move(stop));
    const Stop* added_stop = &stops_.back();

    stopname_to_stop_[added_stop->name] = added_stop;
}

const Stop* TransportCatalogue::FindStop(
    std::string_view stop_name) const noexcept {
    auto it{stopname_to_stop_.find(stop_name)};

    return (it != stopname_to_stop_.cend() ? it->second : nullptr);
}

void TransportCatalogue::AddBus(std::string_view name,
                                const std::vector<std::string_view>& route) {
    if (const Bus* found_bus{FindBus(name)}; nullptr != found_bus) {
        return;
    }

    buses_.push_back(Bus{.name{name}, .route{}});
    Bus& added_bus{buses_.back()};
    busname_to_bus_[added_bus.name] = &added_bus;

    for (const auto& r : route) {
        const Stop* found_stop{FindStop(r)};
        if (nullptr == found_stop) {
            continue;
        }

        buses_throught_stop_[found_stop->name].insert(added_bus.name);

        added_bus.route.push_back(found_stop);
    }
}

const Bus* TransportCatalogue::FindBus(
    std::string_view bus_name) const noexcept {
    auto it{busname_to_bus_.find(bus_name)};

    return (it != busname_to_bus_.cend() ? it->second : nullptr);
}

const std::set<std::string_view>& TransportCatalogue::GetStopInfo(
    std::string_view stop_name) const {
    static const std::set<std::string_view> kBlank{};

    auto it = buses_throught_stop_.find(stop_name);

    return (it != buses_throught_stop_.cend() ? it->second : kBlank);
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

}  // namespace tc