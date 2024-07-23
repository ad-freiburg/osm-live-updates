#include <iostream>
#include "../include/osm-live-updates/sparql/QueryWriter.h"

#include <string>
#include <vector>

int main() {
    std::string subject = "Hallo";
    olu::sparql::QueryWriter queryWriter;
    auto query = queryWriter.writeDeleteQuery(subject);
    std::cout << query << std::endl;

    std::vector<std::string> triples;
    triples.emplace_back("hallo hallo hallo");
    triples.emplace_back("hallo hallo hallo");
    triples.emplace_back("hallo hallo hallo");

    auto query2 = queryWriter.writeInsertQuery(triples);
    std::cout << query2 << std::endl;

    return 0;
}
