#include "transport_catalogue.h"

#include <cstdint>
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

const std::unordered_map<std::string_view, std::set<std::string_view>>&
TransportCatalogue::GetBusesByStop() const noexcept {
    return buses_by_stop_;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const noexcept {
    return buses_;
}

size_t TransportCatalogue::StopPtrHasher::operator()(
    const std::pair<const Stop*, const Stop*> stop_ptr_pair) const {
    const std::uintptr_t kSalt = 37;

    auto s1{reinterpret_cast<std::uintptr_t>(stop_ptr_pair.first)};
    auto s2{reinterpret_cast<std::uintptr_t>(stop_ptr_pair.second)};

    return hasher(s1 + (s2 * kSalt));
}

size_t TransportCatalogue::GetStopsSize() const noexcept {
    return stops_.size();
}

size_t TransportCatalogue::GetBusesSize() const noexcept {
    return buses_.size();
}

}  // namespace tc