#include "map_renderer.h"

using namespace std;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer(RenderSettings set) : settings_(set) {}

void MapRenderer::AddStopCoordinates(const domain::Stop* stop, svg::Point point) {
    stops_with_coordinates_[stop] = point;
}

void MapRenderer::AddBus(const domain::Bus* bus) {
    buses_sorted_.push_back(bus);
}

void MapRenderer::AddStop(const domain::Stop* stop) {
    stops_sorted_.push_back(stop);
}

const unordered_map<const domain::Stop*, svg::Point>& MapRenderer::GetStopsWithCoordinates() const
{
    return stops_with_coordinates_;
}

const vector<const domain::Stop*>& MapRenderer::GetStopsSortedList() const
{
    return stops_sorted_;
}

const vector<const domain::Bus*>& MapRenderer::GetBusesSortedList() const
{
    return buses_sorted_;
}

const RenderSettings& MapRenderer::GetRenderSettings() const {
    return settings_;
}

void MapRenderer::IdentifyBusMapping(const deque<domain::Bus>& buses) {
    for (const domain::Bus& bus : buses) {
        buses_sorted_.push_back(&bus);
    }
    sort(buses_sorted_.begin(), buses_sorted_.end(), [](const domain::Bus* left, const domain::Bus* right) { return left->name < right->name; });
}

void MapRenderer::IdentifyStopMapping(const deque<domain::Stop>& stops) {
    for (const domain::Bus* bus : buses_sorted_) {
        stops_sorted_.insert(stops_sorted_.end(), bus->route.begin(), bus->route.end());
    }
    sort(stops_sorted_.begin(), stops_sorted_.end(), [](const domain::Stop* left, const domain::Stop* right) {return left->name < right->name; });
    auto last = unique(stops_sorted_.begin(), stops_sorted_.end());
    stops_sorted_.erase(last, stops_sorted_.end());
    vector<geo::Coordinates> coordinates;
    for (const domain::Stop* stop : stops_sorted_) {
        coordinates.push_back({ stop->latitude, stop->longitude });
    }
    const SphereProjector proj(coordinates.begin(), coordinates.end(), settings_.width, settings_.height, settings_.padding);
    for (const domain::Stop* stop : stops_sorted_) {
        stops_with_coordinates_[stop] = proj({ stop->latitude, stop->longitude });
    }
}

svg::Document MapRenderer::MakeMap() const {
    svg::Document map;
    MapBusImage(map);
    MapBusNames(map);
    MapStopImage(map);
    MapStopNames(map);
    return map;
}

void MapRenderer::RenderMap(ostream& out, const svg::Document& map) const {
    map.Render(out);
}

void MapRenderer::MapStopNames(svg::Document& map) const {
    for (const domain::Stop* stop : stops_sorted_) {
        svg::Text text;
        text.SetPosition(stops_with_coordinates_.at(stop)).SetOffset(settings_.stop_label_offset).SetFontSize(settings_.stop_label_font_size).
            SetFontFamily("Verdana"s).SetData(stop->name);
        svg::Text underlayer = text;
        underlayer.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        map.Add(move(underlayer));
        svg::Text sign = text;
        sign.SetFillColor("black"s);
        map.Add(move(sign));
    }
}

void MapRenderer::MapBusNames(svg::Document& map) const {
    size_t color_count = 0;
    for (const domain::Bus* bus : buses_sorted_) {
        if (!bus->route.empty()) {
            svg::Text text;
            text.SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
            text.SetPosition(stops_with_coordinates_.at(bus->route[0])).SetData(bus->name);
            svg::Text underlayer = text;
            underlayer.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            map.Add(move(underlayer));
            svg::Text sign = text;
            sign.SetFillColor(settings_.color_palette[color_count]);
            map.Add(move(sign));
            if (!bus->is_round && (bus->route[0]->name != bus->route[bus->route.size() - 1]->name)) {
                svg::Text text;
                text.SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana"s).SetFontWeight("bold"s);
                size_t last_stop = bus->route.size() - 1;
                text.SetPosition(stops_with_coordinates_.at(bus->route[last_stop])).SetData(bus->name);
                svg::Text underlayer = text;
                underlayer.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                map.Add(move(underlayer));
                svg::Text sign = text;
                sign.SetFillColor(settings_.color_palette[color_count]);
                map.Add(move(sign));
            }
            ++color_count;
            if (color_count == settings_.color_palette.size()) {
                color_count = 0;
            }
        }
    }
}

void MapRenderer::MapStopImage(svg::Document& map) const {
    for (const domain::Stop* stop : stops_sorted_) {
        svg::Circle circ;
        circ.SetCenter(stops_with_coordinates_.at(stop)).SetRadius(settings_.stop_radius).SetFillColor("white"s);
        map.Add(move(circ));
    }
}

void MapRenderer::MapBusImage(svg::Document& map) const {
    size_t color_count = 0;
    for (const domain::Bus* bus : buses_sorted_) {
        if (!bus->route.empty()) {
            svg::Polyline poly;
            poly.SetStrokeWidth(settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                SetFillColor(svg::NoneColor).SetStrokeColor(settings_.color_palette[color_count]);
            ++color_count;
            if (color_count == settings_.color_palette.size()) {
                color_count = 0;
            }
            for (const domain::Stop* stop : bus->route) {
                poly.AddPoint(stops_with_coordinates_.at(stop));
            }
            if (!bus->is_round) {
                for (auto rit = next(bus->route.rbegin()); rit != bus->route.rend(); ++rit) {
                    poly.AddPoint(stops_with_coordinates_.at(*rit));
                }
            }
            map.Add(move(poly));
        }
    }
}
