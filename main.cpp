// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "glog/logging.h"

#include "include/flag.h"
#include "include/constant.h"

using std::endl;

namespace tis {

DEFINE_int32(server_port, 9020, "se server port");
DEFINE_int32(server_thread_num, 15, "thread pool server handler thread num");

global_data_t g_data;

}

using namespace tis;

void handle_signal(int sig) {
    LOG(INFO) << "why kill me!" << endl;
    g_data.g_server.stop();
    //pthread_cancel(g_data.g_pid);
    //pthread_cancel(g_data.g_pid2);
    LOG(INFO) << "kill over!" << endl;
    exit(0);
}

int main(int argc, char **argv) {
    ::google::ParseCommandLineFlags(&argc, &argv, false);
    ::google::SetUsageMessage("search engine");

    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);
    signal(SIGPIPE, SIG_IGN);

    //pthread_t pid;
    //pthread_create(&(g_data.g_pid), NULL, start_bottom_remind, NULL);
    //pthread_create(&(g_data.g_pid2), NULL, start_msg_remind, NULL);

    if (0 != g_data.g_server.init()) {
        LOG(WARNING) << "init se server wrong!" << endl;
        return 0;
    }

    if (0 != g_data.g_server.run()) {
        LOG(WARNING) << "run se server error!" << endl;
        return 0;
    }

    //pthread_join(g_data.g_pid, NULL);
    //pthread_join(g_data.g_pid2, NULL);
    return 0;
}
