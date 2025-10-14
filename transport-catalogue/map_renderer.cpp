#include "map_renderer.h"

namespace renderer {

  const RenderSettings MapRenderer::GetSettings() const {
    return render_settings_;
  }

  std::vector <svg::Polyline> MapRenderer::CreateBusLine(std::map <std::string_view, const transport::Bus*> buses, renderer::SphereProjector &projector) const {
    std::vector <svg::Polyline> result;
    int number = 0;
    for (const auto & [name, bus]: buses) {
      if (bus -> stops.empty()) continue;
      svg::Polyline line;
      std::vector <const transport::Stop*> all_stops {bus -> stops.begin(), bus -> stops.end()};
      if (bus -> is_roundtrip == false) all_stops.insert(all_stops.end(), std::next(bus -> stops.rbegin()), bus -> stops.rend());
      for (const auto& stop : all_stops) {
        line.AddPoint(projector(stop -> coordinates));
      }

      line.SetStrokeColor(render_settings_.color_palette[number]);
      line.SetFillColor("none");
      line.SetStrokeWidth(render_settings_.line_width);
      line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
      line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

      if (static_cast <size_t> (number) < render_settings_.color_palette.size() - 1) number++;
      else number = 0;
      result.push_back(line);
    }
    return result;
  }

  std::vector <svg::Text> MapRenderer::CreateBusName(std::map < std::string_view, const transport::Bus*> & buses, renderer::SphereProjector & projector) const {
    std::vector <svg::Text> result;
    svg::Text name;
    svg::Text underlayer;
    int number = 0;
    for (const auto& [bus_name, bus] : buses) {
      if (bus -> stops.empty()) continue;
      name.SetPosition(projector(bus -> stops[0] -> coordinates));
      name.SetOffset(render_settings_.bus_label_offset);
      name.SetFontSize(render_settings_.bus_label_font_size);
      name.SetFontFamily("Verdana");
      name.SetFontWeight("bold");
      name.SetData(bus -> bus_name);
      name.SetFillColor(render_settings_.color_palette[number]);

      underlayer.SetPosition(projector(bus -> stops[0] -> coordinates));
      underlayer.SetOffset(render_settings_.bus_label_offset);
      underlayer.SetFontSize(render_settings_.bus_label_font_size);
      underlayer.SetFontFamily("Verdana");
      underlayer.SetFontWeight("bold");
      underlayer.SetData(bus -> bus_name);
      underlayer.SetFillColor(render_settings_.underlayer_color);
      underlayer.SetStrokeColor(render_settings_.underlayer_color);
      underlayer.SetStrokeWidth(render_settings_.underlayer_width);
      underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
      underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

      result.push_back(underlayer);
      result.push_back(name);
      
      if (bus -> is_roundtrip == false) {
        if (bus -> stops[0] != bus -> stops[bus -> stops.size() - 1]) {
          svg::Text second_name {name};
          second_name.SetPosition(projector(bus -> stops[bus -> stops.size() - 1] -> coordinates));

          svg::Text second_underlayer {underlayer};
          second_underlayer.SetPosition(projector(bus -> stops[bus -> stops.size() - 1] -> coordinates));
          result.push_back(second_underlayer);
          result.push_back(second_name);
        }
      }
      if (static_cast < size_t > (number) < render_settings_.color_palette.size() - 1) number++;
      else number = 0;
    }
    return result;
  }

  std::vector < svg::Circle > MapRenderer::CreateStopCircles(std::map < std::string_view, const transport::Bus * > & buses, renderer::SphereProjector & projector) const {
    std::vector < svg::Circle > result;
    std::map < std::string_view, const transport::Stop*> stops;
    for (const auto& [name, bus] : buses) {
      for (const auto& stop : bus -> stops) {
        stops[stop -> stop_name] = stop;
      }
    }
    for (const auto& [name, stop] : stops) {
      svg::Circle circle;
      circle.SetCenter(projector(stop -> coordinates));
      circle.SetRadius(render_settings_.stop_radius);
      circle.SetFillColor("white");
      result.push_back(circle);
    }
    return result;
  }

  std::vector <svg::Text> MapRenderer::CreateStopName(std::map <std::string_view, const transport::Bus*> & buses, renderer::SphereProjector& projector) const {
    std::vector <svg::Text> result;
    svg::Text name;
    svg::Text underlayer;
    std::map <std::string_view, const transport::Stop*> stops;
    for (const auto& [name, bus] : buses) {
      for (const auto& stop : bus -> stops) {
        stops[stop -> stop_name] = stop;
      }
    }
    for (const auto& [stop_name, stop]: stops) {
      name.SetPosition(projector(stop -> coordinates));
      name.SetOffset(render_settings_.stop_label_offset);
      name.SetFontSize(render_settings_.stop_label_font_size);
      name.SetFontFamily("Verdana");
      name.SetData(stop -> stop_name);
      name.SetFillColor("black");

      underlayer.SetPosition(projector(stop -> coordinates));
      underlayer.SetOffset(render_settings_.stop_label_offset);
      underlayer.SetFontSize(render_settings_.stop_label_font_size);
      underlayer.SetFontFamily("Verdana");
      underlayer.SetData(stop -> stop_name);
      underlayer.SetFillColor(render_settings_.underlayer_color);
      underlayer.SetStrokeColor(render_settings_.underlayer_color);
      underlayer.SetStrokeWidth(render_settings_.underlayer_width);
      underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
      underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

      result.push_back(underlayer);
      result.push_back(name);

    }
    return result;
  }

  svg::Document MapRenderer::CreateBusesMap(const transport::TransportCatalogue& catalogue) const {
    svg::Document result;
    std::map <std::string_view, const transport::Bus*> buses = catalogue.GetAllBuses();
    std::vector <geo::Coordinates> coordinates;
    std::map <std::string_view, transport::Stop*> all_stops;
    for (const auto& [name, bus]: buses) {
      if (bus -> stops.empty()) continue;
      for (const auto& stop: bus -> stops) {
        all_stops[stop -> stop_name] = stop;
      }
    }
    for (const auto[name, stop]: all_stops) {
      coordinates.push_back(stop -> coordinates);
    }
    const renderer::RenderSettings render_settings = GetSettings();
    renderer::SphereProjector projector{coordinates.begin(), coordinates.end(), render_settings.width, render_settings.height, render_settings.padding};
    std::vector <svg::Polyline> lines = CreateBusLine(buses, projector);
    for (auto& line: lines) {
      result.Add(std::move(line));
    }
    std::vector <svg::Text> buses_names = CreateBusName(buses, projector);
    for (auto& bus_name : buses_names) {
      result.Add(std::move(bus_name));
    }
    std::vector <svg::Circle> circles = CreateStopCircles(buses, projector);
    for (auto& circle : circles) {
      result.Add(std::move(circle));
    }
    std::vector <svg::Text> stops_names = CreateStopName(buses, projector);
    for (auto& stop_name : stops_names) {
      result.Add(std::move(stop_name));
    }
    return result;
  }

  std::string MapRenderer::RenderBusesMap(const transport::TransportCatalogue& catalogue) const {
    svg::Document doc = CreateBusesMap(catalogue);
    std::ostringstream os;
    doc.Render(os);
    std::string map_str = os.str();

    return map_str;

  }
}
