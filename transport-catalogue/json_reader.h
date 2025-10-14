#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <iomanip>
#include <iostream>

class JSONReader{
public:
  JSONReader(std::istream& input)
  :input_(json::Load(input)){}

  const json::Node& GetBaseRequest();
  const json::Node& GetStateRequest();
  const json::Node& GetRenderSettings();
  const json::Node& GetRoutingSettings();

  void ParseCatalogue(transport::TransportCatalogue& catalogue);
  renderer::RenderSettings ParseRenderSettings();
  json::Dict CreateDictStop(json::Dict& info, const transport::TransportCatalogue& catalogue);
  json::Dict CreateDictBus(json::Dict& info, const transport::TransportCatalogue& catalogue);
  json::Dict CreateRoute(json::Dict& info, const router::TransportRouter& router, const transport::TransportCatalogue& catalogue);
  json::Dict CreateMap(json::Dict& info, const transport::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer);
  router::RoutingSettings FillRoutingSettings(const json::Dict& request);
  void MakeAndPrint(json::Array requests, const transport::TransportCatalogue& catalogue, const renderer::MapRenderer& map_renderer, const router::TransportRouter& router);

private:

  void ParseFirstPart(renderer::RenderSettings& r_struct, json::Dict& info);
  void ParseLabels(renderer::RenderSettings& r_struct, json::Dict& info)ж
  void ParseUnderlayer(renderer::RenderSettings& r_struct, json::Dict& info);
  void ParsePalette(renderer::RenderSettings& r_struct, json::Dict& info);
  json::Document input_;
  json::Node value_ = nullptr;
};
