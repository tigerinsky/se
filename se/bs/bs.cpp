#include "bs.h"
#include <vector>
#include "glog/logging.h"
#include "define.h"
#include "struct.h"
#include "../flag.h"
#include "heap.hpp"
#include "index/index_data.h"
#include "index_manager.h"

namespace tis { namespace bs {

typedef struct obj_t {
    int id;
    int obj_id;
    double weight;
} obj_t;

typedef struct index_cursor_t {
    int obj_id; 
} index_cursor_t;

class CursorCmp {
public:
bool operator() (const index_cursor_t& a, index_cursor_t& b) const {
    return a.obj_id > b.obj_id;
}
};

typedef struct thread_data_t {
    obj_t obj[BS_MAX_OBJ_NUM]; 
    
    int obj_num;
    IndexData* index_data;
    Heap<index_cursor_t, CursorCmp> index_heap;
} thread_data_t;
__thread thread_data_t* gt_thread_data = NULL;

BS::BS() {
    _index_manager = NULL;
}

BS::~BS() {
}

int BS::init() {
    int ret = -1;
    _index_manager = new(std::nothrow) IndexManager;
    if (!_index_manager) {
        return 1; 
    }
    ret = _index_manager->init(FLAGS_index_conf.c_str());
    if (ret) {
        return 2; 
    }
    return 0;
}

int BS::_prepare() {
    if (!gt_thread_data) {
        gt_thread_data = new(std::nothrow) thread_data_t; 
        if (!gt_thread_data) return 1;
    }
    gt_thread_data->index_data = _index_manager->get_index_data();
    gt_thread_data->obj_num = 0;
    gt_thread_data->index_heap.clear();
    return 0;
}

void BS::_clean() {
    _index_manager->unref_index_data();
}

static int recall() {
     
    return 0;    
}

void BS::basic_search(const bs_input_t& input, bs_output_t* output) {
    int ret = -1;
    ret = _prepare();
    if (ret) {
        return; 
    }
    _clean();
//    IndexData* get_index_data();
//    void unref_index_data();
}

}}
