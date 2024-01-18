#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
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
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
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
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        if (std::holds_alternative<std::monostate>(color)) {
            out << "none"sv;
        }
        if (std::holds_alternative<std::string>(color)) {
            out << std::get<std::string>(color);
        }
        if (std::holds_alternative<Rgb>(color)) {
            Rgb rgb = std::get<Rgb>(color);
            out << "rgb("sv << std::to_string(rgb.red) << ',' << std::to_string(rgb.green) << ',' << std::to_string(rgb.blue) << ')';
        }
        if (std::holds_alternative<Rgba>(color)) {
            Rgba rgba = std::get<Rgba>(color);
            out << "rgba("sv << std::to_string(rgba.red) << ',' << std::to_string(rgba.green) << ',' << std::to_string(rgba.blue) << ',' << rgba.opacity << ')';
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
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        if (!points_.empty()) {
            out << points_.begin()->x << ',' << points_.begin()->y;
            auto it = std::next(points_.begin());
            for (auto i = it; i < points_.end(); ++i) {
                out << ' ' << i->x << ',' << i->y;
            }
        }
        out << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
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
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        text_ = data;
        return *this;
    }

    /* Двойная кавычка " заменяется на &quot;
    Одинарная кавычка или апостроф ' заменяется на &apos;
    Символы < и > заменяются на &lt; и &gt; соответственно.
    Амперсанд& заменяется на &amp; */

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x = \""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << '>';
        if (!text_.empty()) {
            for (auto c : text_) {
                switch (c) {
                case '"':
                    out << "&quot;"sv;
                    break;
                case '\'':
                    out << "&apos;"sv;
                    break;
                case '<':
                    out << "&lt;"sv;
                    break;
                case '>':
                    out << "&gt;"sv;
                    break;
                case '&':
                    out << "&amp;";
                    break;
                default:
                    out << c;
                    break;
                }
            }
        }
        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext context(out);
        for (const auto& obj : objects_) {
            obj.get()->Render(context);
        }
        out << "</svg>"sv;
    }

}  // namespace svg