#ifndef __DA_H_
#define __DA_H_

#include "struct.h"

namespace tis { namespace da {

class DA {
public:
    DA();
    virtual ~DA();

public:
    int init();
    void query_analysis(const da_input_t& input, da_output_t* output);

private:

};


}}


#endif
