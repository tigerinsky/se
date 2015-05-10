
#ifndef __STRUCT_H_
#define __STRUCT_H_

namespace tis {

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
}

#endif
