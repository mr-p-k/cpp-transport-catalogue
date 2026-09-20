#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью
 * предметной области (domain) вашего приложения и не зависят от транспортного
 * справочника. Например, Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в
 * отдельный заголовочный файл может оказаться полезным, когда дело дойдёт до
 * визуализации карты маршрутов: визуализатор карты (map_renderer) можно будет
 * сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте
 * этот файл пустым.
 *
 */

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
    bool is_roundtrip;
};

struct BusInfo {
    std::string_view name;
    size_t stops;
    size_t unique_stops;
    int route_length;
    double curvature;
};

struct StopPtrHasher {
    std::hash<std::uintptr_t> hasher;

    size_t operator()(std::pair<const Stop*, const Stop*> stop_ptr_pair) const {
        const std::uintptr_t kSalt = 37;

        auto s1{reinterpret_cast<std::uintptr_t>(stop_ptr_pair.first)};
        auto s2{reinterpret_cast<std::uintptr_t>(stop_ptr_pair.second)};

        return hasher(s1 + (s2 * kSalt));
    }
};

}  // namespace domain