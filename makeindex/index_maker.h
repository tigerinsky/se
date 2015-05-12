
#ifndef __INDEX_MAKER_H_
#define __INDEX_MAKER_H_

#include <stdint.h>
#include <unordered_map>

namespace tis {

class Segment;
class FileGroupWriter;

struct term_info_t;
struct index_info_t;
typedef uint8_t field_t;

class IndexMaker{
public:
    static const uint32_t DEFAULT_INDEX_BUFFER_SIZE = 1024;

public:
    IndexMaker();
    virtual ~IndexMaker();
    int init();
    void start_obj();
    int add_field(const char* field, const char* value);
    int finish_obj();
    int flush();

private:
    term_info_t* __get_term_info(uint64_t sign);
    index_info_t* __add_index_info();

private:
    //resource
    Segment* _segment;
    std::unordered_map<std::string, field_t> _field_map; 
    
    //global
    uint32_t _obj_num;
    std::unordered_map<uint64_t, term_info_t*> _term_map;

    //obj
    index_info_t* _obj_index_buffer;
    uint32_t _obj_index_buffer_size; 
    uint32_t _obj_index_num;
    uint32_t _field_num;

    //output
    FileGroupWriter* _index_writer;
    FileGroupWriter* _hit_writer;
    FILE* _term_fp;
};

}

#endif
