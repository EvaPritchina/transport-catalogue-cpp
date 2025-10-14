#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <numeric>
#include <optional>
#include <map>

namespace transport {

  class TransportCatalogue {
    public:

      void AddStop(std::string_view stop_name, const geo::Coordinates coordinates);
      void AddBus(std::string_view bus_name,
      const std::vector <Stop*> stops, bool is_roundtrip);
      Stop* FindStop(std::string_view stop_name) const;
      Bus* FindBus(std::string_view bus_name) const;
      const std::set <std::string>& BusesForStop(Stop* stop) const;
      void AddDistance(std::pair <const Stop*, const Stop* > dist_pair, int distance);
      int FindDistance(const Stop* from, const Stop* to) const;
      int GetUniqueStops(std::string_view bus_name) const;
      std::map <std::string_view, const Bus*> GetAllBuses() const;
      const std::map<std::string_view, const Stop*> GetAllStops() const;
      std::optional <transport::BusStats> GetBusStats(const std::string bus_name) const;

    private:

      std::unordered_map < std::string_view, Bus* > busname_to_bus;
      std::unordered_map <std::pair <const Stop*, const Stop*>, int, Hasher> stops_distance_;
      std::deque <Stop> stops_;
      std::deque <Bus> buses_;
      std::unordered_map <std::string_view, Stop* > stopname_to_stop;

  };
}
