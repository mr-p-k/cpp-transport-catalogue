#pragma once

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

private:
    std::deque<Stop> stops_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;

    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::unordered_map<std::string_view, std::set<std::string_view>>
        buses_throught_stop_;
};

}  // namespace tc