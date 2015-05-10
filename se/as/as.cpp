#include "as.h"
#include "../service/se_service.h"

namespace tis { namespace as {

AS::AS() {

}

AS::~AS() {

}

int AS::init() {
    return 0;
}

void AS::advance_search(const as_input_t& input, as_output_t* output) {
    da::da_input_t da_input;
    da::da_output_t da_output;
    da_input.query = input.query;
    SeService::call_da_service(da_input, &da_output); 

    bs::bs_input_t bs_input;
    bs::bs_output_t bs_output;
    SeService::call_bs_service(bs_input, &bs_output); 
}


}}
