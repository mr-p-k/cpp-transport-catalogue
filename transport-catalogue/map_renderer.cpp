#include "map_renderer.h"

#include <algorithm>
#include <utility>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты
 * маршрутов в формате SVG. Визуализация маршртутов вам понадобится во второй
 * части итогового проекта. Пока можете оставить файл пустым.
 */

namespace renderer {

namespace {

SphereProjector MakeProjector(const std::vector<const Bus*>& buses,
                              const RenderSettings& settings) {
    std::vector<geo::Coordinates> geo_coords;
    geo_coords.reserve(buses.size());

    for (const auto& bus : buses) {
        for (const auto& stop : bus->route) {
            geo_coords.emplace_back(stop->coordinates);
        }
    }

    return SphereProjector(geo_coords.cbegin(), geo_coords.cend(),
                           settings.width, settings.height, settings.padding);
}

}  // namespace

MapRenderer::MapRenderer(RenderSettings settings)
    : settings_(std::move(settings)) {
}

// svg::Document MapRenderer::Render(const std::vector<const Bus*>& buses,
//                                   const std::vector<const Stop*>& stops)
//                                   const {
//     auto copy_buses{buses};
//     auto copy_stops{stops};

//     return Render(std::move(copy_buses), std::move(copy_stops));
// }

svg::Document MapRenderer::Render(const std::vector<const Bus*>& buses,
                                  const std::vector<const Stop*>& stops) const {
    auto copy_buses{buses};
    auto copy_stops{stops};
    // sort buses and stops vectors
    std::ranges::sort(copy_buses, Comparator<Bus>{});
    std::ranges::sort(copy_stops, Comparator<Stop>{});

    const SphereProjector proj{MakeProjector(copy_buses, settings_)};

    svg::Document image;

    // render image
    RenderRouteLines(copy_buses, image, proj);
    RenderRouteNames(copy_buses, image, proj);
    RenderStopCircles(copy_stops, image, proj);
    RenderStopNames(copy_stops, image, proj);

    return image;
}

void MapRenderer::RenderRouteLines(const std::vector<const Bus*>& routes,
                                   svg::Document& map,
                                   const SphereProjector& proj) const {
    size_t route_counter = 0;
    for (const auto& route : routes) {
        svg::Polyline r;

        size_t idx = route_counter % settings_.color_palette.size();
        r.SetStrokeColor(settings_.color_palette[idx])
            .SetFillColor(svg::NoneColor)
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto& stop : route->route) {
            r.AddPoint(proj(stop->coordinates));
        }

        if (!route->is_roundtrip) {
            for (auto it{std::next(route->route.crbegin(), 1)};
                 it != route->route.crend(); ++it) {
                r.AddPoint(proj((*it)->coordinates));
            }
        }

        map.Add(r);

        ++route_counter;
    }
}

void MapRenderer::RenderRouteNames(
    [[maybe_unused]] const std::vector<const Bus*>& routes,
    [[maybe_unused]] svg::Document& image, const SphereProjector& proj) const {
    using std::string_literals::operator""s;

    svg::Text name;

    name.SetOffset({settings_.bus_label_offset.first,
                    settings_.bus_label_offset.second})
        .SetFontSize(settings_.bus_label_font_size)
        .SetFontFamily("Verdana"s)
        .SetFontWeight("bold"s);

    svg::Text underlayer{name};

    underlayer.SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    size_t route_counter = 0;
    for (const auto route : routes) {
        svg::Text begin_name{name};
        svg::Text begin_underlayer{underlayer};

        size_t idx = route_counter % settings_.color_palette.size();
        begin_name.SetFillColor(settings_.color_palette[idx]);

        svg::Point text_pos_begin{proj(route->route.front()->coordinates)};

        begin_name.SetPosition(text_pos_begin);
        begin_underlayer.SetPosition(text_pos_begin);

        begin_name.SetData(route->name);
        begin_underlayer.SetData(route->name);

        svg::Point text_pos_end{proj(route->route.back()->coordinates)};
        if (!route->is_roundtrip &&
            route->route.front() != route->route.back()) {
            svg::Text end_name{begin_name};
            svg::Text end_underlayer{begin_underlayer};

            end_name.SetPosition(text_pos_end);
            end_underlayer.SetPosition(text_pos_end);

            image.Add(begin_underlayer);
            image.Add(begin_name);

            image.Add(end_underlayer);
            image.Add(end_name);
        } else {
            image.Add(begin_underlayer);
            image.Add(begin_name);
        }

        ++route_counter;
    }
}

void MapRenderer::RenderStopCircles(const std::vector<const Stop*>& stops,
                                    svg::Document& image,
                                    const SphereProjector& proj) const {
    using std::string_literals::operator""s;

    for (const Stop* stop : stops) {
        if (nullptr == stop) {
            continue;
        }

        svg::Circle stop_circle;

        stop_circle.SetRadius(settings_.stop_radius)
            .SetFillColor("white"s)
            .SetCenter(proj(stop->coordinates));

        image.Add(stop_circle);
    }
}

void MapRenderer::RenderStopNames(const std::vector<const Stop*>& stops,
                                  svg::Document& image,
                                  const SphereProjector& proj) const {
    using std::string_literals::operator""s;

    svg::Text underlayer;

    underlayer
        .SetOffset({settings_.stop_label_offset.first,
                    settings_.stop_label_offset.second})
        .SetFontSize(settings_.stop_label_font_size)
        .SetFontFamily("Verdana"s);

    svg::Text name{underlayer};

    underlayer.SetFillColor(settings_.underlayer_color)
        .SetStrokeColor(settings_.underlayer_color)
        .SetStrokeWidth(settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    name.SetFillColor("black"s);

    for (const auto* stop : stops) {
        if (nullptr == stop) {
            continue;
        }

        svg::Text stop_underlayer{underlayer};
        svg::Text stop_name{name};

        auto stop_pos{proj(stop->coordinates)};

        stop_underlayer.SetPosition(stop_pos);
        stop_name.SetPosition(stop_pos);

        stop_underlayer.SetData(stop->name);
        stop_name.SetData(stop->name);

        image.Add(stop_underlayer);
        image.Add(stop_name);
    }
}

}  // namespace renderer