#include "json_reader.h"

#include <algorithm>
#include <cassert>
#include <sstream>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из
 * JSON, а также код обработки запросов к базе и формирование массива ответов в
 * формате JSON
 */

namespace json_reader {

std::vector<std::pair<std::string_view, int>> ParseDistance(
    const Node& distances) {
    const auto& distances_map{distances.AsMap()};
    std::vector<std::pair<std::string_view, int>> result;
    result.reserve(distances_map.size());

    auto transformer =
        [](const auto& dist) -> std::pair<std::string_view, int> {
        return {dist.first, dist.second.AsInt()};
    };

    std::ranges::transform(distances_map, std::back_inserter(result),
                           transformer);

    return result;
}

std::vector<std::string_view> ParseRoute(const Node& route) {
    using std::string_literals::operator""s;

    const auto& route_map{route.AsMap()};

    const auto& stops_arr{route_map.at("stops"s).AsArray()};

    std::vector<std::string_view> result;

    result.reserve(stops_arr.size());

    auto transformer = [](const auto& stop) -> std::string_view {
        return stop.AsString();
    };

    std::ranges::transform(stops_arr, std::back_inserter(result), transformer);

    return result;
}

renderer::Color ParseColor(const Node& node) {
    renderer::Color color;
    if (node.IsArray()) {
        const auto& arr{node.AsArray()};

        const size_t kRGBSize = 3;
        const size_t kRGBASize = 4;

        if (kRGBSize == arr.size()) {
            uint8_t red = static_cast<uint8_t>(arr.at(0).AsInt());
            uint8_t green = static_cast<uint8_t>(arr.at(1).AsInt());
            uint8_t blue = static_cast<uint8_t>(arr.at(2).AsInt());

            color = renderer::Rgb{red, green, blue};

        } else if (kRGBASize == arr.size()) {
            uint8_t red = static_cast<uint8_t>(arr.at(0).AsInt());
            uint8_t green = static_cast<uint8_t>(arr.at(1).AsInt());
            uint8_t blue = static_cast<uint8_t>(arr.at(2).AsInt());
            double alpha = arr.at(3).AsDouble();

            color = renderer::Rgba{red, green, blue, alpha};
        }
    } else if (node.IsString()) {
        color = node.AsString();
    }

    return color;
}

Node CreateErrorNode(const int id) {
    using std::string_literals::operator""s;

    json::Dict error;

    error["request_id"s] = id;
    error["error_message"s] = "not found"s;

    return Node{std::move(error)};
}

Node CreateBusInfoNode(const int id, const std::string_view name,
                       const RequestHandler& rhandler) {
    using std::string_literals::operator""s;

    Node result;

    if (const auto kBusInfo{rhandler.GetBusInfo(name)}; kBusInfo.has_value()) {
        json::Dict node;

        node["request_id"s] = id;
        node["route_length"s] = kBusInfo->route_length;
        node["stop_count"s] = static_cast<int>(kBusInfo->stops);
        node["unique_stop_count"s] = static_cast<int>(kBusInfo->unique_stops);
        node["curvature"s] = kBusInfo->curvature;

        result = std::move(node);
    } else {
        result = CreateErrorNode(id);
    }

    return result;
}

Node CreateStopInfoNode(const int id, const std::string_view name,
                        const RequestHandler& rhandler) {
    using std::string_literals::operator""s;

    if (!rhandler.IsStopExists(name)) {
        return CreateErrorNode(id);
    }

    json::Dict node;
    node["request_id"s] = id;

    if (const auto& kBusesByStop{rhandler.GetBusesByStop(name)};
        !kBusesByStop.empty()) {
        json::Array buses;
        buses.reserve(kBusesByStop.size());

        auto transformer = [](std::string_view bus_name) -> Node {
            return Node{std::string{bus_name}};
        };

        std::ranges::transform(kBusesByStop, std::back_inserter(buses),
                               transformer);

        node["buses"s] = std::move(buses);
    } else {
        node["buses"s] = json::Array{};
    }

    return Node{node};
}

Node CreateMapInfoNode(const int id, const RequestHandler& rhandler) {
    using std::string_literals::operator""s;
    std::ostringstream sstream;
    rhandler.RenderMap().Render(sstream);

    json::Dict node;

    node["request_id"s] = id;
    node["map"s] = sstream.str();

    return Node{node};
}

Document ReadJSON(std::istream& in) {
    return json::Load(in);
}

JsonReader::JsonReader(Document requests) : requests_(std::move(requests)) {
}

void JsonReader::ParseBaseRequests() {
    using std::string_literals::operator""s;

    const auto& stat_requests_array{
        GetRequestsNode("base_requests"s).AsArray()};

    for (const auto& request_node : stat_requests_array) {
        const auto& request_map{request_node.AsMap()};

        if (auto it = request_map.find("type"s); it != request_map.cend()) {
            if ("Stop"s == it->second) {
                base_stop_requests_.push_back(&request_node);
            } else if ("Bus"s == it->second) {
                base_bus_requests_.push_back(&request_node);
            }
        }
    }
}

RenderSettings JsonReader::ParseRenderSettings() const {
    using std::string_literals::operator""s;

    [[maybe_unused]]
    const auto& render_settings_map{
        GetRequestsNode("render_settings"s).AsMap()};

    RenderSettings result;

    result.width = render_settings_map.at("width"s).AsDouble();
    result.height = render_settings_map.at("height"s).AsDouble();

    result.padding = render_settings_map.at("padding"s).AsDouble();

    result.line_width = render_settings_map.at("line_width"s).AsDouble();
    result.stop_radius = render_settings_map.at("stop_radius"s).AsDouble();

    result.bus_label_font_size = static_cast<uint32_t>(  //
        render_settings_map.at("bus_label_font_size"s).AsInt());

    const auto& bus_label_offset_arr{
        render_settings_map.at("bus_label_offset"s).AsArray()};
    result.bus_label_offset = {bus_label_offset_arr.at(0).AsDouble(),
                               bus_label_offset_arr.at(1).AsDouble()};

    result.stop_label_font_size = static_cast<uint32_t>(  //
        render_settings_map.at("stop_label_font_size"s).AsInt());

    const auto& stop_label_offset_arr{
        render_settings_map.at("stop_label_offset"s).AsArray()};
    result.stop_label_offset = {stop_label_offset_arr.at(0).AsDouble(),
                                stop_label_offset_arr.at(1).AsDouble()};

    const auto& underlayer_color_node{
        render_settings_map.at("underlayer_color"s)};
    result.underlayer_color = ParseColor(underlayer_color_node);

    result.underlayer_width =
        render_settings_map.at("underlayer_width"s).AsDouble();

    const auto& color_palette_arr{
        render_settings_map.at("color_palette"s).AsArray()};
    result.color_palette.reserve(color_palette_arr.size());

    auto transformer = [](const Node& color) { return ParseColor(color); };

    std::ranges::transform(color_palette_arr,
                           std::back_inserter(result.color_palette),
                           transformer);

    return result;
}

void JsonReader::ApplyBaseRequests(tc::TransportCatalogue& db) const {
    ApplyBaseStopRequests(db);

    ApplyBaseBusRequests(db);
}

void JsonReader::PrintStatRequests(std::ostream& out,
                                   const RequestHandler& rhandler) const {
    using std::string_literals::operator""s;

    const auto& stat_requests_arr{GetRequestsNode("stat_requests"s).AsArray()};

    json::Array arr{};
    arr.reserve(stat_requests_arr.size());

    for (const auto& stat_request_node : stat_requests_arr) {
        const auto& stat_request_map{stat_request_node.AsMap()};

        const int kID = stat_request_map.at("id"s).AsInt();
        const std::string_view kType{stat_request_map.at("type"s).AsString()};

        if ("Bus"s == kType) {
            const std::string_view kName{
                stat_request_map.at("name"s).AsString()};
            arr.emplace_back(CreateBusInfoNode(kID, kName, rhandler));
        } else if ("Stop"s == kType) {
            const std::string_view kName{
                stat_request_map.at("name"s).AsString()};
            arr.emplace_back(CreateStopInfoNode(kID, kName, rhandler));
        } else if ("Map"s == kType) {
            // TODO add map svg document
            arr.emplace_back(CreateMapInfoNode(kID, rhandler));
        }
    }

    json::Print(Document{arr}, out);
}

const Node& JsonReader::GetRequestsNode(const std::string& key) const {
    using std::string_literals::operator""s;

    const Node& root = requests_.GetRoot();
    if (!root.IsMap()) {
        throw json::ParsingError("wrong input json format, expect map"s);
    }

    const auto& dict{root.AsMap()};
    const auto kIt = dict.find(key);
    if (kIt == dict.cend()) {
        throw json::ParsingError("expect \""s + key + "\" map object"s);
    }
    if (!kIt->second.IsArray() && !kIt->second.IsMap()) {
        throw json::ParsingError("wrong \""s + key +
                                 "\" node type, expect array or map"s);
    }

    return kIt->second;
}

void JsonReader::ApplyBaseStopRequests(tc::TransportCatalogue& db) const {
    using std::string_literals::operator""s;

    // fill base with stops (name+coordinates) only
    for (const auto* request : base_stop_requests_) {
        const auto& req_map{request->AsMap()};

        const std::string_view kName{req_map.at("name"s).AsString()};
        const double kLat = req_map.at("latitude"s).AsDouble();
        const double kLng = req_map.at("longitude"s).AsDouble();

        db.AddStop({.name{kName}, .coordinates{.lat = kLat, .lng = kLng}});
    }

    // fill distances between stops
    for (const auto* request : base_stop_requests_) {
        const auto& req_map{request->AsMap()};

        const auto& distances_node{req_map.at("road_distances"s)};

        const std::vector<std::pair<std::string_view, int>> kDistances{
            ParseDistance(distances_node)};

        if (!kDistances.empty()) {
            for (const auto& distance : kDistances) {
                db.SetDistance(req_map.at("name"s).AsString(), distance.first,
                               distance.second);
            }
        }
    }
}

void JsonReader::ApplyBaseBusRequests(tc::TransportCatalogue& db) const {
    using std::string_literals::operator""s;

    for (const auto* request : base_bus_requests_) {
        const bool kIsRoundtrip = request->AsMap().at("is_roundtrip"s).AsBool();
        const std::string_view kName{request->AsMap().at("name"s).AsString()};

        const std::vector<std::string_view> kRoute{ParseRoute(*request)};

        if (!kRoute.empty()) {
            db.AddBus(kName, kRoute, kIsRoundtrip);
        }
    }
}

}  // namespace json_reader