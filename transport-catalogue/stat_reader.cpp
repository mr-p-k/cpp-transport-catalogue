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
    if ("Bus"sv == command) {
        const auto kBusInfo{transport_catalogue.GetBusInfo(query)};

        output << "Bus "s << query << ": "s;

        if (kBusInfo.name.empty()) {
            output << "not found"s;
        } else {
            const int kDoublePrecision = 6;
            const auto kOldPrecision{output.precision()};

            output << kBusInfo.stops << " stops on route, "s
                   << kBusInfo.unique_stops << " unique stops, "s
                   << std::setprecision(kDoublePrecision)
                   << kBusInfo.route_length << " route length"s;

            output << std::setprecision(static_cast<int>(kOldPrecision));
        }
    } else if ("Stop"sv == command) {
        const auto kStopInfo{transport_catalogue.GetStopInfo(query)};
        output << "Stop "s << query << ": "s;

        if (kStopInfo.name.empty()) {
            output << "not found"s;
        } else if (kStopInfo.buses.empty()) {
            output << "no buses"s;
        } else {
            output << "buses"s;

            for (const auto kBus : kStopInfo.buses) {
                output << " "s << kBus;
            }
        }
    }

    output << std::endl;
}

}  // namespace stat