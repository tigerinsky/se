#include "index_maker.h"
#include <algorithm>
#include "glog/logging.h"
#include "sign.h"
#include "segment.h"
#include "file_group_writer.h"
#include "string_helper.h"
#include "flag.h"
#include "struct.h"
#include "define.h"

namespace tis {

typedef struct index_node_t {
    index_node_t* next;
    index_t index;
} index_node_t;

typedef struct term_info_t {
    uint64_t sign; 
    uint32_t num;
    index_node_t* index;
} term_info_t;

typedef struct index_info_t {
    uint64_t sign; 
    field_t field;
    uint8_t no;
    uint32_t offset;
} index_info_t;

IndexMaker::IndexMaker() {
    _segment = NULL;
    _obj_num = 0;
    _obj_index_buffer = NULL; 
    _obj_index_buffer_size = 0;
    _obj_index_num = 0;
    _field_num = 0;
    _index_writer = NULL;
    _hit_writer = NULL;
    _term_fp = NULL;
}

IndexMaker::~IndexMaker() {
    if (_segment) delete _segment; 
    if (_index_writer) delete _index_writer; 
    if (_hit_writer) delete _hit_writer; 
    if (_term_fp) (void)fclose(_term_fp);
}

static int load_field_conf(const char* path, 
                           std::unordered_map<std::string, field_t>& map) {
    char buf[512];
    FILE* fp = fopen(path, "r");
    if (!fp) {
        return 1; 
    }
    map.clear();
    while (true) {
        if (!fgets(buf, 511, fp)) {
            if (feof(fp)) {
                break; 
            } else {
                fclose(fp);
                return 2; 
            }
        }
        StringHelper::trim(buf); if ('#' == buf[0] 
                || '\t' == buf[0]) {
            continue; 
        }
        char* p = strchr(buf, '\t');
        char* end = NULL;
        if (!p) {
            continue; 
        }
        *p++ = '\0';
        field_t fid = (field_t)strtol(p, &end, 10); 
        if ('\0' != *end) {
            continue;
        }
        LOG(INFO) << "IndexMaker: add field, name["
            << buf << "] id[" << (int)fid << "]";
        map[buf] = fid;
    }
    (void) fclose(fp);
    return 0;
}

static FILE* init_term_fp(const char* dir, const char* file_name) {
    char buf[512];
    snprintf(buf, 511, "%s/%s", dir, file_name);
    return fopen(buf, "w");
}

int IndexMaker::init() {
    int ret = -1;
    // 1. init segment
    ret = Segment::init(FLAGS_segment_dict.c_str());
    if (ret) {
        Segment::destroy();
        LOG(WARNING) << "IndexMaker: init segment dict error, ret["<<ret<<"]"; 
        goto fail;
    }
    _segment = Segment::create(2048);
    if (!_segment) {
        LOG(WARNING) << "IndexMaker: new segment error";
        goto fail;
    }
    ret = _segment->init(FLAGS_segment_dict.c_str());
    if (ret) {
        LOG(WARNING) << "IndexMaker: init segment error, ret["
            << ret <<"] dict["<< FLAGS_segment_dict.c_str() << "]"; 
        goto fail;
    }
    // 2. init index writer
    _index_writer = new(std::nothrow) FileGroupWriter;
    if (!_index_writer) {
        LOG(WARNING) << "IndexMaker: new index writer error";
        goto fail;
    }
    ret = _index_writer->init(FLAGS_output.c_str(), INDEX_PREFIX);
    if (ret) {
        LOG(WARNING) << "IndexMaker: init index writer error, ret["<<ret<<"]"; 
        goto fail;
    }
    // 3. init hit writer
    _hit_writer = new(std::nothrow) FileGroupWriter;
    if (!_hit_writer) {
        LOG(WARNING) << "IndexMaker: new hit writer error";
        goto fail;
    }
    ret = _hit_writer->init(FLAGS_output.c_str(), HIT_PREFIX);
    if (ret) {
        LOG(WARNING) << "IndexMaker: init hit writer error, ret["<<ret<<"]"; 
        goto fail;
    }
    // 4. init term_fp
    _term_fp = init_term_fp(FLAGS_output.c_str(), TERM_FILE);
    if (!_term_fp) {
        LOG(WARNING) << "IndexMaker: init term fp error";
        goto fail;
    }
    // 5. field map
    ret = load_field_conf(FLAGS_field_conf.c_str(), _field_map);
    if (ret) {
        LOG(WARNING) << "IndexMaker: load field conf error, ret["<<ret<<"]"; 
        goto fail;
    }
    return 0;
fail:
    if (_segment) {
        delete _segment; 
        _segment = NULL;
    }
    if (_index_writer) {
        delete _index_writer;
        _index_writer = NULL;
    }
    if (_hit_writer) {
        delete _hit_writer;
        _hit_writer = NULL;
    }
    if (_term_fp) {
        (void)fclose(_term_fp); 
        _term_fp = NULL;
    }
    return ret;
}

void IndexMaker::start_obj() {
    _field_num = 0;
    _obj_index_num = 0;
}

term_info_t* IndexMaker::__get_term_info(uint64_t sign) {
    term_info_t* term_info = NULL;
    std::unordered_map<uint64_t, term_info_t*>::iterator term_ite;

    term_ite = _term_map.find(sign);
    if (term_ite == _term_map.end()) {
        term_info = new(std::nothrow) term_info_t;
        if (NULL == term_info) {
            return NULL; 
        }
        term_info->sign = sign;
        term_info->num = 0;
        term_info->index = NULL;
        _term_map[sign] = term_info;
        _term_arr.push_back(term_info);
    } else {
        term_info = static_cast<term_info_t*>(term_ite->second); 
    }
    return term_info;
}

index_info_t* IndexMaker::__add_index_info() {
    if (_obj_index_buffer) {
        if (_obj_index_num == _obj_index_buffer_size) {
            uint32_t new_buffer_size = _obj_index_buffer_size * 1.5 + 1;
            index_info_t* new_buffer = (index_info_t*)malloc(new_buffer_size * sizeof(*_obj_index_buffer));  
            if (!new_buffer) {
                return NULL; 
            }
            memcpy(new_buffer, _obj_index_buffer, sizeof(*_obj_index_buffer) * _obj_index_num);
            free(_obj_index_buffer);
            _obj_index_buffer = new_buffer;
            _obj_index_buffer_size = new_buffer_size;
        }
        return _obj_index_buffer + _obj_index_num++;
    } else {
        _obj_index_buffer =  (index_info_t*)malloc(DEFAULT_INDEX_BUFFER_SIZE * sizeof(*_obj_index_buffer)); 
        if (!_obj_index_buffer) {
            return NULL; 
        }
        _obj_index_buffer_size = DEFAULT_INDEX_BUFFER_SIZE;
        _obj_index_num = 1;
        return _obj_index_buffer;
    }
}

int IndexMaker::add_field(const char* field, const char* value) {
    std::vector<token_t> tokens;
    int ret = 0;
    // 1. get fid
    if (!field) {
        LOG(WARNING) << "IndexMaker: field null";  
        return 1;
    }
    //std::unordered_map<std::string, field_t>::iterator field_ite;
    auto field_ite = _field_map.find(field);
    if (_field_map.end() == field_ite) {
        LOG(WARNING) << "IndexMaker: field not exist, field["<<field<<"]";  
        return 1; 
    }
    field_t field_id = field_ite->second;
    // 2. segment
    ret = _segment->segment(value, strlen(value));
    if (ret) {
        LOG(WARNING) << "IndexMaker: segment error, ret["<<ret<<"]" ;  
        return 2;
    }
    // 3. add index 
    _segment->get_all_unique_tokens(tokens);
    //std::vector<token_t>::iterator ite;
    for (auto& ite : tokens) {
        uint64_t sign = sign64_str(ite.str, ite.len); 
        index_info_t* index_info = __add_index_info();
        if (!index_info) {
            return 3; 
        }
        index_info->sign = sign; 
        index_info->field = field_id;
        index_info->no = _field_num;
        index_info->offset = ite.offset;
    }
    ++_field_num;
    return 0;
}

static int index_compare(const void* a, const void* b) {
    const index_info_t* ia = (const index_info_t*)a;
    const index_info_t* ib = (const index_info_t*)b;
    if (ia->sign != ib->sign) {
        return ia->sign < ib->sign ? -1 : 1; 
    } else if (ia->field != ib->field) {
        return ia->field < ib->field ? -1 : 1; 
    } else if (ia->no != ib->no) {
        return ia->no < ib->no ? -1 : 1; 
    } else if (ia->offset != ib->offset) {
        return ia->offset < ib->offset ? -1 : 1; 
    } else {
        return 0; 
    }
}

int IndexMaker::finish_obj() {
    int ret = -1;
    if (!_obj_index_num) {
        return 0; 
    }
    qsort(_obj_index_buffer,
          _obj_index_num,
          sizeof(*_obj_index_buffer),
          index_compare);
    hit_t hit;
    int file_no = 0;
    uint32_t off = 0;
    uint8_t cur_fno = 0;
    uint32_t last_no = 0;
    uint64_t last_sign = 0; 
    field_t last_field = 0;
    index_node_t* index_node = NULL;
    for (int i = 0; i < _obj_index_num; ++i) {
        index_info_t* index_info = _obj_index_buffer + i; 
        if (!index_node) {
            index_node = new(std::nothrow) index_node_t; 
            if (!index_node) {
                LOG(WARNING) << "IndexMaker: create index node error";
                return 1; 
            }
            memset(index_node, 0 , sizeof(*index_node));
            index_node->index.obj_id = _obj_num;
        } else if(index_info->sign != last_sign) {
            term_info_t* term_info = __get_term_info(last_sign);
            if (!term_info) {
                LOG(WARNING) << "IndexMaker: create term info error";
                return 2; 
            }
            ++term_info->num; 
            if (term_info->index) {
                index_node->next = term_info->index; 
                term_info->index = index_node;
            } else {
                term_info->index = index_node; 
            }
            index_node = new(std::nothrow) index_node_t;
            if (!index_node) {
                LOG(WARNING) << "IndexMaker: create index node error";
                return 3; 
            }
            memset(index_node, 0 , sizeof(*index_node));
            index_node->index.obj_id = _obj_num;
        }

        index_node->index.hit_info.field |= index_info->field; 

        if (index_info->field != last_field) {
            cur_fno = 0; 
        } else if (index_info->no != last_no) {
            ++cur_fno; 
        }
        hit.field = index_info->field;
        hit.no = cur_fno;
        hit.no = index_info->offset;
        if (index_info->sign == last_sign) {
            ret = _hit_writer->write(&hit, sizeof(hit), NULL, NULL, true);
            if (ret) {
                LOG(WARNING) << "IndexWriter: write hit error, ret["<<ret<<"]"; 
                return 4;
            }
            index_node->index.hit_info.num++;
        } else {
            ret = _hit_writer->write(&hit, sizeof(hit), &file_no, &off, false);
            if (ret) {
                LOG(WARNING) << "IndexWriter: write hit error, ret["<<ret<<"]"; 
                return 4;
            }
            index_node->index.hit_info.file_no = file_no; 
            index_node->index.hit_info.num = 1;
            index_node->index.hit_info.off = off;
        }
        last_field = index_info->field;
        last_sign = index_info->sign;
        last_no = index_info->no;
    }
    term_info_t* term_info = __get_term_info(last_sign);
    if (!term_info) {
        LOG(WARNING) << "IndexMaker: create term info error";
        return 2; 
    }
    ++term_info->num; 
    if (term_info->index) {
        index_node->next = term_info->index; 
        term_info->index = index_node;
    } else {
        term_info->index = index_node; 
    }

    ++_obj_num;
    return 0;
}

bool TermCompare (term_info_t* a, term_info_t* b) {
    return a->sign < b->sign;
}

int IndexMaker::flush() {
    int ret = -1;
    term_t term;
    std::sort(_term_arr.begin(), _term_arr.end(), TermCompare);
    for (auto& ite : _term_arr) {
        term_info_t* term_info = ite;
        index_node_t* node = term_info->index;
        int index_num = 0;
        int file_no = 0;
        uint32_t offset = 0;
        while (node) {
            if (0 == index_num) {
                ret = _index_writer->write(&(node->index), 
                                           sizeof(node->index), 
                                           &file_no, 
                                           &offset, 
                                           false);
            } else {
                ret = _index_writer->write(&(node->index), 
                                           sizeof(node->index), 
                                           NULL, 
                                           NULL, 
                                           true);
            }
            if (ret) {
                LOG(WARNING) << "IndexMaker: write index error";
                return 1;
            }
            node = node->next;             
            ++index_num;
        }
        if (term_info->num != index_num) {
            LOG(WARNING) << "IndexMaker: index size doesn't match, in_term["
                << term_info->num << "] count["<< index_num << "]";
            return 2;
        }
        term.sign = term_info->sign;
        term.file_no = file_no;
        term.offset = offset;
        term.num = term_info->num;
        ret = fwrite(&term, sizeof(term), 1, _term_fp);
        if (1 != ret) {
            LOG(WARNING) << "IndexMaker: write term error";
            return 3;
        }
    } 
    _hit_writer->finish();
    _index_writer->finish();
    (void)fclose(_term_fp);
    return 0;
}

}

