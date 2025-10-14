#pragma once

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"
#include "graph.h"

#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>

namespace router {

    struct RoutingSettings {
        int bus_wait_time;
        double bus_velocity;
        bool operator ==(RoutingSettings settings){
            return bus_wait_time == settings.bus_wait_time && bus_velocity == settings.bus_velocity;
        }
    };

    struct RouteInfo{
        std::string name;
        int span_count;
        std::string stop_wait;
        int wait_time;
        double time;
    };

    
    class TransportRouter {
        public:
        TransportRouter(const transport::TransportCatalogue& catalogue, RoutingSettings settings)
        :settings_(settings)
        {   
            size_t stop_count = catalogue.GetAllStops().size();
            stops_to_graph_.resize(stop_count);
            vertexes_.reserve(stop_count);
            graph_ = graph::DirectedWeightedGraph<double>(stop_count  * 2);
            AddVertexes(catalogue);
            BuildGraph(catalogue);
            router_ = std::make_unique<graph::Router<double>>(graph_);
        }

        std::optional<std::vector<RouteInfo>> FindRouteInfo(transport::Stop* from, transport::Stop* to) const;
        RoutingSettings GetSettings() const;
    
        private:
        void AddVertexes(const transport::TransportCatalogue& catalogue);
        void BuildGraph(const transport::TransportCatalogue& catalogue);
        const transport::Stop* GetStop(graph::VertexId id) const;

        template <typename Iterator>
        std::vector<graph::VertexId> ParseBusRouteOnVertexes(Iterator first, Iterator last){
            std::vector<graph::VertexId> result;
            result.reserve(std::distance(first, last));
            for(auto i = first; i != last; ++i){
            result.push_back(vertexes_.at(*i));
            }
        return result;
        }
        
        RoutingSettings settings_;
        std::vector<const transport::Stop*> stops_to_graph_;
        std::unordered_map<const transport::Stop*, graph::VertexId> vertexes_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;  
    };
    
    
}
