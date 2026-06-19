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
    std::vector<Stop*> route;
};

struct StopInfo {
    std::string_view name;
    std::set<std::string_view> buses;
};

struct BusInfo {
    std::string_view name;
    size_t stops;
    size_t unique_stops;
    double route_length;
};

class TransportCatalogue {
public:
    [[maybe_unused]] Stop* AddStop(Stop&& stop);
    [[nodiscard]] Stop* FindStop(std::string_view stop_name) const noexcept;

    [[maybe_unused]] Bus* AddBus(Bus&& bus);
    [[nodiscard]] Bus* FindBus(std::string_view bus_name) const noexcept;

    [[nodiscard]] StopInfo GetStopInfo(std::string_view stop_name) const;
    [[nodiscard]] BusInfo GetBusInfo(std::string_view bus_name) const;

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;

    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
};

}  // namespace tc