#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из
 * JSON, а также код обработки запросов к базе и формирование массива ответов в
 * формате JSON
 */

namespace json_reader {

using json::Document;

using json::Node;

using request_handler::RequestHandler;

using renderer::RenderSettings;

Document ReadJSON(std::istream& in);

class JsonReader final {
public:
    explicit JsonReader(Document requests);

    void ParseBaseRequests();

    [[nodiscard]] renderer::RenderSettings ParseRenderSettings() const;

    void ApplyBaseRequests(tc::TransportCatalogue& db) const;

    void PrintStatRequests(std::ostream& out,
                           const RequestHandler& rhandler) const;

private:
    [[nodiscard]] const Node& GetRequestsNode(const std::string& key) const;

    void ApplyBaseStopRequests(tc::TransportCatalogue& db) const;

    void ApplyBaseBusRequests(tc::TransportCatalogue& db) const;

    Document requests_;

    std::vector<const Node*> base_stop_requests_;

    std::vector<const Node*> base_bus_requests_;
};

}  // namespace json_reader