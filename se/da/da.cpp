#include "da.h"
#include "../flag.h"
#include "segment.h"
#include "../../index/catalog_data_reader.h"
#include "glog/logging.h"

#include <stdio.h>

//using tis::Segment;
namespace tis { namespace da {

DA::DA() {
    
}

DA::~DA() {
    
}
int DA::init() {
    int ret = -1;
    //init segment
    ret = Segment::init(FLAGS_segment_dict.c_str());
    if (ret) {
        Segment::destroy();
        LOG(WARNING) << "DA: init segment dict error, ret["<<ret<<"]"; 
        goto fail;
    }

    _segment = Segment::create(2048);
    if (!_segment) {
        LOG(WARNING) << "DA: new segment error";
        goto fail;
    }
    ret = _segment->init(FLAGS_segment_dict.c_str());
    if (ret) {
        LOG(WARNING) << "DA: init segment error, ret["
        << ret <<"] dict["<< FLAGS_segment_dict.c_str() << "]";
        goto fail;
    }

    //load catalog dict
    _cata_reader = new CatalogDataReader();
    ret = _cata_reader->load(FLAGS_catalog_dict);
    if (ret != 0) {
        LOG(WARNING) << "DA: load catalog dict error, ret["
        << ret <<"] dict["<< FLAGS_catalog_dict.c_str() << "]";
        goto fail;
    }

    return 0;
fail:
    if (_segment) {
        delete _segment;
        _segment = NULL;
    }

    if (_cata_reader) {
        delete _cata_reader;
        _cata_reader = NULL;
    }

    return ret;
}

void DA::query_analysis(const da_input_t& input, da_output_t* output) {
    std::string query = input.query;
    //1:normalize query
    normalize_query(query);
    output->query = query;

    //2: word seg
    int ret = _segment->segment(query.c_str(), strlen(query.c_str()));
    if (ret) {
        LOG(ERROR) << "DA:segment error, query[" << query << "] ret[" << ret << "]";
        return;
    }
    std::vector<tis::token_t> token;
    _segment->get_cat_tokens(token);
    std::vector<tis::token_t>::iterator ite; 
    for (ite = token.begin(); ite != token.end(); ite++) {
        output->token.push_back(ite->str);
    }

    //3:get catalog id 
    if (output->token.size() == 2) {
        int id;
        printf("in if: \n");
        ret = _cata_reader->get_id(output->token[0], output->token[1], &id);
        printf("ret:%d, id:%d\n", ret, id);
        if (ret == 0) {
            output->catalog = id;
        }
    }
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

        if (query[j] == '\t' || query[j] == '\n' || query[j] == '\r') {//TODO 换成一个配置，需要替换的空白字符待添加
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
