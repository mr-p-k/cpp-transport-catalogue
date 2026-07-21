#pragma once

#include <cstdint>
#include <deque>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#include "domain.h"

namespace tc {

namespace {
template <typename T>
size_t ComputeUnique(const std::vector<T>& vec) {
    std::set<T> tmp{vec.cbegin(), vec.cend()};

    return tmp.size();
}

}  // namespace

struct StopPtrHasher;

using domain::Bus;
using domain::BusInfo;
using domain::Stop;

class TransportCatalogue final {
public:
    void AddBus(std::string_view name,
                const std::vector<std::string_view>& route, bool is_roundtrip);

    void AddStop(const Stop& stop);

    void SetDistance(std::string_view stop_a, std::string_view stop_b,
                     int dist);

    [[nodiscard]] const Bus* FindBus(std::string_view bus_name) const noexcept;

    [[nodiscard]] const Stop* FindStop(
        std::string_view stop_name) const noexcept;

    [[nodiscard]] std::optional<BusInfo> GetBusInfo(
        std::string_view bus_name) const;

    [[nodiscard]] const std::set<std::string_view>& GetBusesByStop(
        std::string_view stop_name) const;

    [[nodiscard]] int GetDistance(const Stop* stop_a_ptr,
                                  const Stop* stop_b_ptr) const noexcept;

    [[nodiscard]] std::vector<const Bus*> GetAllBusesWithStops() const;

    [[nodiscard]] std::vector<const Stop*> GetAllStopsInRoutes() const;

private:
    struct StopPtrHasher {
        std::hash<std::uintptr_t> hasher;

        size_t operator()(
            std::pair<const Stop*, const Stop*> stop_ptr_pair) const;
    };

    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;

    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::unordered_map<std::string_view, std::set<std::string_view>>
        buses_by_stop_;

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPtrHasher>
        distance_;
};

}  // namespace tc