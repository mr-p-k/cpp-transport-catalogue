#pragma once

#include <optional>
#include <set>

#include "map_renderer.h"
#include "transport_catalogue.h"

namespace request_handler {

using domain::Bus;
using domain::BusInfo;
using domain::Stop;

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего
 * логику, которую не хотелось бы помещать ни в transport_catalogue, ни в json
 * reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика
 * запросов. Вы можете реализовать обработку запросов способом, который удобнее
 * вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON
// reader-а с другими подсистемами приложения. См. паттерн проектирования Фасад:
// https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler() = delete;

    RequestHandler(const tc::TransportCatalogue& db,
                   const renderer::MapRenderer& renderer);

    RequestHandler(const RequestHandler& other) = delete;

    RequestHandler(RequestHandler&& other) = delete;

    RequestHandler& operator=(const RequestHandler& other) = delete;

    RequestHandler& operator=(RequestHandler&& other) = delete;

    ~RequestHandler() = default;

    // Возвращает информацию о маршруте (запрос Bus)
    [[nodiscard]] std::optional<BusInfo> GetBusInfo(
        std::string_view bus_name) const;

    // Возвращает маршруты, проходящие через
    [[nodiscard]] const std::set<std::string_view>& GetBusesByStop(
        std::string_view stop_name) const;

    [[nodiscard]] bool IsStopExists(std::string_view name) const;

    [[nodiscard]] const std::deque<const Bus*> GetNonEmptyRoutes() const;

    [[nodiscard]] const std::deque<const Stop*> GetStopsWithBuses() const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    std::vector<const Bus*> GetNonEmptySortedBuses() const;

    std::vector<const Stop*> GetNonEmptySortedStops() const;

    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и
    // "Визуализатор Карты"
    const tc::TransportCatalogue& db_;

    const renderer::MapRenderer& renderer_;
};

template <typename T>
size_t ComputeUnique(const std::vector<T>& vec) {
    std::set<T> tmp{vec.cbegin(), vec.cend()};

    return tmp.size();
}

}  // namespace request_handler