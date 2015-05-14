#include "as.h"
#include <sys/time.h>
#include "glog/logging.h"
#include "string_helper.h"
#include "../service/se_service.h"
#include "../flag.h"
#include "ret.h"
#include "define.h"

namespace tis { namespace as {

AS::AS() {

}

AS::~AS() {

}

int AS::init() {
    char buff[1024];
    FILE* fp = fopen(FLAGS_catalog_info_conf.c_str(), "r");
    if (!fp) {
        return 1; 
    }
    catalog_info_t catalog_info;
    tag_group_t tag_group;
    std::vector<std::string> fields;
    while (true) {
        if (!fgets(buff, 1024, fp)) {
            if (feof(fp)) {
                break; 
            } else {
                return 2; 
            }
        }
        StringHelper::split(buff, '\t', &fields);
        if (fields.size() < 3) {
            continue; 
        }
        catalog_info.tag_group.clear();
        catalog_info.id = atoi(fields[0].c_str());
        catalog_info.name = fields[1];
        for (int i = 2; i < fields.size(); ++i) {
            const char* p1 = fields[i].c_str();   
            const char* p2 = strchr(p1, ':');
            if (!p2) {
                continue; 
            }
            tag_group.name.assign(p1, p2 - p1);
            StringHelper::split(p2 + 1, ' ', &(tag_group.tag), false);
            catalog_info.tag_group.push_back(tag_group);
        }
        _catalog_info_dict[catalog_info.id] = catalog_info;
    }
    fclose(fp);
    return 0;
}

static void _fill_tag_filter(const char* str, search_condition_t* condition) {
    const char* begin = str;
    const char* end = NULL;
    tag_filter_t tag_filter;
    while (true) {
        end = strchr(begin, '^');
        if (end) {
            if (end - begin) {
                tag_filter.tag.assign(begin, end - begin);
                condition->tag_filter.push_back(tag_filter); 
            }
            begin = end + 1;
        } else {
            tag_filter.tag.assign(begin); 
            if (tag_filter.tag.size()) {
                condition->tag_filter.push_back(tag_filter); 
            }
            break;
        }
    }
}

static void _fill_numeric_filter(const char* str, search_condition_t* condition) {
    int ret = -1;
    const char* p = str;
    char field[128];
    int low = 0;
    int high = 0;
    numeric_filter_t filter;
    while (true) {
        p = strchr(p, '^');  
        if (!p) {
            break; 
        }
        p = p + 1;
        ret = sscanf(p, "%[^(](%d,%d)", field, &low, &high);
        if (3 != ret) {
            LOG(WARNING) << "as: error numeric filter string, str["
                << str << "] pos["<< p - str << "]"; 
            continue;
        }
        filter.name = field;
        filter.low = low;
        filter.high = high;
        condition->numeric_filter.push_back(filter);
    }
}

void _add_numeric_filter(const char* name, 
                         int low, 
                         int high, 
                         search_condition_t* condition) {
    numeric_filter_t filter; 
    filter.name = name;
    filter.low = low;
    filter.high = high;
    condition->numeric_filter.push_back(filter);
}

void AS::advance_search(const as_input_t& input, as_output_t* output) {
    struct timeval begin; 
    struct timeval end;
    da::da_input_t da_input;
    da::da_output_t da_result;
    da_input.query = input.query;
    (void)gettimeofday(&begin, NULL);
    SeService::call_da_service(da_input, &da_result); 
    (void)gettimeofday(&end, NULL);
    output->da_cost = TIMEDIFF(begin, end);
    if (ret::OK != da_result.err_no) {
        output->err_no = da_result.err_no;
        return;
    }

    bs::bs_input_t bs_input;
    bs::bs_output_t bs_output;
    bs_input.query = da_result.query;
    bs_input.token = da_result.token;
    _fill_tag_filter(input.tag_filter.c_str(), &(bs_input.search_condition));
    _fill_numeric_filter(input.numeric_filter.c_str(), &(bs_input.search_condition));
    // 业务逻辑
    int catalog = -1;
    if (input.catalog > 0) {
        catalog = input.catalog;
    } else if (da_result.catalog > 0) {
        catalog = da_result.catalog;
    }
    if (catalog > 0) {
        _add_numeric_filter("catalog", 
                            catalog, 
                            catalog, 
                            &(bs_input.search_condition)); 
    }
    (void)gettimeofday(&begin, NULL);
    SeService::call_bs_service(bs_input, &bs_output); 
    (void)gettimeofday(&end, NULL);
    output->bs_cost = TIMEDIFF(begin, end);
    if (bs_output.err_no != ret::OK) {
        output->err_no = bs_output.err_no; 
        return;
    }
    output->err_no = ret::OK;
    for (int i = input.pn * input.rn; 
            i < bs_output.id.size() && i < (input.pn + 1) * input.rn; ++i) {
        output->id.push_back(bs_output.id[i]);
    }
    output->total_num = bs_output.id.size();
    output->search_condition = bs_input.search_condition;
    output->catalog.id = catalog;
    if (catalog > 0 && _catalog_info_dict.find(catalog) != _catalog_info_dict.end()) {
        output->catalog = _catalog_info_dict[catalog]; 
    } else {
        output->catalog.id = -1; 
    }
}

}}
