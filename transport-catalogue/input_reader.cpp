#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
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
        return {.lat = kNan, .lng = kNan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    const double kLat =
        std::stod(std::string(str.substr(not_space, comma - not_space)));
    const double kLng = std::stod(std::string(str.substr(not_space2)));

    return {.lat = kLat, .lng = kLng};
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

std::vector<std::pair<std::string_view, int>> ParseDistance(
    std::string_view str) {
    using std::string_view_literals::operator""sv;
    // std::clog << "ParseDistance arg: str: " << str << std::endl;

    auto substrs{Split(str, ',')};
    if (substrs.empty()) {
        return {};
    }

    const auto kBufSize = 100;  // по условию
    std::vector<std::pair<std::string_view, int>> distances;
    distances.reserve(kBufSize);

    for (auto s : substrs) {
        auto suffix_m = s.find('m');

        auto skip_to_begin = s.find_first_not_of(' ', suffix_m + 1);
        auto skip_to_end = s.find('o', skip_to_begin + 1);

        auto stop_name_begin = s.find_first_not_of(' ', skip_to_end + 1);

        if (suffix_m == std::string_view::npos ||
            stop_name_begin == std::string_view ::npos) {
            continue;
        }

        auto distance = std::stoi(std::string(s.substr(0, suffix_m)));
        auto stop_name = s.substr(stop_name_begin);

        if (distances.size() >= kBufSize) {
            break;
        }

        distances.emplace_back(stop_name, distance);
    }

    return distances;
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

    // std::copy(stops.cbegin(), stops.cend(), std::back_inserter(result));
    std::ranges::copy(stops, std::back_inserter(result));
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

    return {.command = std::string(line.substr(0, space_pos)),
            .id = std::string(line.substr(not_space, colon_pos - not_space)),
            .description = std::string(line.substr(colon_pos + 1))};
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

    // add stop and coordinates
    for (const auto& el : commands_) {
        const std::string_view kCommand{el.command};

        if (kCommand == "Stop"sv) {
            catalogue.AddStop(
                {.name{el.id}, .coordinates{ParseCoordinates(el.description)}});
        }
    }

    // add distances between stops
    for (const auto& el : commands_) {
        const std::string_view kCommand{el.command};

        if (kCommand == "Stop"sv) {
            auto distances_for_stop{ParseDistance(el.description)};
            if (!distances_for_stop.empty()) {
                catalogue.SetDistance(el.id, distances_for_stop);
            }
        }
    }

    // add routes
    for (const auto& el : commands_) {
        const std::string_view kCommand{el.command};

        if (kCommand == "Bus"sv) {
            const std::vector<std::string_view> kRoute{
                ParseRoute(el.description)};

            catalogue.AddBus(el.id, kRoute);
        }
    }
}

}  // namespace reader