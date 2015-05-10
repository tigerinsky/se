
#ifndef __STRUCT_H_
#define __STRUCT_H_

#include <vector>
#include <deque>
#include <string>

namespace tis {

// --- index -----
typedef uint8_t field_t;

typedef struct hit_t {
    field_t field;
    uint8_t no;
    uint32_t offset;
} hit_t;

typedef struct hit_info_t {
    field_t field;
    uint16_t file_no:5;
    uint16_t num:11; 
    uint32_t off;
} hit_info_t;

typedef struct index_t {
    uint32_t obj_id;
    hit_info_t hit_info;
} index_t;

typedef struct term_t {
    uint64_t sign;
    int8_t file_no;
    int32_t offset;
    int32_t num;
} term_t;
// ---- index end ----

// --- as ----
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
    std::vector<std::string> tag;
} tag_group_t;

typedef struct catalog_info_t {
    int id;
    std::string name;
    std::vector<tag_group_t> tag_group;
} catalog_info_t;

namespace as {
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
}
// --- as end ----

// ---- bs ----
namespace bs {
typedef struct bs_input_t {
    std::string query; 
    std::deque<std::string> token;
    search_condition_t search_condition;
} bs_input_t;

typedef struct bs_output_t {

} bs_output_t;
}


// ---- bs end ----

// ---- da ----
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
}
// ---- da end ----
}

#endif
