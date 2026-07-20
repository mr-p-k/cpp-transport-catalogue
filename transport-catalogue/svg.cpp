#include "svg.h"

namespace svg {

using namespace std::literals;

class RenderSpecialSymbols {
public:
    explicit RenderSpecialSymbols(std::string_view data) : data_(data) {
    }

    friend std::ostream& operator<<(std::ostream& out,
                                    const RenderSpecialSymbols& renderer);

private:
    std::string_view data_;
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    using std::string_view_literals::operator""sv;

    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    using std::string_view_literals::operator""sv;

    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, Color color) {
    using std::string_view_literals::operator""sv;

    std::visit(ColorPrinter{out}, color);

    return out;
}

ColorPrinter::ColorPrinter(std::ostream& ostr) : out(ostr) {
}

void ColorPrinter::operator()(std::monostate /*none*/) const {
    out << "none"sv;
}

void ColorPrinter::operator()(const std::string& color) const {
    out << color;
}

void ColorPrinter::operator()(Rgb rgb) const {
    out << "rgb("sv << static_cast<unsigned int>(rgb.red) << ","sv
        << static_cast<unsigned int>(rgb.green) << ","sv
        << static_cast<unsigned int>(rgb.blue) << ")"sv;
}

void ColorPrinter::operator()(Rgba rgba) const {
    out << "rgba("sv << static_cast<unsigned int>(rgba.red) << ","sv
        << static_cast<unsigned int>(rgba.green) << ","sv
        << static_cast<unsigned int>(rgba.blue) << ","sv << rgba.opacity
        << ")"sv;
}

std::ostream& operator<<(std::ostream& out,
                         const RenderSpecialSymbols& renderer) {
    for (auto symbol : renderer.data_) {
        switch (symbol) {
            case '\'':
                out << "&apos;";
                break;
            case '"':
                out << "&quot;";
                break;
            case '<':
                out << "&lt;";
                break;
            case '>':
                out << "&gt;";
                break;
            case '&':
                out << "&amp;";
                break;
            default:
                out << symbol;
                break;
        }
    }

    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y
        << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);

    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<polyline points=\""sv;

    bool is_first = true;
    for (const auto& p : points_) {
        if (!is_first) {
            out << ' ';
        } else {
            is_first = false;
        }

        out << p.x << ',' << p.y;
    }

    out << "\"";

    RenderAttrs(out);

    out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
    pos_ = pos;

    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;

    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;

    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);

    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weigth_ = std::move(font_weight);

    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);

    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text";
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv
        << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv
        << font_size_;

    if (!font_family_.empty()) {
        out << "\" font-family=\""sv << font_family_;
    }

    if (!font_weigth_.empty()) {
        out << "\" font-weight=\""sv << font_weigth_;
    }

    out << "\">"sv << RenderSpecialSymbols(data_) << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

    for (const auto& obj : objects_) {
        obj->Render(RenderContext(out, 2, 2));
    }

    out << "</svg>"sv;
}

}  // namespace svg