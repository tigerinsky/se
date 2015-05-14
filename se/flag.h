#ifndef  __FLAG_H_ 
#define  __FLAG_H_

#include "google/gflags.h"

namespace tis {

DECLARE_int32(port);
DECLARE_int32(thread_num);
DECLARE_string(catalog_info_conf);
DECLARE_string(segment_dict);
DECLARE_string(catalog_conf);
DECLARE_string(index_conf);
DECLARE_string(tag_conf);
DECLARE_string(version_file);

}
#endif
