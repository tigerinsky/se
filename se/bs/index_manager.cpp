#include "index_manager.h"
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "glog/logging.h"
#include "string_helper.h"
#include "define.h"
#include "index/index_data.h"
#include "ret.h"

namespace tis {
namespace bs {

void* switch_index_func(void* arg) {
    IndexManager* manager = static_cast<IndexManager*>(arg); 
    while (true) {
        sleep(IndexManager::SWITCH_SLEEP_TIME);  
        int ret = manager->update();
        if (ret) {
            LOG(ERROR) << "index manager: switch index error, ret["<< ret <<"]"; 
        }
    }
}

__thread uint32_t IndexManager::_thread_version = NO_VERSION;

IndexManager::IndexManager() {
    _latest_version = 0;
}

IndexManager::~IndexManager() {
}

static time_t get_file_mtime(const char* fname) {
    struct stat file_stat;
    if (stat(fname, &file_stat) < 0) {
        return 0;
    }
    return file_stat.st_mtime;
}

static int get_index_path(const char* index_conf, std::string* path) {
    char buff[512];
    FILE* fp = fopen(index_conf, "r");
    if (!fp) {
        (void)fclose(fp);
        return 1; 
    }
    if (!fgets(buff, 512, fp)) {
        (void)fclose(fp);
        return 2;
    }
    path->assign(StringHelper::rtrim(buff));
    return 0;
}

int IndexManager::init(const char* index_conf, bool enable_switch) {
    int ret = -1;
    _index_conf = index_conf;
    ret = update();
    if (ret) {
        LOG(ERROR) << "index manager: init update index error ,ret["
            <<ret<<"] conf["<<index_conf<<"]"; 
        return 1;
    } 
    if (enable_switch) {
        if (pthread_create(&_switch_index_thread, NULL, switch_index_func, this)) {
            LOG(ERROR) << "index manager: create switch thread error";
            return 2; 
        }
    }
    return 0;
}

static void update_version_file (const char* path, const char* index_path) {
    if (!path || '\0' == path[0]) return;
    FILE* fp = fopen(path, "w");
    if (!fp) {
        LOG(WARNING) << "index manager: open version file error, path["<< path <<"]";
        return; 
    }
    (void)fprintf(fp, "%s", index_path);
    (void)fclose(fp);
}

int IndexManager::update() {
    int ret = -1;
    const char* index_conf = _index_conf.c_str();
    time_t last_time = _index[_latest_version].timestamp;   
    time_t curr_time = get_file_mtime(index_conf);
    uint32_t curr_version = _latest_version;
    uint32_t new_version = (_latest_version + 1) % VERSION_NUM;

    if (last_time >= curr_time) {
        return ret::OK; 
    }
    if (_index[new_version].ref) {
        LOG(INFO) << "index manager: new version index is still using, give up, new_version[" 
            << new_version <<"] ref["<< _index[new_version].ref << "]"; 
        return ret::OK; 
    }
    std::string index_path;
    if (get_index_path(index_conf, &index_path)) return ret::bs::ERR_GET_INDEX_PATH; 
    LOG(INFO) << "index manager: start load index, path["
        << index_path.c_str() <<"] last_time["
        << last_time <<"] curr_time["<< curr_time <<"]"; 
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    index_data_t& index = _index[new_version];
    if (!index.data) {
        index.data = new(std::nothrow) IndexData; 
        if (!index.data) return ret::bs::ERR_NEW_INDEX_DATA;
    }
    if (ret = index.data->load(index_path.c_str())) {
        LOG(ERROR) << "index manager: index_data load error, ret["<<ret<<"]";
        return ret::bs::ERR_LOAD_INDEX_DATA; 
    }
    index.ref = 0;
    index.timestamp = curr_time;
    _latest_version = new_version;
    update_version_file(_version_file.c_str(), index_path.c_str());
    gettimeofday(&end, NULL);
    LOG(INFO) << "index manager: load index over! cost[" << TIMEDIFF(start, end) << "] version[" << new_version << "]";
    return 0;
}

IndexData* IndexManager::get_index_data() {
    assert(NO_VERSION == _thread_version);
    uint32_t version = _latest_version;
    ++_index[version].ref;
    _thread_version = version;
    return _index[version].data;
}

void IndexManager::unref_index_data() {
    --_index[_thread_version].ref; 
    _thread_version = NO_VERSION;
}

}
}
