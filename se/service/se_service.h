#ifndef __SE_SERVICE_
#define __SE_SERVICE_

#include "../as/as.h"
#include "../bs/bs.h"
#include "../da/da.h"

namespace tis {

typedef as::as_input_t se_input_t ;
typedef as::as_output_t se_output_t;

class SeService {
public:
    static int init();
    static void call_as_service(const as::as_input_t& input, 
                                as::as_output_t* output);
    static void call_bs_service(const bs::bs_input_t& input,
                                bs::bs_output_t* output); 
    static void call_da_service(const da::da_input_t& input,
                                da::da_output_t* output); 
    static void search(const se_input_t& input, se_output_t* output);

private:
    static as::AS* _as;
    static bs::BS* _bs;
    static da::DA* _da;

private:
    SeService() {}
    virtual ~SeService() {}
   

};

}

#endif
