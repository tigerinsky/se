#include <sys/time.h>
#include <unistd.h>

#include "glog/logging.h"

#include "server_handler.h"

using std::endl;

namespace tis {

    int ServerHandler::init() {
        return 0;
    }

    void ServerHandler::search(SeResponse& _return, const SeRequest& request) {
        // Your implementation goes here

    }

    void ServerHandler::heartbeat(EchoResponse& _return, const EchoRequest& request) {
        // Your implementation goes here
    }


}
