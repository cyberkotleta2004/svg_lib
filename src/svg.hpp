#pragma once

#include <optional>
#include <variant>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

namespace svg {

struct Rgb {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct Rgba {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    double opacity;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_linecap);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_linejoin);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out);

    RenderContext(std::ostream& out, int indent_step, int indent = 0);

    RenderContext Indented() const;

    void RenderIndent() const;

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
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

struct ColorVisitor {
    std::string property_name;
    std::ostream& out;

    void operator()(std::monostate) const;
    void operator()(const std::string& str) const;
    void operator()(const Rgb& rgb) const;
    void operator()(const Rgba& rgba) const;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color fill_color) {
        fill_color_ = std::move(fill_color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color stroke_color) {
        stroke_color_ = std::move(stroke_color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double stroke_width) {
        stroke_width_ = stroke_width;
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap stroke_linecap) {
        stroke_linecap_ = stroke_linecap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin stroke_linejoin) {
        stroke_linejoin_ = stroke_linejoin;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        std::visit(ColorVisitor{"fill", out}, fill_color_);
        std::visit(ColorVisitor{"stroke", out}, stroke_color_);

        if(stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }

        if(stroke_linecap_) {
            out << " stroke-linecap=\"" << *stroke_linecap_ << "\"";
        }

        if(stroke_linejoin_) {
            out << " stroke-linejoin=\"" << *stroke_linejoin_ << "\"";
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    Color fill_color_;
    Color stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
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

    Point center_;
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

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point position);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t font_size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point position_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::optional<std::string> font_family_;
    std::optional<std::string> font_weight_;
    std::optional<std::string> data_;
};

template <typename T>
concept Obj = std::is_base_of_v<Object, std::remove_reference_t<T>>;

class ObjectContainer {
public:
    template <Obj T>
    void Add(T&& obj) {
        AddPtr(std::make_unique<std::decay_t<T>>(std::forward<T>(obj)));
    }
protected:
    ~ObjectContainer() = default;
private:
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    std::vector<std::unique_ptr<Object>> objects_uptrs_;
};

}  // namespace svg