#include <signal.h>
#include "glog/logging.h"
#include "server/thrift/thrift_nonblocking_server.h"
#include "service/se_service.h"

#include "flag.h"

namespace tis {

DEFINE_int32(port, 9060, "se server port");
DEFINE_int32(thread_num, 15, "thread pool server handler thread num");
DEFINE_string(catalog_info_conf, "./conf/catalog_info.conf", "catalog info conf");

}

tis::Server* g_server = NULL;

void handle_signal(int sig) {
    LOG(INFO) << "why kill me!";
    g_server->stop();
    LOG(INFO) << "kill over!";
}

int main(int argc, char **argv) {
    int ret = -1;
    ::google::ParseCommandLineFlags(&argc, &argv, false);
    ::google::SetUsageMessage("search engine");
    ::google::InitGoogleLogging(argv[0]);

    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);
    signal(SIGPIPE, SIG_IGN);

    g_server = new(std::nothrow) tis::thrift::NonBlockingServer;
    if (!g_server) {
        LOG(ERROR) << "new service, server error"; 
        return 1;
    }
    if (ret = tis::SeService::init()) {
        LOG(ERROR) << "init service error, ret["<< ret << "]";
        return 2;
    }
    g_server->set_thread_num(tis::FLAGS_thread_num);
    g_server->set_port(tis::FLAGS_port);
    if (ret = g_server->run()) {
        LOG(ERROR) << "run se server error, ret["<< ret <<"]";
        return 3;
    }
    return 0;
}
