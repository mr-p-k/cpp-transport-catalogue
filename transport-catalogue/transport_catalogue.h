#pragma once

#include <cstdint>
#include <deque>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "geo.h"

namespace tc {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
};

struct BusInfo {
    std::string_view name;
    size_t stops;
    size_t unique_stops;
    double route_length;
    double curvature;
};

class TransportCatalogue {
public:
    void AddStop(const Stop& stop);

    [[nodiscard]] const Stop* FindStop(
        std::string_view stop_name) const noexcept;

    void AddBus(std::string_view name,
                const std::vector<std::string_view>& route);

    [[nodiscard]] const Bus* FindBus(std::string_view bus_name) const noexcept;

    [[nodiscard]] const std::set<std::string_view>& GetStopInfo(
        std::string_view stop_name) const;

    [[nodiscard]] BusInfo GetBusInfo(std::string_view bus_name) const;

    void SetDistance(std::string_view stop_a, std::string_view stop_b,
                     int dist);

    [[nodiscard]] int GetDistance(const Stop* stop_a_ptr,
                                  const Stop* stop_b_ptr) const noexcept;

private:
    struct StopPtrHasher {
        size_t operator()(
            std::pair<const Stop*, const Stop*> stop_ptr_pair) const;

        std::hash<std::uintptr_t> hasher;
    };

    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;

    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::unordered_map<std::string_view, std::set<std::string_view>>
        buses_throught_stop_;

    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPtrHasher>
        distance_;
};

}  // namespace tc