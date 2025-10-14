#include "transport_router.h" 

namespace router{

    void TransportRouter::AddVertexes(const transport::TransportCatalogue& catalogue){
        graph::VertexId id = 0;
        const auto& all_stops = catalogue.GetAllStops();
        for(const auto& [name, stop] : all_stops){
            vertexes_[stop] = id;
            stops_to_graph_[id] = stop;
            id++;
        }
    }

    void TransportRouter::BuildGraph(const transport::TransportCatalogue& catalogue){
        const double METERS = 1000.0;
        const double MINUTES = 60.0;
        const auto& all_buses = catalogue.GetAllBuses();
        for(const auto& [name, bus] : all_buses){
            const auto& all_stops = bus->stops;
            auto bus_vertex = ParseBusRouteOnVertexes(all_stops.begin(), all_stops.end());
            for(size_t i = 0; i + 1 < bus_vertex.size(); ++i){
                int span_count = 0;
                int distance = 0;
                int back_distance = 0;
                for(size_t j = i + 1; j < bus_vertex.size(); ++j) {
                    distance +=  catalogue.FindDistance(stops_to_graph_.at(bus_vertex[j - 1]), stops_to_graph_.at(bus_vertex[j]));
                    span_count++;
                     graph_.AddEdge({
                    name,
                    span_count,
                    bus_vertex.at(i),
                    bus_vertex.at(j),
                    (static_cast<double>(distance) / (settings_.bus_velocity * METERS / MINUTES)) + settings_.bus_wait_time 
                });

                if(!bus -> is_roundtrip){
                    auto back_bus_vertex = ParseBusRouteOnVertexes(all_stops.rbegin(), all_stops.rend());
                    back_distance += catalogue.FindDistance(stops_to_graph_.at(back_bus_vertex[j - 1]), stops_to_graph_.at(back_bus_vertex[j]));
                    graph_.AddEdge({
                    name,
                    span_count,
                    back_bus_vertex.at(i),
                    back_bus_vertex.at(j),
                    (static_cast<double>(back_distance) / (settings_.bus_velocity * METERS / MINUTES)) + settings_.bus_wait_time 
                });
                }
            }
        }
    }
        
}

    std::optional<std::vector<RouteInfo>> TransportRouter::FindRouteInfo(transport::Stop* from, transport::Stop* to) const {
        std::optional<std::vector<RouteInfo>> result;
        graph::VertexId id_from = vertexes_.at(from);
        graph::VertexId id_to = vertexes_.at(to);
        std::optional<graph::Router<double>::RouteInfo> route_info = router_ -> BuildRoute(id_from, id_to);
        if(!route_info.has_value()){
            return std::nullopt;
        }
        else{
            result.emplace();
            result->reserve(route_info.value().edges.size());
            int wait_time = settings_.bus_wait_time;
            for(auto id : route_info.value().edges){
                auto edge = graph_.GetEdge(id);
                std::string stop_wait = std::string(GetStop(edge.from) -> stop_name);
                result->emplace_back(RouteInfo{std::string(edge.name), edge.span_count, stop_wait, wait_time, edge.weight});
            }
        }
        return result;
    }


    RoutingSettings TransportRouter::GetSettings() const{
        return settings_;
    }
    
    const transport::Stop* TransportRouter::GetStop(graph::VertexId id) const {
        return stops_to_graph_.at(id);
    
    }

    
}
