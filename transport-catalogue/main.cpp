#include <cstdlib>
#include <iostream>

#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace {

class App final {
public:
    explicit App(std::istream& in, std::ostream& out) : in_(in), out_(out) {
    }

    App(const App&) = delete;
    App(App&&) = delete;

    App& operator=(const App&) = delete;
    App& operator=(App&&) = delete;

    ~App() = default;

    void Run() const {
        tc::TransportCatalogue db;

        json_reader::JsonReader reader{json_reader::ReadJSON(in_)};

        reader.ParseBaseRequests();

        reader.ApplyBaseRequests(db);

        const renderer::MapRenderer kRenderer(reader.ParseRenderSettings());

        const request_handler::RequestHandler kRHandler{db, kRenderer};

        const router::TransportRouter kRouter(reader.ParseRoutingSettings(),
                                              kRHandler);

        reader.PrintStatRequests(out_, kRHandler, kRouter);
    }

private:
    std::istream& in_;
    std::ostream& out_;
};

}  // namespace

int main() {
    const App kApp(std::cin, std::cout);

    kApp.Run();

    return EXIT_SUCCESS;
}