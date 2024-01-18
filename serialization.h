#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <variant>

#include <transport_catalogue.pb.h>

class Serializator {
public:
	struct DeserializationResult {
		transport_catalogue::catalogue::TransportCatalogue catalogue;
		MapRenderer renderer;
		RouterBuilder router;
	};

	Serializator() = default;

	Serializator(std::string);

	void Serialize(const transport_catalogue::catalogue::TransportCatalogue&, const MapRenderer&, const RouterBuilder&);

	DeserializationResult Deserialize();

private:
	std::string filename_;
	transport_catalogue_serialize::TransportCatalogue db_serialization_;

	void SerializeCatalogue(const transport_catalogue::catalogue::TransportCatalogue&);
	void SerializeRenderer(const transport_catalogue::catalogue::TransportCatalogue&, const MapRenderer&);

	transport_catalogue::catalogue::TransportCatalogue DeserializeCatalogue() const;
	MapRenderer DeserializeRenderer(const transport_catalogue::catalogue::TransportCatalogue& cat) const;

	void SerializeColor(svg_serialize::Color* clr, const std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>& source_clr);
	std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba> DeserializeColor(const svg_serialize::Color& source_clr) const;

	void SerializeRouter(const transport_catalogue::catalogue::TransportCatalogue&, const RouterBuilder&);
	RouterBuilder DeserializeRouter(const transport_catalogue::catalogue::TransportCatalogue&);
};
