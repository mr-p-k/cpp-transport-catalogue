#include "request_handler.h"

#include <algorithm>
#include <utility>

namespace request_handler {
/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего
 * логику, которую не хотелось бы помещать ни в transport_catalogue, ни в json
 * reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const tc::TransportCatalogue& db,
                               const renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) {
}

std::optional<BusInfo> RequestHandler::GetBusInfo(
    std::string_view bus_name) const {
    //
    const Bus* found_bus{db_.FindBus(bus_name)};
    if (nullptr == found_bus) {
        return std::nullopt;
    }

    const size_t route_size{found_bus->route.size()};

    const size_t kStopCount =
        (found_bus->is_roundtrip ? route_size : (2 * route_size) - 1);
    const size_t kUniqueStopsCount = ComputeUnique(found_bus->route);

    double geo_length = 0.0;
    int route_length = 0;

    for (auto it = found_bus->route.cbegin(); it != found_bus->route.cend();
         ++it) {  //
        if (auto next_it{std::next(it, 1)};
            next_it != found_bus->route.cend()) {  //
            geo_length += geo::ComputeDistance((*it)->coordinates,
                                               (*next_it)->coordinates);

            route_length += db_.GetDistance(*it, *next_it);
        }
    }

    if (!found_bus->is_roundtrip) {
        for (auto it = found_bus->route.crbegin();
             it != found_bus->route.crend(); ++it) {  //
            if (auto next_it{std::next(it, 1)};
                next_it != found_bus->route.crend()) {  //
                geo_length += geo::ComputeDistance((*it)->coordinates,
                                                   (*next_it)->coordinates);

                route_length += db_.GetDistance(*it, *next_it);
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

const std::set<std::string_view>& RequestHandler::GetBusesByStop(
    std::string_view stop_name) const {
    static const std::set<std::string_view> kBlank{};

    const auto& buses_by_stop = db_.GetBusesByStop();

    auto it = buses_by_stop.find(stop_name);

    return (it != buses_by_stop.cend() ? it->second : kBlank);
}

bool RequestHandler::IsStopExists(std::string_view name) const {
    return nullptr != db_.FindStop(name);
}

std::vector<const Bus*> RequestHandler::GetNonEmptySortedBuses() const {
    std::vector<const Bus*> buses;

    for (const auto& bus : db_.GetBuses()) {
        if (!bus.route.empty()) {
            buses.emplace_back(&bus);
        }
    }

    // sort by bus name
    auto comp = [](const Bus* lhs, const Bus* rhs) -> bool {
        return std::lexicographical_compare(
            lhs->name.cbegin(), lhs->name.cend(), rhs->name.cbegin(),
            rhs->name.cend());
    };

    if (!std::is_sorted(buses.cbegin(), buses.cend(), comp)) {
        std::sort(buses.begin(), buses.end(), comp);
    }

    return buses;
}

std::vector<const Stop*> RequestHandler::GetNonEmptySortedStops() const {
    std::vector<const Stop*> stops;

    const auto& buses_by_stop{db_.GetBusesByStop()};

    for (const auto& [stop_name, route] : buses_by_stop) {
        if (route.empty()) {
            continue;
        }

        const Stop* stop_ptr{db_.FindStop(stop_name)};
        if (nullptr == stop_ptr) {
            continue;
        }

        stops.emplace_back(stop_ptr);
    }

    const auto comp = [](const Stop* lhs, const Stop* rhs) -> bool {
        return std::lexicographical_compare(
            lhs->name.cbegin(), lhs->name.cend(), rhs->name.cbegin(),
            rhs->name.cend());
    };

    std::ranges::sort(stops, comp);

    return stops;
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.Render(GetNonEmptySortedBuses(), GetNonEmptySortedStops());
}

}  // namespace request_handler