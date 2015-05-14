#ifndef __INDEX_MANAGER_H_
#define __INDEX_MANAGER_H_

#include <string>

namespace tis {

class IndexData;

namespace bs {
typedef struct index_data_t {
    IndexData* data;
    int ref;
    time_t timestamp;
    index_data_t () {
        data = NULL; 
        ref = 0;
        timestamp = 0;
    }
    ~index_data_t() {
        if (data) delete data; 
    }
} index_data_t;

class IndexManager {
public:
    static const int VERSION_NUM = 2;
    static const int SWITCH_SLEEP_TIME = 5;
    static const uint32_t NO_VERSION = static_cast<uint32_t>(-1);

private:
    static __thread uint32_t _thread_version;
    
public:
    IndexManager();
    virtual ~IndexManager();

public:
    int init(const char* index_conf, bool enable_switch = true);
    void set_version_conf(const char* version_conf) { _version_conf = version_conf; }
    int update();//update index
    IndexData* get_index_data();
    void unref_index_data();


private:
    std::string _index_conf;
    std::string _version_conf;
    index_data_t _index[VERSION_NUM];
    uint32_t _latest_version; 
    pthread_t _switch_index_thread;

};//clas IndexManager
}

}//namespace tis

#endif
