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

#define DISALLOW_COPY_AND_ASSIGN(TypeName)\
    TypeName(const TypeName&);\
    TypeName& operator=(const TypeName&)

const int MAX_PATH_LEN = 128;
const int MAX_LINE_LEN = 1024;
const int MAX_CATALOG_NAME_LEN = 128;
const int MAX_TAG_NAME_LEN = 128;
const int MAX_TAG_VALUE_LEN = 128;

const char INDEX_PREFIX[] = "index"; 
const char HIT_PREFIX[] = "hit";
const char TERM_FILE[] = "term";
const char BRIEF_FILE[] = "brief";
const char FIELD_CONF[] = "field.conf";

#endif

