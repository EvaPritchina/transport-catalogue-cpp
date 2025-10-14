#include "json_reader.h" 
#include "transport_catalogue.h" 

 int main() { 
   JSONReader json_input(std::cin); 
    transport::TransportCatalogue catalogue; 
    json_input.ParseCatalogue(catalogue); 
    renderer::RenderSettings r_struct = json_input.ParseRenderSettings(); 
    router::RoutingSettings route_settings = json_input.FillRoutingSettings(json_input.GetRoutingSettings().AsMap());
    const renderer::MapRenderer map_renderer{r_struct}; 
    const router::TransportRouter router{catalogue, route_settings};
    json::Array requests = json_input.GetStateRequest().AsArray(); 
    json_input.MakeAndPrint(requests, catalogue, map_renderer, router); 
  } 
