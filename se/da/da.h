#ifndef __DA_H_
#define __DA_H_

#include <deque>
#include <string>

namespace tis { namespace da {

typedef struct da_input_t {
    std::string query;
} da_input_t;

typedef struct da_output_t {
    std::string query;
    std::deque<std::string> toekn;
    int catalog;
} da_output_t;


class DA {
public:
    DA();
    virtual ~DA();

public:
    void query_analysis(const da_input_t& input, da_output_t* output);

private:

};


}}


#endif
