#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"

#include <fstream>
#include <iostream>

using namespace std;
using namespace transport_catalogue;

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    const std::string_view mode(argv[1]);
    if (mode == "make_base"sv) {
        ifstream fin("make_base_example.json");
        JSONReader json_reader(fin);
        const transport_catalogue::catalogue::TransportCatalogue cat = json_reader.BrowseCatalogue();       
        RequestHandler handler(cat);
        MapRenderer rend = json_reader.BrowseRenderer();
        handler.InitializeRenderer(&rend);
        RouterBuilder router = json_reader.BrowseRouter();
        handler.InitializeRouter(&router);
        Serializator serie = json_reader.BrowseSerialization();
        handler.InitializeSerializator(&serie);
        handler.Serialize();
    }
    else if (mode == "process_requests"sv) {
        ifstream fin("process_requests_example.json");
        JSONReader json_reader(fin);
        Serializator ser = json_reader.BrowseSerialization();
        Serializator::DeserializationResult ds_result = ser.Deserialize();
        const RequestHandler handler(ds_result.catalogue, &ds_result.renderer, &ds_result.router, nullptr);
        ofstream fout;
        fout.open("output.json");
        json_reader.PrintRequestJSON(fout, handler);
    }
    else {
        PrintUsage();
        return 1;
    }
}
