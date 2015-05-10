#ifndef __DA_H_
#define __DA_H_

//#include <deque>
#include <string>
#include <vector>

namespace tis { 
class Segment;
class CatalogDataReader;

namespace da {
typedef struct da_input_t {
    std::string query;
} da_input_t;

typedef struct da_output_t {
    std::string query;
    //std::deque<std::string> token;
    std::vector<std::string> token;
    int catalog;
} da_output_t;


class DA {
public:
    DA();
    virtual ~DA();

public:
    void query_analysis(const da_input_t& input, da_output_t* output);
    int init();

private:
    void normalize_query(std::string &query);

private:
    Segment* _segment;
    CatalogDataReader *_cata_reader;

};


}
}


#endif
