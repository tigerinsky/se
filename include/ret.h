#ifndef __RET_H_
#define __RET_H_

namespace tis {

namespace ret {
const int OK = 0;

namespace bs {
const int ERR_NEW_INDEX_MANAGER = 301;
const int ERR_INIT_INDEX_MANAGER = 302;
const int ERR_INIT_THREAD_DATA = 303;
const int ERR_EMPTY_TOKEN = 304;
const int ERR_NEW_TAG_READER = 305;
const int ERR_INIT_TAG_READER = 305;
}

namespace da {
const int ERR_INIT_THREAD_DATA = 201;
const int ERR_INIT_SEGMENT = 202;
const int ERR_INIT_SEGMENT_LOCAL = 203;
const int ERR_INIT_CATALOG_DATA = 204;
const int ERR_SEGMENT = 205;
}

}
}

#endif
