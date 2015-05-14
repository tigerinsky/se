#include <ctype.h>
#include <unordered_map>

#include "da.h"
#include "../flag.h"
#include "segment.h"
#include "../../shared/catalog_data_reader.h"
#include "glog/logging.h"
#include "ret.h"

namespace tis { namespace da {

typedef struct thread_data_t {
    Segment* segment;
} thread_data_t;

__thread thread_data_t* gt_thread_data = NULL;

DA::~DA() {
    if (_cata_reader) {delete _cata_reader;}    
}
int DA::init() {
    int ret = -1;
    //init segment
    ret = Segment::init(FLAGS_segment_dict.c_str());
    if (ret) {
        Segment::destroy();
        LOG(WARNING) << "DA: init segment dict error, ret["<<ret<<"]"; 
        ret = ret::da::ERR_INIT_SEGMENT;
        goto fail;
    }

    //load catalog dict
    _cata_reader = new CatalogDataReader();
    ret = _cata_reader->load(FLAGS_catalog_conf);
    if (ret != 0) {
        LOG(WARNING) << "DA: load catalog dict error, ret["
        << ret <<"] dict["<< FLAGS_catalog_conf.c_str() << "]";
        ret = ret::da::ERR_INIT_CATALOG_DATA;
        goto fail;
    }

    return 0;
fail:
    if (_cata_reader) {
        delete _cata_reader;
        _cata_reader = NULL;
    }
    return ret;
}

static int prepare() {
    if (!gt_thread_data) {
        gt_thread_data = new(std::nothrow) thread_data_t; 
        if (!gt_thread_data) return ret::da::ERR_INIT_THREAD_DATA;  
        gt_thread_data->segment = Segment::create(1024);
        if (!gt_thread_data->segment) return ret::da::ERR_INIT_SEGMENT_LOCAL;
    }
    return ret::OK;
}

void DA::query_analysis(const da_input_t& input, da_output_t* output) {
    int ret = -1;
    if (ret::OK != (ret = prepare())) {
        output->err_no = ret; 
        return;
    }

    //1:normalize query
    std::string query = input.query;
    normalize_query(query);
    output->query = query;

    //2: word seg
    Segment* segment = gt_thread_data->segment;
    ret = segment->segment(query.c_str(), strlen(query.c_str()));
    if (ret) {
        LOG(ERROR) << "DA:segment error, query[" << query << "] ret[" << ret << "]";
        output->err_no = ret::da::ERR_SEGMENT;
        return;
    }
    std::vector<tis::token_t> token;
    segment->get_cat_tokens(token);
    std::vector<tis::token_t>::iterator ite; 
    std::unordered_map<std::string, int16_t> token_map;  //为了去重
    for (ite = token.begin(); ite != token.end(); ite++) {
        auto it = token_map.find(ite->str);
        if (it == token_map.end()) {
            output->token.push_back(ite->str);
            token_map[ite->str] = 1;
        }
    }

    //3:get catalog id 
    int id;
    ret = _cata_reader->get_id(output->token, &id, 1);
    output->catalog = -1;
    if (ret == 0) {
        output->catalog = id;
    }
    output->err_no = ret::OK;
}

/**将空白字符统一替换成空格，且去除多余空格
 */
void DA::normalize_query(std::string &query) {
    int i = -1;
    int j = 0;
    int size = query.size();
    
    while (j < size) {
        if (query[j] >= 'A' && query[j] <='Z') {
            query[j] = query[j]+32;
        }

        if (isspace(query[j])) {
            query[j] = ' ';
        }

        if (query[j] == ' ' && query[i] == ' ') {
            j++;
        }
        else {
            i++;
            query[i] = query[j];
            j++;
        }
    }

    query = query.substr(0, i+1);
}

}
}//namespace
