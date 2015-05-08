
#include "flag.h"
#include "glog/logging.h"
#include "makeindex.h"


DEFINE_string(input, "./input", "input file");
DEFINE_string(output, "./output", "output path");
DEFINE_string(conf, "./conf", "conf path");
DEFINE_string(segment_dict, "./conf/seg/utf8", "segment dict");

int main (int argc, char** argv) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    ::google::InitGoogleLogging(argv[0]);
    ::google::SetUsageMessage("makeindex");
    
    LOG(INFO) << "makeindex start"; 
    if (tis::makeindex()) {
        LOG(WARNING) << "makeindex failed";    
    } else {
        LOG(INFO) << "makeindex finish"; 
    }
    return 0;
}
