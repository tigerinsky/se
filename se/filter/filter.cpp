#include "filter.h"

using std:vector;

namespace tis {
Filter::EnumFilter() {
}

Filter::~EnumFilter() {
}

bool Filter::filter_result(vector<int> tag_id_list, vector<int> condition_list) {
    int isOk = 0;
    int con_size = condition_list.size();
    int tag_size = tag_id_list.size();
    for (int i = 0; i < con_size; i++) {
        isOk = 0;
        for (int j = 0; j < tag_size; j++) {
            if (condition_list[i] == tag_id_list[i]) {
                isOk = 1;
                break;
            }
        }

        if (isOk == 0) {
            return false;
        }
    }

    return true;
}

bool Filter::filter_result(int item_catalog_id, int condition_catalog_id) {
    if (item_catalog_id == condition_catalog_id)
        return true;

    return false;
}

}
