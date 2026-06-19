#include "stat_reader.h"

#include <iomanip>
#include <iostream>

namespace stat {

void ParseAndPrintStat(const tc::TransportCatalogue& transport_catalogue,
                       std::string_view request, std::ostream& output) {
    if (request.empty()) {
        return;
    }

    size_t space_pos = request.find(' ');

    std::string_view command = request.substr(0, space_pos++);
    std::string_view query = request.substr(space_pos);

    using std::string_view_literals::operator""sv;
    using std::string_literals::operator""s;
    output << command << " "s << query << ": "s;
    if ("Stop"sv == command) {
        PrintStopInfo(transport_catalogue, query, output);
    } else if ("Bus"sv == command) {
        PrintBusInfo(transport_catalogue, query, output);
    }

    output << std::endl;
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
               << " route length"s;

        output << std::setprecision(static_cast<int>(kOldPrecision));
    }
}

}  // namespace stat