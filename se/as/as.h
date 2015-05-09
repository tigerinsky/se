#ifndef __AS_H_
#define __AS_H_

#include <string>
#include <deque>

namespace tis {

typedef struct numeric_filter_t {
    std::string name;
    int id;
    int low;
    int high;
} numeric_filter_t;

typedef struct tag_filter_t {
    std::string tag;
    int id;
} tag_filter_t; 

typedef struct search_condition_t {
    std::deque<numeric_filter_t> numeric_filter;
    std::deque<tag_filter_t> tag_filter;
} search_condition_t;

typedef struct tag_group_t {
    std::string name;
    std::deque<std::string> tag;
} tag_group_t;

typedef struct catalog_info_t {
    int id;
    std::string name;
    std::deque<tag_group_t> tag_group;
} catalog_info_t;

typedef struct as_input_t {
    std::string query;
    int pn;
    int rn; 
    std::string tag_filter;
    std::string numeric_filter;
} as_input_t;

typedef struct as_output_t {
    int errno;
    std::deque<int> id;
    search_condition_t search_condition; 
    catalog_info_t catalog;
} as_output_t;

class AS {
public:
    AS();
    virtual ~AS();

public:
    void search(const as_input_t& input, as_output_t* output);

private:

};

}

#endif
