#include "bs.h"
#include <assert.h>
#include <vector>
#include "glog/logging.h"
#include "define.h"
#include "struct.h"
#include "../flag.h"
#include "heap.hpp"
#include "index/index_data.h"
#include "index_manager.h"
#include "ret.h"
#include "sign.h"
#include "proto/brief.pb.h"
#include "shared/tag_data_reader.h"

namespace tis { namespace bs {

typedef struct obj_t {
    int64_t id;
    int obj_id;
    double basic_weight;
} obj_t;

typedef struct index_cursor_t {
    int32_t left;
    const index_t* index; 
} index_cursor_t;

typedef struct term_info_t {
    const char* txt;
    uint64_t sign;
    index_cursor_t index_cursor;
} term_info_t;

class CursorCmp {
public:
bool operator() (term_info_t*& a, term_info_t*& b) const {
    return a->index_cursor.index->obj_id < b->index_cursor.index->obj_id;
}
};

typedef struct thread_data_t {
    IndexData* index_data;
    const bs_input_t* input;
    bs_output_t* output;

    std::vector<obj_t> obj; 
    term_info_t term[BS_MAX_TERM_NUM];
    int term_num;
    Heap<term_info_t*, CursorCmp> index_heap;
    std::vector<int> tag_filt_id;
} thread_data_t;
__thread thread_data_t* gt_thread_data = NULL;

BS::BS() {
    _index_manager = NULL;
    _tag_data = NULL;
}

BS::~BS() {
    if (_index_manager) delete _index_manager;
    if (_tag_data) delete _tag_data;
}

int BS::init() {
    int ret = -1;
    _index_manager = new(std::nothrow) IndexManager;
    if (!_index_manager) {
        return ret::bs::ERR_NEW_INDEX_MANAGER; 
    }
    _index_manager->set_version_file(FLAGS_version_file.c_str());
    ret = _index_manager->init(FLAGS_index_conf.c_str());
    if (ret) {
        return ret::bs::ERR_INIT_INDEX_MANAGER; 
    }
    _tag_data = new(std::nothrow) TagDataReader;
    if (!_tag_data) {
        return ret::bs::ERR_NEW_TAG_READER; 
    }
    if (_tag_data->load(FLAGS_tag_conf)) {
        return ret::bs::ERR_INIT_TAG_READER; 
    }
    return ret::OK;
}

int BS::_prepare(const bs_input_t& input, bs_output_t* output) {
    if (!gt_thread_data) {
        gt_thread_data = new(std::nothrow) thread_data_t; 
        if (!gt_thread_data) return ret::bs::ERR_INIT_THREAD_DATA;
    }
    gt_thread_data->index_data = _index_manager->get_index_data();
    gt_thread_data->obj.clear();
    gt_thread_data->term_num = 0;
    gt_thread_data->index_heap.clear();
    gt_thread_data->input = &input;
    gt_thread_data->output = output;
    output->id.clear();
    gt_thread_data->tag_filt_id.clear(); 
    for (auto& ite : input.search_condition.tag_filter) {
        int tag_id = 0;
        if (!_tag_data->get_id(ite.tag, &tag_id)) {
            gt_thread_data->tag_filt_id.push_back(tag_id); 
        } 
    }
    std::sort(gt_thread_data->tag_filt_id.begin(), gt_thread_data->tag_filt_id.end());
    return ret::OK;
}

void BS::_clean() {
    _index_manager->unref_index_data();
}

static int load_index() {
    const bs_input_t* input = gt_thread_data->input;
    IndexData* index_data = gt_thread_data->index_data;
    int term_num = 0;
    uint64_t sign = 0;
    const term_t* term = NULL;
    int i = 0;
    for (; i < BS_MAX_TERM_NUM && i < input->token.size(); ++i) {
        const std::string& term_txt = input->token[i];
        if (0 == term_txt.size()) {
            continue; 
        } 
        sign = sign64_str(term_txt.c_str(), term_txt.size());
        term = index_data->locate_term(sign);
        if (!term) {
           // LOG(WARNING) << "locate term error, sign:"<<sign << " term:" << term_txt;
            continue; 
        }
        term_info_t& term_info = gt_thread_data->term[term_num];
        term_info.txt = term_txt.c_str();
        term_info.sign = sign;
        term_info.index_cursor.index = index_data->load_index(term);
        term_info.index_cursor.left = term->num;
        ++term_num;
    }
    if (0 == term_num) {
        return ret::bs::EMPTY_TOKEN; 
    }
    gt_thread_data->term_num = term_num;
    if (i < input->token.size()) {
        LOG(WARNING) << "bs: too many terms, token_num["
            <<input->token.size()<<"] load_num["
            <<term_num<<"] max["<<BS_MAX_TERM_NUM<<"]"; 
    }
    return ret::OK;
}

static bool numeric_filter(const Brief* brief, const search_condition_t& condition) {
    //先人工实现
    for (auto& ite : condition.numeric_filter) {
        int value = 0;
        if (0 == strcmp(ite.name.c_str(), "type")) {
            value = brief->type(); 
        } else if (0 == strcmp(ite.name.c_str(), "catalog")) {
            value = brief->catalog_id(); 
        } else {
            continue; 
        }
        if (value < ite.low || value > ite.high) return true; 
    }
    return false;
}

static bool tag_filter(const Brief* brief, const search_condition_t& condition) {
    for (int i = 0, j = 0; i < gt_thread_data->tag_filt_id.size() 
            && j < brief->tag_id_size(); ++i) {
        int filt_tag = gt_thread_data->tag_filt_id[i]; 
        bool found = false;
        for (;j < brief->tag_id_size(); ++j) {
            int brief_tag = brief->tag_id(j);
            if (brief_tag < filt_tag) {
                continue; 
            } else if (brief_tag > filt_tag) {
                return true; 
            } else {
                found = true;
                break; 
            }
        }
        if (found) {
            ++j; 
        } else {
            return true; 
        }
    }
    return false;
}

static bool filt(uint32_t obj_id) {
    const Brief* brief = gt_thread_data->index_data->get_brief(obj_id);
    const search_condition_t& search_condition = gt_thread_data->input->search_condition;
    if (!brief) {
        return true;
    }
    if (numeric_filter(brief, search_condition)) { 
        //LOG(INFO) << "filt by numeric filter: " << obj_id;
        return true; 
    }
    if (tag_filter(brief, search_condition)) {
        //LOG(INFO) << "filt by tag filter: " << obj_id;
        return true;
    }
    return false;
}

static void add_obj(uint32_t obj_id, const hit_info_t& hit_info) {
    const Brief* brief = gt_thread_data->index_data->get_brief(obj_id); 
    obj_t* obj = NULL; 
    int obj_num = gt_thread_data->obj.size();
    assert(brief);
    if (obj_num > 0 && obj_id == gt_thread_data->obj[obj_num - 1].obj_id) {
        obj = &(gt_thread_data->obj[obj_num - 1]);
    } else {
        obj_t o;
        o.id = brief->id();
        o.obj_id = obj_id;
        o.basic_weight = 0.0;
        gt_thread_data->obj.push_back(o);
        obj = &(gt_thread_data->obj[obj_num]);
    }
    int hit_field_num = 0;
    field_t fflag = hit_info.field;
    while (fflag) {
        fflag = fflag & (fflag - 1); 
        ++hit_field_num;
    }
    double hit_score = (hit_field_num * hit_info.num * 0.1);
    hit_score = hit_score > 0.5 ? 0.5 : hit_score;
    obj->basic_weight += (1 + hit_score);
}

bool compare_basic_weight (const obj_t& a, const obj_t& b) {
    return a.basic_weight > b.basic_weight; 
}

static int _recall() {
    int ret = -1;
    ret = load_index();
    if (ret::bs::EMPTY_TOKEN == ret) return ret::OK;
    if (ret::OK != ret) return ret;
    Heap<term_info_t*, CursorCmp>& heap = gt_thread_data->index_heap;
    for (int i = 0; i < gt_thread_data->term_num; ++i) {
        heap.push(&(gt_thread_data->term[i])); 
    }
    uint32_t last_filt_obj = -1;
    uint32_t last_valid_obj = -1;
    while (heap.size() && gt_thread_data->obj.size() < BS::MAX_RECALL_NUM) {
        term_info_t* term_info = heap.pop(); 
        index_cursor_t& cursor = term_info->index_cursor;
        uint32_t obj_id = cursor.index->obj_id;
        if (last_filt_obj == obj_id) {
            goto next;
        }
        if (last_valid_obj != obj_id && filt(obj_id)) {
            last_filt_obj = obj_id; 
            goto next;
        }
        last_valid_obj = obj_id;
        add_obj(obj_id, cursor.index->hit_info);
    next:
        if (--cursor.left) {
            ++cursor.index;
            heap.push(term_info);
        }
    }
    std::stable_sort(gt_thread_data->obj.begin(), 
                     gt_thread_data->obj.end(),
                     compare_basic_weight); 
    return ret::OK;    
}

static int _sort() {
    return ret::OK;
}

static void _copy_result() {
    for (auto& ite : gt_thread_data->obj) {
        gt_thread_data->output->id.push_back(ite.id); 
    }
    gt_thread_data->output->err_no = ret::OK;
}

void BS::basic_search(const bs_input_t& input, bs_output_t* output) {
#define TEST(ret) do { \
    if (ret) {output->err_no = ret; goto end;} \
} while(0);
    TEST(_prepare(input, output));
    TEST(_recall());
    TEST(_sort());
    _copy_result();
end:
    _clean();
}

}}
