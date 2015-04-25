
#ifndef __STRUCT_H_
#define __STRUCT_H_


namespace tis {

typedef struct index_t {
    int32_t obj_id;
    uint8_t field;
} index_t;

typedef struct term_t {
    uint64_t sign;
    int8_t file_no;
    int32_t offset;
    int32_t num;
} term_t;

}

#endif
