#include "se_service.h"
#include <assert.h> 
#include "../flag.h"
#include "glog/logging.h"

namespace tis {

as::AS* SeService::_as = NULL;
bs::BS* SeService::_bs = NULL;
da::DA* SeService::_da = NULL;

void SeService::call_as_service(const as::as_input_t& input,
                                as::as_output_t* output) {
    assert(_as);
    _as->advance_search(input, output);
}

void SeService::call_bs_service(const bs::bs_input_t& input,
                                bs::bs_output_t* output) {
    assert(_bs);
    _bs->basic_search(input, output);
}

void SeService::call_da_service(const da::da_input_t& input,
                                da::da_output_t* output) {
    assert(_da);
    _da->query_analysis(input, output);
}

int SeService::init() {
    int ret = -1;
    _as = new(std::nothrow) as::AS;
    if (!_as) {
        LOG(ERROR) << "se_service: new as error";
        return 1; 
    }
    ret = _as->init(); 
    if (ret) {
        LOG(ERROR) << "se_service: init as error, ret["<< ret <<"]";
        return 2; 
    }
    _bs = new(std::nothrow) bs::BS;
    if (!_bs) {
        LOG(ERROR) << "se_service: new bs error";
        return 3; 
    }
    ret = _bs->init();
    if (ret) {
        LOG(ERROR) << "se_service: init bs error, ret["<< ret <<"]";
        return 4; 
    }
    _da = new(std::nothrow) da::DA;
    if (!_da) {
        LOG(ERROR) << "se_service: new da error";
        return 5; 
    }
    ret = _da->init();
    if (ret) {
        LOG(ERROR) << "se_service: init da error, ret["<< ret <<"]";
        return 6; 
    }
    return 0;
}

void SeService::search(const se_input_t& input, se_output_t* output) {
    call_as_service(input, output);
}

}
