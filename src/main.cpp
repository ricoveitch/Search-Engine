#include <iostream>
#include "search.h"
#include "indexer.h"
#include "server_socket.h"

void help(){
    std::cerr << "USAGE: ./search" << " <option> <file>\n"
        << "Options and their files:\n"
        << "-i <path to wsj.xml file> -- to preform indexing on the wsj.xml file\n"
        << "-s <query test file> -- to search for a query\n";
}

int main(int argc, char** argv) {
    SE::Indexer indexer;
    SE::Search search;
    if(argc <= 1){
        help();
        return EXIT_FAILURE;
    }
    std::string option = argv[1];
    if(option == "-i"){
        if(argc != 2){
            help();
            return EXIT_FAILURE;
        }
        indexer.index();
    }else if(option == "-s"){ 
        SE::Server_Socket server_socket;
        if(!server_socket.set_up())
            return EXIT_FAILURE;
        char *request;
        std::string raw_query;
        std::string request_type;
        while((request = server_socket.accept_client()) != NULL){
            raw_query = std::string(request);
            request_type = raw_query.substr(0, 2);
            raw_query.erase(0, 2);

            if(request_type == "-d"){
                uint64_t doc_id = static_cast<uint64_t>(std::stoull(raw_query));
                std::string doc_contents = search.get_doc_content(doc_id, false);
                server_socket.send_client(doc_contents);
            }else{
                std::istringstream iss(raw_query);
                std::string query = "";
                std::string token;
                while(iss >> token){
                    if(indexer.stop_words.find(token) == indexer.stop_words.end()){
                        query += token + " ";
                    }
                }
                std::vector<uint64_t> doc_ids = search.search(query);
                if(doc_ids.empty()){
                    std::string no_results = "No Results Found\n";
                    server_socket.send_client(no_results);
                }else{
                    std::string result = "";
                    for(uint64_t id : doc_ids){
                        result += std::to_string(id);
                        result += search.get_doc_content(id, true) + "\n";
                    }
                    server_socket.send_client(result);
                }
            }
        }

    }else{
        help();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}