#ifndef __THRIFT_NONBLOCKING_SERVER_H_
#define __THRIFT_NONBLOCKING_SERVER_H_

#include "../server.h"
#include "SeServer.h"

namespace apache { namespace thrift { namespace server {
    class TNonblockingServer;
}}}

namespace tis { namespace thrift {

class NonBlockingServer : public Server {
public:
    NonBlockingServer() : _is_stop(false), _server(NULL) {}
    ~NonBlockingServer();

    int run();
    void stop();

private:
    bool _is_stop;
    apache::thrift::server::TNonblockingServer* _server;
};

class ServerIf : public SeServerIf {
public:
    explicit ServerIf(SeService* s) { _service = s; }
    virtual ~ServerIf() {}
    void search(SeResponse& response, const SeRequest& request);

private:
    SeService* _service;
};

}}

#endif

