#include "svg.hpp"

namespace svg {

using namespace std::literals;
// -------- Render Context ------------

RenderContext::RenderContext(std::ostream& out)
    : out(out) {
}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
    : out(out)
    , indent_step(indent_step)
    , indent(indent) {
}

RenderContext RenderContext::Indented() const {
    return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}
// --------- PathProps -----------------

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_linecap) {
    switch(stroke_linecap) {
    case(StrokeLineCap::BUTT):
        out << "butt";
        break;
    case(StrokeLineCap::ROUND):
        out << "round";
        break;
    case(StrokeLineCap::SQUARE):
        out << "square";
        break;
    }
    
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_linejoin) {
    switch(stroke_linejoin) {
    case(StrokeLineJoin::ARCS):
        out << "arcs";
        break;
    case(StrokeLineJoin::BEVEL):
        out << "bevel";
        break;
    case(StrokeLineJoin::MITER):
        out << "miter";
        break;
    case(StrokeLineJoin::MITER_CLIP):
        out << "miter-clip";
        break;
    case(StrokeLineJoin::ROUND):
        out << "round";
        break;
    }
    
    return out;
}


// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\"" << center_.x << "\" cy=\"" << center_.y << "\" ";
    out << "r=\"" << radius_ << "\" ";
    RenderAttrs(out);
    out << "/>";
}

// --------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";

    for(const auto point : points_) {
        out << point.x << "," << point.y << " ";
    }

    out << "\"";
    RenderAttrs(out);
    out << "/>";
}

// ----------- Text ------------------

Text& Text::SetPosition(Point position) {
    position_ = position;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t font_size) {
    font_size_ = font_size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}


void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text";

    //render attributes
    {
        out << " x=\"" << position_.x << "\"";
        out << " y=\"" << position_.y << "\"";
        out << " dx=\"" << offset_.x << "\"";
        out << " dy=\"" << offset_.y << "\"";
        out << " font-size=\"" << font_size_ << "\"";
    
        if(font_family_) {
            out << " font-family=\"" << *font_family_ << "\"";
        }
    
        if(font_weight_) {
            out << " font-weight=\"" << *font_weight_ << "\"";
        }
        RenderAttrs(out);
    }

    out << ">";
    //render text
    {   
        if(data_) {
            for(char ch : *data_) {
                switch(ch) {
                case '"': out << "&quot;"; break;
                case '<': out << "&lt;"; break;
                case '>': out << "&gt;"; break;
                case '\'': out << "&apos;"; break;
                case '&': out << "&amp;"; break;
                default: out << ch;
                }
            }
        }
    }
    out << "</text>";
}

// ---------- Document ----------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_uptrs_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    RenderContext context(out, 2, 2);
    for(auto& uptr : objects_uptrs_) {
        uptr->Render(context);
    }
    out << "</svg>\n";
}
}  // namespace svg