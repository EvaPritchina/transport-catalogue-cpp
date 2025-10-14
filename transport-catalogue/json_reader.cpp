#include "json_reader.h"
#include "json_builder.h"

using namespace std::literals;

const json::Node& JSONReader::GetBaseRequest(){
    if (!input_.GetRoot().AsMap().count("base_requests"s)) return value_;
    return input_.GetRoot().AsMap().at("base_requests"s);
}

const json::Node& JSONReader::GetStateRequest(){
    if (!input_.GetRoot().AsMap().count("stat_requests"s)) {
        return value_;
    }
    return input_.GetRoot().AsMap().at("stat_requests"s);
}

const json::Node& JSONReader::GetRenderSettings(){
    if(!input_.GetRoot().AsMap().count("render_settings"s)) return value_;
    return input_.GetRoot().AsMap().at("render_settings"s);
}

const json::Node& JSONReader::GetRoutingSettings(){
    if(!input_.GetRoot().AsMap().count("routing_settings"s))
    return value_;
    return input_.GetRoot().AsMap().at("routing_settings"s);
}

void JSONReader::ParseCatalogue(transport::TransportCatalogue& catalogue) {
    json::Array requests = GetBaseRequest().AsArray();
    for(auto& request : requests){
        json::Dict info = request.AsMap();
        if(info.at("type"s).AsString() == "Stop"s){
            std::string stop_name = info.at("name"s).AsString();
            geo::Coordinates coordinates = {info.at("latitude"s).AsDouble(), info.at("longitude"s).AsDouble()};
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    for(auto& request : requests){
        json::Dict info = request.AsMap();
        if(info.at("type"s).AsString() == "Stop"s){
            std::string stop_name = info.at("name"s).AsString();
            std::map<std::string_view, int> distances;
            for(auto& [name, dist] : info.at("road_distances"s).AsMap()){
                distances.emplace(name, dist.AsInt());
        }
        for(auto& [name, dist] : distances){
            transport::Stop* from = catalogue.FindStop(stop_name);
            transport::Stop* to = catalogue.FindStop(name);
            auto dist_pair = std::make_pair(from, to);
            catalogue.AddDistance(dist_pair, dist);   
        }
    }
 }
    for(auto& request : requests){
        json::Dict info = request.AsMap();
        if(info.at("type"s).AsString() == "Bus"){
            std::string bus_name = info.at("name"s).AsString();
            bool is_roundtrip = info.at("is_roundtrip").AsBool();
            std::vector<transport::Stop*> stops;
            for(auto& name : info.at("stops"s).AsArray()){
                stops.push_back(catalogue.FindStop(name.AsString()));
            }
            catalogue.AddBus(bus_name, stops, is_roundtrip);
        }
    } 
}

void JSONReader::ParseFirstPart(renderer::RenderSettings&r_struct, json::Dict& info){
    r_struct.width = info.at("width"s).AsDouble();
    r_struct.height = info.at("height"s).AsDouble();
    r_struct.padding = info.at("padding"s).AsDouble();
    r_struct.line_width = info.at("line_width"s).AsDouble();
    r_struct.stop_radius = info.at("stop_radius"s).AsDouble();
}

void JSONReader::ParseLabels(renderer::RenderSettings& r_struct, json::Dict& info){
    r_struct.bus_label_font_size = info.at("bus_label_font_size"s).AsInt();
    const json::Array& bus_label_offset = info.at("bus_label_offset"s).AsArray();
    r_struct.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};
    r_struct.stop_label_font_size = info.at("stop_label_font_size"s).AsInt();
    const json::Array& stop_label_offset = info.at("stop_label_offset"s).AsArray();
    r_struct.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};
}

void JSONReader::ParseUnderlayer(renderer::RenderSettings& r_struct, json::Dict& info){
    if(info.at("underlayer_color"s).IsString()){
        r_struct.underlayer_color = info.at("underlayer_color"s).AsString();
    }
    if(info.at("underlayer_color"s).IsArray()){
        const json::Array& underlayer_color = info.at("underlayer_color"s).AsArray();
        uint8_t red_ = underlayer_color[0].AsInt();
        uint8_t green_ = underlayer_color[1].AsInt();
        uint8_t blue_ = underlayer_color[2].AsInt();
        if(underlayer_color.size() == 3){
            r_struct.underlayer_color = svg::Rgb(red_, green_, blue_);
        }
        if(underlayer_color.size() == 4){
            double opacity_ = underlayer_color[3].AsDouble();
            r_struct.underlayer_color = svg::Rgba(red_, green_, blue_, opacity_);
        }
    }
    r_struct.underlayer_width = info.at("underlayer_width"s).AsDouble();
}

void JSONReader::ParsePalette(renderer::RenderSettings& r_struct, json::Dict& info){
    const json::Array& color_palette = info.at("color_palette"s).AsArray();
    for(const auto& color : color_palette) {
        if(color.IsString()){
            r_struct.color_palette.push_back(color.AsString());
    }
        if(color.IsArray()){
            const json::Array& colors = color.AsArray();
            uint8_t red_ = colors[0].AsInt();
            uint8_t green_ = colors[1].AsInt();
            uint8_t blue_ = colors[2].AsInt();
            if(colors.size() == 3){
                r_struct.color_palette.push_back(svg::Rgb(red_, green_, blue_));  
            }
            if(colors.size() == 4){
                double opacity_ = colors[3].AsDouble();
                r_struct.color_palette.push_back(svg::Rgba(red_, green_, blue_, opacity_)); 
            }
        }
    }
}

