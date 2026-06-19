#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace stat {

void ParseAndPrintStat(const tc::TransportCatalogue& transport_catalogue,
                       std::string_view request, std::ostream& output);

void PrintStopInfo(const tc::TransportCatalogue& transport_catalogue,
                   std::string_view query, std::ostream& output);

void PrintBusInfo(const tc::TransportCatalogue& transport_catalogue,
                  std::string_view query, std::ostream& output);

}  // namespace stat