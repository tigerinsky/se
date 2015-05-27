#ifndef __BS_H_
#define __BS_H_

#include "struct.h"

namespace tis { 

class TagDataReader;

namespace bs {

class IndexManager;

class BS {
public:
    static const int RET_OK = 0;
    static const int RET_INIT_THREAD_DATA_ERR = 1;
    static const int MAX_RECALL_NUM = 760;

public:
    BS();
    virtual ~BS();

public:
    int init();
    void basic_search(const bs_input_t& input, bs_output_t* output);

private:
    int _prepare(const bs_input_t& input, bs_output_t* output);
    void _clean();

private:
    IndexManager* _index_manager;
    TagDataReader* _tag_data;
};

}

}

#endif
