#include "json_reader.h"

#include <algorithm>
#include <cassert>
#include <sstream>

#include "json_builder.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из
 * JSON, а также код обработки запросов к базе и формирование массива ответов в
 * формате JSON
 */

namespace json_reader {

std::vector<std::pair<std::string_view, int>> ParseDistance(
    const Node& distances) {
    const auto& distances_map{distances.AsDict()};
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

    const auto& route_map{route.AsDict()};

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
            const auto kRed = static_cast<uint8_t>(arr.at(0).AsInt());
            const auto kGreen = static_cast<uint8_t>(arr.at(1).AsInt());
            const auto kBlue = static_cast<uint8_t>(arr.at(2).AsInt());

            color = renderer::Rgb{.red = kRed, .green = kGreen, .blue = kBlue};

        } else if (kRGBASize == arr.size()) {
            const auto kRed = static_cast<uint8_t>(arr.at(0).AsInt());
            const auto kGreen = static_cast<uint8_t>(arr.at(1).AsInt());
            const auto kBlue = static_cast<uint8_t>(arr.at(2).AsInt());
            const double kAlpha = arr.at(3).AsDouble();

            color = renderer::Rgba{
                .red = kRed, .green = kGreen, .blue = kBlue, .opacity = kAlpha};
        }
    } else if (node.IsString()) {
        color = node.AsString();
    }

    return color;
}

Node CreateErrorNode(const int id) {
    using std::string_literals::operator""s;

    return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(id)
        .Key("error_message"s)
        .Value("not found"s)
        .EndDict()
        .Build();
}

Node CreateBusInfoNode(const int id, const std::string_view name,
                       const RequestHandler& rhandler) {
    //
    using std::string_literals::operator""s;

    json::Builder result;

    if (const auto kBusInfo{rhandler.GetBusInfo(name)}; kBusInfo.has_value()) {
        result.StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("route_length"s)
            .Value(kBusInfo->route_length)
            .Key("stop_count"s)
            .Value(static_cast<int>(kBusInfo->stops))
            .Key("unique_stop_count"s)
            .Value(static_cast<int>(kBusInfo->unique_stops))
            .Key("curvature"s)
            .Value(kBusInfo->curvature)
            .EndDict();
    } else {
        result.Value(CreateErrorNode(id).GetValue());
    }

    return result.Build();
}

Node CreateStopInfoNode(const int id, const std::string_view name,
                        const RequestHandler& rhandler) {
    using std::string_literals::operator""s;

    if (!rhandler.IsStopExists(name)) {
        return CreateErrorNode(id);
    }

    json::Builder result;
    result.StartDict().Key("request_id"s).Value(id);

    if (const auto& kBusesByStop{rhandler.GetBusesByStop(name)};
        !kBusesByStop.empty()) {
        //
        json::Builder buses;

        buses.StartArray();

        for (const std::string_view kBus : kBusesByStop) {
            buses.Value(std::string{kBus});
        }

        result.Key("buses"s).Value(buses.EndArray().Build().GetValue());
    } else {
        result.Key("buses"s).StartArray().EndArray();
    }

    return result.EndDict().Build();
}

Node CreateMapInfoNode(const int id, const RequestHandler& rhandler) {
    using std::string_literals::operator""s;
    std::ostringstream sstream;
    rhandler.RenderMap().Render(sstream);

    return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(id)
        .Key("map"s)
        .Value(sstream.str())
        .EndDict()
        .Build();
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
        const auto& request_map{request_node.AsDict()};

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

    const auto& render_settings_map{
        GetRequestsNode("render_settings"s).AsDict()};

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

    json::Builder result;
    result.StartArray();

    for (const auto& stat_request_node : stat_requests_arr) {
        const auto& stat_request_map{stat_request_node.AsDict()};

        const int kID = stat_request_map.at("id"s).AsInt();
        const std::string_view kType{stat_request_map.at("type"s).AsString()};

        if ("Bus"s == kType) {
            const std::string_view kName{
                stat_request_map.at("name"s).AsString()};
            result.Value(CreateBusInfoNode(kID, kName, rhandler).GetValue());
        } else if ("Stop"s == kType) {
            const std::string_view kName{
                stat_request_map.at("name"s).AsString()};
            result.Value(CreateStopInfoNode(kID, kName, rhandler).GetValue());
        } else if ("Map"s == kType) {
            result.Value(CreateMapInfoNode(kID, rhandler).GetValue());
        }
    }

    json::Print(Document{result.EndArray().Build()}, out);
}

const Node& JsonReader::GetRequestsNode(const std::string& key) const {
    using std::string_literals::operator""s;

    const Node& root = requests_.GetRoot();
    if (!root.IsDict()) {
        throw json::ParsingError("wrong input json format, expect map"s);
    }

    const auto& dict{root.AsDict()};
    const auto kIt = dict.find(key);
    if (kIt == dict.cend()) {
        throw json::ParsingError("expect \""s + key + "\" map object"s);
    }
    if (!kIt->second.IsArray() && !kIt->second.IsDict()) {
        throw json::ParsingError("wrong \""s + key +
                                 "\" node type, expect array or map"s);
    }

    return kIt->second;
}

void JsonReader::ApplyBaseStopRequests(tc::TransportCatalogue& db) const {
    using std::string_literals::operator""s;

    // fill base with stops (name+coordinates) only
    for (const auto* request : base_stop_requests_) {
        const auto& req_map{request->AsDict()};

        const std::string_view kName{req_map.at("name"s).AsString()};
        const double kLat = req_map.at("latitude"s).AsDouble();
        const double kLng = req_map.at("longitude"s).AsDouble();

        db.AddStop({.name{kName}, .coordinates{.lat = kLat, .lng = kLng}});
    }

    // fill distances between stops
    for (const auto* request : base_stop_requests_) {
        const auto& req_map{request->AsDict()};

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
        const bool kIsRoundtrip =
            request->AsDict().at("is_roundtrip"s).AsBool();
        const std::string_view kName{request->AsDict().at("name"s).AsString()};

        const std::vector<std::string_view> kRoute{ParseRoute(*request)};

        if (!kRoute.empty()) {
            db.AddBus(kName, kRoute, kIsRoundtrip);
        }
    }
}

}  // namespace json_reader