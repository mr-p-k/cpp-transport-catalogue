#include "transport_catalogue.h"

#include <cstdint>
#include <set>
#include <utility>

namespace tc {

size_t TransportCatalogue::StopPtrHasher::operator()(
    const std::pair<const Stop*, const Stop*> stop_ptr_pair) const {
    const std::uintptr_t kSalt = 37;

    auto s1{reinterpret_cast<std::uintptr_t>(stop_ptr_pair.first)};
    auto s2{reinterpret_cast<std::uintptr_t>(stop_ptr_pair.second)};

    return hasher(s1 + (s2 * kSalt));
}

template <typename T>
size_t ComputeUnique(const std::vector<T>& vec) {
    const std::set<T> kCounter(vec.cbegin(), vec.cend());

    return kCounter.size();
}

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

    double geo_length = 0.0;
    for (int i = 1; std::cmp_less(i, kStopCount); ++i) {
        geo_length += ComputeDistance(found_bus->route[i - 1]->coordinates,
                                      found_bus->route[i]->coordinates);
    }

    double route_length = 0.0;
    for (int i = 1; std::cmp_less(i, kStopCount); ++i) {
        const int kDist =
            GetDistance(found_bus->route[i - 1], found_bus->route[i]);

        route_length += static_cast<double>(kDist);
    }

    return {
        .name{found_bus->name},
        .stops = kStopCount,
        .unique_stops = kUniqueStopsCount,
        .route_length = route_length,
        .curvature = route_length / geo_length,
    };
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

    distance_.insert({{stop_a_ptr, stop_b_ptr}, dist});
}

int TransportCatalogue::GetDistance(const Stop* stop_a_ptr,
                                    const Stop* stop_b_ptr) const noexcept {
    auto found_a_to_b{distance_.find({stop_a_ptr, stop_b_ptr})};

    if (found_a_to_b == distance_.cend()) {
        auto found_b_to_a{distance_.find({stop_b_ptr, stop_a_ptr})};

        return (found_b_to_a != distance_.cend()) ? found_b_to_a->second : 0;
    }

    return found_a_to_b->second;

    return 0;
}

}  // namespace tc