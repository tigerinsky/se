#ifndef __UID_SERVER_HANDLER_H_
#define __UID_SERVER_HANDLER_H_

#include <string>
#include <map>
#include <pthread.h>
#include "interface/SeServer.h"

using namespace ::apache::thrift;
using std::string;
using std::map;

namespace tis {

class ServerHandler : virtual public SeServerIf {
public:
    ServerHandler() {
        // Your initialization goes here
    }

    int init();

    void search(SeResponse& _return, const SeRequest& request);

    void heartbeat(EchoResponse& _return, const EchoRequest& request);

private:

};

}
#endif
