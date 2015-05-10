#ifndef __BS_H_
#define __BS_H_

#include "struct.h"

namespace tis { namespace bs {

class BS {
public:
    BS();
    virtual ~BS();

public:
    int init();
    void basic_search(const bs_input_t& input, bs_output_t* output);

private:

};

}}

#endif
