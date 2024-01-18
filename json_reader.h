#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>

class JSONReader {
public:
	JSONReader() = default;

	explicit JSONReader(std::istream&);

	//Вывод ответов на запросы
	void PrintRequestJSON(std::ostream&, const RequestHandler&) const;

	//Заполнение каталога
	transport_catalogue::catalogue::TransportCatalogue BrowseCatalogue() const;

	//Считывание настроек отрисовки и создание отрисовщика
	MapRenderer BrowseRenderer() const;

	//Чтение запросов на заполнение каталога и запросов к каталогу
	void ReadRequestJSON(std::istream&);

	//Считывание настроек маршрутизатора и создание маршрутизатора
	RouterBuilder BrowseRouter() const;

	//Считывание настроек сериализации и слздание сериализатора
	Serializator BrowseSerialization() const;

private:
	json::Document document_;
};