#ifndef __AS_H_
#define __AS_H_

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

};

}}

#endif
