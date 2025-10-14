#include "transport_catalogue.h"

namespace transport {

  void TransportCatalogue::AddStop(std::string_view stop_name, const geo::Coordinates coordinates) {
    stops_.push_back({std::string(stop_name), coordinates, {}});
    stopname_to_stop[stops_.back().stop_name] = & stops_.back();
  }

  void TransportCatalogue::AddBus(std::string_view bus_name, const std::vector <Stop*> stops, bool is_roundtrip) {
    buses_.push_back({std::string(bus_name), stops, is_roundtrip});
    busname_to_bus[buses_.back().bus_name] = &buses_.back();
    for (const auto& stop: stops) {
      for (auto& stop_: stops_) {
        if (stop_.stop_name == stop -> stop_name) {
          stop_.buses.insert(std::string(bus_name));
        }
      }
    }
  }

  Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
    auto it = stopname_to_stop.find(stop_name);
    if (it != nullptr) {
      return it -> second;
    }
    return nullptr;
  }

  Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
    auto it = busname_to_bus.find(bus_name);
    if (it != nullptr) {
      return it -> second;
    }
    return nullptr;
  }

  void TransportCatalogue::AddDistance(std::pair<const Stop*, const Stop*> dist_pair, int distance) {
    stops_distance_.insert({dist_pair, istance});
  }

  int TransportCatalogue::FindDistance(const Stop* from, const Stop* to) const {
    if (stops_distance_.count({from, to})) return stops_distance_.at({from, to});
    else if (stops_distance_.count({to, from })) return stops_distance_.at({to, from});
    else return 0;
  }

  int TransportCatalogue::GetUniqueStops(std::string_view bus_name) const {
    std::unordered_set < transport::Stop*> unique_stops;
    for (const auto& stop: busname_to_bus.at(bus_name) -> stops) {
      unique_stops.insert(stop);
    }
    return unique_stops.size();
  }

  const std::set <std::string>& TransportCatalogue::BusesForStop(Stop* stop) const {
    return stop -> buses;
  }

  std::map < std::string_view, const Bus* > TransportCatalogue::GetAllBuses() const {
    std::map < std::string_view, const Bus* > result;
    for (const auto & [name, bus]: busname_to_bus) {
      result.insert({name, bus});
    }
    return result;
  }

  const std::map<std::string_view, const Stop*> TransportCatalogue::GetAllStops() const {
    std::map<std::string_view, const Stop*> result;
    for(const auto& [name, stop] : stopname_to_stop){
      result.insert({name, stop});
    }
    return result;
  }

  std::optional <transport::BusStats> TransportCatalogue::GetBusStats(const std::string bus_name) const {
    transport::BusStats result {};
    transport::Bus* bus = FindBus(bus_name);
    if (!bus) throw std::invalid_argument("bus not found");
    if (bus -> is_roundtrip) result.stops_count = bus -> stops.size();
    else result.stops_count = bus -> stops.size() * 2 - 1;

    int route_length = 0;
    double geographic_length = 0.0;

    for (size_t i = 0; i < bus -> stops.size() - 1; ++i) {
      auto from = bus -> stops[i];
      auto to = bus -> stops[i + 1];
      if (bus -> is_roundtrip) {
        route_length += FindDistance(from, to);
        geographic_length += geo::ComputeDistance(from -> coordinates, to -> coordinates);
      } 
      else {
        route_length += FindDistance(from, to) + FindDistance(to, from);
        geographic_length += geo::ComputeDistance(from -> coordinates, to -> coordinates) * 2;
      }
    }
    result.unique_stops_count = GetUniqueStops(bus_name);
    result.route_length = route_length;
    result.curvature = route_length / geographic_length;
    return result;
  }

}
