#include "thrift_nonblocking_server.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/concurrency/ThreadManager.h"
#include "thrift/concurrency/PosixThreadFactory.h"
#include "thrift/server/TNonblockingServer.h"
#include "glog/logging.h"
#include "../../service/se_service.h"

namespace tis { namespace thrift {

void ServerIf::search (SeResponse& response, const SeRequest& request) {
    (void)_service->search();   
}

NonBlockingServer::~NonBlockingServer() {
    if (_server) {
        if (!_is_stop) {
            _server->stop(); 
        } 
        delete _server;
    }    
}

int NonBlockingServer::run() {
using boost::shared_ptr;
using apache::thrift::protocol::TProtocolFactory;
using apache::thrift::protocol::TBinaryProtocolFactory;
using apache::thrift::TProcessor;
using apache::thrift::concurrency::ThreadManager;
using apache::thrift::concurrency::PosixThreadFactory;
using apache::thrift::server::TNonblockingServer;
using apache::thrift::TException;
    if (!_se_service) {
        LOG(ERROR) << "ThriftNonblockingServer: empty service";
        return 1; 
    }
    if (_is_stop) {
        LOG(ERROR) << "ThriftNonblockingServer: server has stopped";
        return 5; 
    }

    // 1. thrift server
    try {
        //protocol
        shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
        //register processor
        shared_ptr<ServerIf> handler(new ServerIf(_se_service));
        shared_ptr<TProcessor> processor(new SeServerProcessor(handler));
        // handler thread pool
        shared_ptr<ThreadManager> 
            thread_manager = ThreadManager::newSimpleThreadManager(_thread_num);
        shared_ptr<PosixThreadFactory> thread_factory(new PosixThreadFactory());
        thread_manager->threadFactory(thread_factory);
        thread_manager->start();
        // server
        _server = new(std::nothrow) TNonblockingServer(processor,
                                                       protocol_factory,
                                                       _port,
                                                       thread_manager);
        if (NULL == _server) {
            LOG(ERROR) << "ThriftNonblockingServer: create nonblocking server error";
            return 2;
        }
        _server->serve();
    } catch (TException& ex){
        LOG(ERROR) << "ThriftNonblockingServer: thrift exception: [" 
            << ex.what() << "]";
        return 3;
    } catch (std::exception& ex) {
        LOG(ERROR) << "ThriftNonblockingServer: unknown exception: [" 
            << ex.what() << "]";
        return 4;
    }
    return 0;
}

void NonBlockingServer::stop() {
    if (!_is_stop && _server) { 
        _server->stop(); 
        _is_stop = true;
    }
}

}}