renderer::RenderSettings JSONReader::ParseRenderSettings(){
    renderer::RenderSettings r_struct;
    const json::Node& settings = GetRenderSettings();
    json::Dict info = settings.AsMap();
    ParseFirstPart(r_struct, info);
    ParseLabels(r_struct, info);
    ParseUnderlayer(r_struct, info);
    ParsePalette(r_struct, info);
    return r_struct;
}



json::Dict JSONReader::CreateDictStop(json::Dict& info, const transport::TransportCatalogue& catalogue){
    json::Dict answer;
    if(info.empty()) throw std::logic_error("info is empty");
    int id = info.at("id"s).AsInt();
    if(!info.at("name").IsString()) throw std::logic_error("name is not string");
    std::string_view stop_name = info.at("name").AsString();
    if(catalogue.FindStop(stop_name)){
        json::Array buses;
        for(const auto& bus : catalogue.BusesForStop(catalogue.FindStop(stop_name))){
            buses.push_back(bus);
    }
        answer = json::Builder{}
            .StartDict()
                .Key("id"s)
                .Value(id)
                .Key("buses"s)
                .Value(buses)
            .EndDict()
        .Build().AsMap();
        
    }
    else {
        answer = json::Builder{}
            .StartDict()
                .Key("id"s)
                .Value(id)
                .Key("error_message"s)
                .Value("not found"s)
            .EndDict()
        .Build().AsMap();
    }
    return answer;    
}

json::Dict JSONReader::CreateDictBus(json::Dict& info, const transport::TransportCatalogue& catalogue){
    if(info.empty()) throw std::logic_error("info is empty");
    json::Dict answer;
    int id = info.at("id"s).AsInt();
    if(!info.at("name").IsString()) throw std::logic_error("name is not string");
    auto bus_name = info.at("name").AsString();
    if(!catalogue.FindBus(bus_name)){
        answer = json::Builder{}
            .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("error _message")
                .Value("not found")
            .EndDict()
        .Build().AsMap();
    }

    else {
        auto stats = catalogue.GetBusStats(bus_name);
        answer = json::Builder{}
            .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("route_length"s)
                .Value(stats->route_length)
                .Key("stop_count"s)
                .Value(stats->stops_count)
                .Key("unique_stop_count")
                .Value(stats->unique_stops_count)
                .Key("curvature"s)
                .Value(stats->curvature)
            .EndDict()
        .Build().AsMap();

    }
    return answer;
}

json::Dict JSONReader::CreateRoute(json::Dict& info, const router::TransportRouter& router, const transport::TransportCatalogue& catalogue){
    json::Dict answer;
    int id = info.at("id"s).AsInt();
    auto stop_from = catalogue.FindStop(info.at("from"s).AsString());
    auto stop_to = catalogue.FindStop(info.at("to").AsString());
    auto route_info = router.FindRouteInfo(stop_from, stop_to);
    if(!route_info.has_value()){ 
        answer = json::Builder{}  
            .StartDict()  
                .Key("error_message"s)  
                .Value("not found"s)  
                .Key("request_id"s)  
                .Value(id)  
            .EndDict()  
        .Build().AsMap();  
    }
    else{  
        double total_time = 0.0;
        json::Array items;
        items.reserve(route_info.value().size());
        for(auto el : route_info.value()){
            items.emplace_back(json::Node{json::Builder{}
            .StartDict()
                .Key("type"s)
                .Value("Wait"s)
                .Key("stop_name"s)
                .Value(el.stop_wait)
                .Key("time"s)
                .Value(el.wait_time)
            .EndDict()
            .Build().AsMap()});
                
            items.emplace_back(json::Node{json::Builder{}
            .StartDict()
                .Key("type"s)
                .Value("Bus"s)
                .Key("bus")
                .Value(el.name)
                .Key("span_count"s)
                .Value(el.span_count)
                .Key("time"s)
                .Value(el.time)
            .EndDict().
            Build().AsMap()});
            total_time += el.time;
    }
        answer = json::Builder{}
            .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("total_time"s)
                .Value(total_time)
                .Key("items"s)
                .Value(items)
            .EndDict()
        .Build().AsMap();
    }   
    return answer;
    }

router::RoutingSettings JSONReader::FillRoutingSettings(const json::Dict& request) {
        router::RoutingSettings settings;
        settings.bus_wait_time = request.at("bus_wait_time"s).AsInt();
        settings.bus_velocity = request.at("bus_velocity"s).AsDouble();
        return settings;
    }


json::Dict JSONReader::CreateMap(json::Dict& info, const transport::TransportCatalogue& catalogue, const renderer::MapRenderer& map_renderer){
    if(info.empty()) throw std::logic_error("info is empty");
    json::Dict answer;
    std::string map = map_renderer.RenderBusesMap(catalogue);
    int id = info.at("id"s).AsInt();
    answer = json::Builder{}
        .StartDict()
            .Key("map"s)
            .Value(map)
            .Key("request_id"s)
            .Value(id)
        .EndDict()
    .Build().AsMap();
    return answer;
}


void JSONReader::MakeAndPrint(json::Array requests, const transport::TransportCatalogue& catalogue, const renderer::MapRenderer& map_renderer, const router::TransportRouter& router){
    if(requests.empty()) throw std::logic_error("requests are empty");
    json::Array result;
    for(auto& request : requests){
        json::Dict info = request.AsMap();
        auto type = info.at("type").AsString();
        if(type == "Stop"){
            result.push_back(CreateDictStop(info, catalogue));
        }
        if(type == "Bus"){
            result.push_back(CreateDictBus(info, catalogue));
        }
        if(type == "Map"){
            result.push_back(CreateMap(info, catalogue, map_renderer));
        }
        if(type == "Route"){
            result.push_back(CreateRoute(info, router, catalogue));
        }
    }
    json::Print(json::Document{result}, std::cout);
}
