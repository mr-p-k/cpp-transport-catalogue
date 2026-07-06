#include "stat_reader.h"

#include <iomanip>
#include <iostream>

namespace stat {

void PrintStopInfo(const tc::TransportCatalogue& transport_catalogue,
                   std::string_view query, std::ostream& output);

void PrintBusInfo(const tc::TransportCatalogue& transport_catalogue,
                  std::string_view query, std::ostream& output);

void ParseAndPrintStat(const tc::TransportCatalogue& transport_catalogue,
                       std::string_view request, std::ostream& output) {
    if (request.empty()) {
        return;
    }

    size_t space_pos = request.find(' ');

    const std::string_view kCommand = request.substr(0, space_pos++);
    const std::string_view kQuery = request.substr(space_pos);

    using std::string_view_literals::operator""sv;
    using std::string_literals::operator""s;
    output << kCommand << " "s << kQuery << ": "s;
    if ("Stop"sv == kCommand) {
        PrintStopInfo(transport_catalogue, kQuery, output);
    } else if ("Bus"sv == kCommand) {
        PrintBusInfo(transport_catalogue, kQuery, output);
    }

    output << '\n';
}

void PrintStopInfo(const tc::TransportCatalogue& transport_catalogue,
                   std::string_view query, std::ostream& output) {
    using std::string_literals::operator""s;

    if (const auto* found_stop{transport_catalogue.FindStop(query)};
        nullptr == found_stop) {
        output << "not found"s;

        return;
    }

    const auto& stop_info{transport_catalogue.GetStopInfo(query)};
    if (stop_info.empty()) {
        output << "no buses"s;
    } else {
        output << "buses"s;

        for (auto busname : stop_info) {
            output << " "s << busname;
        }
    }
}

void PrintBusInfo(const tc::TransportCatalogue& transport_catalogue,
                  std::string_view query, std::ostream& output) {
    const auto kBusInfo{transport_catalogue.GetBusInfo(query)};

    using std::string_literals::operator""s;
    if (kBusInfo.name.empty()) {
        output << "not found"s;
    } else {
        const int kDoublePrecision = 6;
        const auto kOldPrecision{output.precision()};

        output << kBusInfo.stops << " stops on route, "s
               << kBusInfo.unique_stops << " unique stops, "s
               << std::setprecision(kDoublePrecision) << kBusInfo.route_length
               << " route length, "s << kBusInfo.curvature << " curvature";

        output << std::setprecision(static_cast<int>(kOldPrecision));
    }
}

}  // namespace stat