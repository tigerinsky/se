#ifndef __DA_H_
#define __DA_H_

//#include <deque>
#include <string>
#include <vector>
#include "struct.h"

namespace tis { 
class Segment;
class CatalogDataReader;

namespace da {
class DA {
public:
    DA();
    virtual ~DA();

public:
    int init();
    void query_analysis(const da_input_t& input, da_output_t* output);

private:
    void normalize_query(std::string &query);

private:
    Segment* _segment;
    CatalogDataReader *_cata_reader;

};


}
}


#endif
