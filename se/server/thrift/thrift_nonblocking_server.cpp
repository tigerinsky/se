#include "thrift_nonblocking_server.h"
#include "thrift/protocol/TBinaryProtocol.h"
#include "thrift/concurrency/ThreadManager.h"
#include "thrift/concurrency/PosixThreadFactory.h"
#include "thrift/server/TNonblockingServer.h"
#include "glog/logging.h"
#include "../../service/se_service.h"

namespace tis { namespace thrift {


static void _adapt_input(const SeRequest& request,
                         se_input_t* input) {
    char buf[1024];
    input->query = request.query;
    input->pn = request.pn;
    input->rn = request.rn;
    snprintf(buf, 
             1024, 
             "^type(%d,%d)",
             request.type, request.type);
    input->numeric_filter = buf;
    for (auto ite = request.tag.begin(); ite != request.tag.end(); ++ite) {
        input->tag_filter.append("^");
        input->tag_filter.append(ite->c_str()); 
    }
    input->catalog = request.catalog;
}

static void _adapt_response(const se_output_t& output,
                            SeResponse* response) {
    response->err_no = output.err_no;
    response->id =  output.id;
    
    class NumericAttr numeric_attr;
    for (auto ite = output.search_condition.numeric_filter.begin(); ite != output.search_condition.numeric_filter.end(); ++ite) {
        numeric_attr.name = ite->name;
        numeric_attr.low = ite->low;
        numeric_attr.high = ite->high;
        response->search_condition.num_filter.push_back(numeric_attr);
    }
    class TagAttr tag_attr;
    for (auto ite = output.search_condition.tag_filter.begin(); ite != output.search_condition.tag_filter.end(); ++ite) {
        tag_attr.name = ite->tag; 
        response->search_condition.tag_filter.push_back(tag_attr);
    }
    response->catalog.id = output.catalog.id;
    if (response->catalog.id > 0) {
        response->catalog.name = output.catalog.name; 
        class TagGroup group;
        for (auto ite = output.catalog.tag_group.begin(); ite != output.catalog.tag_group.end(); ++ite) {
            group.name = ite->name; 
            group.tag = ite->tag;
            response->catalog.tag_group.push_back(group);
        }
    }
}

void ServerIf::search (SeResponse& response, const SeRequest& request) {
    se_input_t input;
    se_output_t output;
    _adapt_input(request, &input);
    SeService::search(input, &output);   
    _adapt_response(output, &response);
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
    if (_is_stop) {
        LOG(ERROR) << "ThriftNonblockingServer: server has stopped";
        return 5; 
    }

    // 1. thrift server
    try {
        //protocol
        shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
        //register processor
        shared_ptr<ServerIf> handler(new ServerIf());
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
