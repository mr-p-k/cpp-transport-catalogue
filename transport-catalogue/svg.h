#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace svg {

using std::string_view_literals::operator""sv;

enum class StrokeLineCap : char {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin : char {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

struct Rgb {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct Rgba {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

inline const Color NoneColor{std::monostate{}};

std::ostream& operator<<(std::ostream& out, Color color);

struct ColorPrinter {
    explicit ColorPrinter(std::ostream& ostr);

    std::ostream& out;

    void operator()(std::monostate /*none*/) const;

    void operator()(const std::string& color) const;

    void operator()(Rgb rgb) const;

    void operator()(Rgba rgba) const;
};

template <typename Owner>
class PathProps {
private:
    PathProps() = default;

public:
    Owner& SetFillColor(Color fill_color) {
        fill_color_ = std::move(fill_color);

        return AsOwner();
    }

    Owner& SetStrokeColor(Color stroke_color) {
        stroke_color_ = std::move(stroke_color);

        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;

        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;

        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;

        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        if (fill_color_.has_value()) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }

        if (stroke_color_.has_value()) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }

        if (stroke_width_.has_value()) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }

        if (stroke_line_cap_.has_value()) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }

        if (stroke_line_join_.has_value()) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

private:
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;

    friend Owner;
};

class Object;

class ObjectContainer {
public:
    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    virtual ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& object_contailer) const = 0;

    virtual ~Drawable() = default;
};

struct Point {
    Point() = default;

    Point(double x, double y) : x(x), y(y) {
    }

    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с
 * отступами. Хранит ссылку на поток вывода, текущее значение и шаг отступа при
 * выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out) : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out), indent_step(indent_step), indent(indent) {
    }

    [[nodiscard]] RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    Object() = default;

    Object(const Object& other) = default;

    Object(Object&& other) = default;

    virtual ~Object() = default;

    Object& operator=(const Object& other) = default;

    Object& operator=(Object&& other) = default;

    void Render(const RenderContext& context) const;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);

    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

private:
    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

private:
    std::deque<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

private:
    Point pos_ = {0.0, 0.0};
    Point offset_ = {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weigth_;
    std::string data_;
};

class Document final : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
    std::deque<std::unique_ptr<Object>> objects_;
};

}  // namespace svg