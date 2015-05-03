#ifndef __ENUM_FILTER_H_
#define __ENUM_FILTER_H_
#include <vector>

using std:vector;

namespace tis {

class Filter {

public:
    Filter();
    ~Filter();
    bool filter_result(vector<int> tag_id_list, vector<int> condition_list);
    bool filter_result(int item_catalog_id, int condition_catalog_id);
}

}

#endif
