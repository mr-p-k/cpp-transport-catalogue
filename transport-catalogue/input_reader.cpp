#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace reader {

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта,
 * долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double kNan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == std::string_view::npos) {
        return {kNan, kNan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat =
        std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto kStart = string.find_first_not_of(' ');
    if (kStart == std::string_view::npos) {
        return {};
    }
    return string.substr(kStart, string.find_last_not_of(' ') + 1 - kStart);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя
 * delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == std::string_view::npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos));
            !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок
 * [A,B,C,A] Для некольцевого маршрута (A-B-C-D) возвращает массив названий
 * остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != std::string_view::npos) {
        return Split(route, '>');
    }

    auto stops{Split(route, '-')};

    decltype(stops) result;
    result.reserve((stops.size() * 2) - 1);

    std::copy(stops.cbegin(), stops.cend(), std::back_inserter(result));
    std::copy(std::next(stops.crbegin()), stops.crend(),
              std::back_inserter(result));

    return result;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == std::string_view::npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(tc::TransportCatalogue& catalogue) const {
    using std::string_view_literals::operator""sv;
    using tc::Stop;

    for (const auto& el : commands_) {
        std::string_view c{el.command};

        if (c == "Stop"sv) {
            catalogue.AddStop(
                {.name{el.id}, .coordinates{ParseCoordinates(el.description)}});
        } else if (c == "Bus"sv) {
            std::vector<Stop*> stops_on_route{};

            std::vector<std::string_view> route{ParseRoute(el.description)};
            for (auto r : route) {
                if (Stop* added_stop{
                        catalogue.AddStop({.name{r}, .coordinates{}})};
                    nullptr != added_stop) {
                    stops_on_route.push_back(added_stop);
                }
            }

            if (!stops_on_route.empty()) {
                catalogue.AddBus(
                    {.name{el.id}, .route{std::move(stops_on_route)}});
            }
        }
    }
}

}  // namespace reader