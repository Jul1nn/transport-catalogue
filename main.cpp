#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <chrono>

using namespace std;
using namespace transport_catalogue;
using namespace std::chrono;

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
    	auto start = high_resolution_clock::now();
        ifstream fin("s14_3_opentest_3_make_base.json");
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
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
 
    	cerr << "Time taken by make_base: " << duration.count() << " microseconds" << endl;
    }
    else if (mode == "process_requests"sv) {
    	auto start = high_resolution_clock::now();
        ifstream fin("s14_3_opentest_3_process_requests.json");
        JSONReader json_reader(fin);
        Serializator ser = json_reader.BrowseSerialization();
        Serializator::DeserializationResult ds_result = ser.Deserialize();
        const RequestHandler handler(ds_result.catalogue, &ds_result.renderer, &ds_result.router, nullptr);
        ofstream fout;
        fout.open("output.json");
        json_reader.PrintRequestJSON(fout, handler);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
 
    	cerr << "Time taken by process_requests: " << duration.count() << " microseconds" << endl;
    }
    else {
        PrintUsage();
        return 1;
    }
}
