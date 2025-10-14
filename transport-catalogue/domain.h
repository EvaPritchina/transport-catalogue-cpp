#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

namespace transport {

  struct Bus;

  struct Stop {
    std::string stop_name;
    geo::Coordinates coordinates;
    std::set < std::string > buses;
    bool operator == (const Stop & other) {
      return stop_name == other.stop_name && coordinates == other.coordinates;
    }
  };

  struct Bus {
    std::string bus_name;
    std::vector < Stop * > stops;
    bool is_roundtrip;
    bool operator == (const Bus & other) {
      return bus_name == other.bus_name;
    }
  };

  struct BusStats {
    int stops_count;
    int unique_stops_count;
    int route_length;
    double curvature;

  };

  struct Hasher {
    size_t operator()(std::pair <const Stop *, const Stop *> stops) const {
      return std::hash <const Stop*>()(stops.first) * 37 + std::hash <const Stop*>()(stops.second);
    }
  };
} 
