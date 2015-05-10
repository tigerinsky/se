#ifndef __SERVER_H_
#define __SERVER_H_

#include "stdlib.h"

namespace tis {

class SeService;

class Server {
public:
    static const int DEFAULT_THREAD_NUM = 5;
    static const int DEFAULT_PORT = 9523;

public:
    Server() : 
        _thread_num(DEFAULT_THREAD_NUM), 
        _port(DEFAULT_PORT){}
    virtual ~Server() {}

public:
    void set_thread_num(int num) { _thread_num = num; }
    int get_thread_num() { return _thread_num; }
    void set_port(int port) { _port = port; }
    int get_port() { return _port; }

public:
    virtual int run() = 0;
    virtual void stop() = 0;

protected:
    int _thread_num;
    int _port;
};

}

#endif

