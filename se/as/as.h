#ifndef __AS_H_
#define __AS_H_

#include <unordered_map>
#include "struct.h" 

namespace tis { namespace as {

class AS {
public:
    AS();
    virtual ~AS();

public:
    int init();
    void advance_search(const as_input_t& input, as_output_t* output);

private:
    std::unordered_map<int, catalog_info_t> _catalog_info_dict;

};

}}

#endif
