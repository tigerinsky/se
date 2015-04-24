#ifndef __DEFINE_H_
#define __DEFINE_H_

#define R_ASSERT(EXP, RET)\
    do {\
        if (!(EXP)) {\
            LOG(INFO) << #EXP << " failed " << endl;\
            return RET;\
        }\
    }while(0)

#define TIMEDIFF(s, e)  ((e.tv_sec-s.tv_sec)*1000000 + e.tv_usec - s.tv_usec)

const int MAX_PATH_LEN = 128
#endif

