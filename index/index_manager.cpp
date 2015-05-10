#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "glog/logging.h"

#include "index_manager.h"
#include "define.h"
#include "index_data.h"

namespace tis {

IndexManager::IndexManager() {
    for (int i = 0; i < VERSION_NUM; i++) {
        _index_data[i] = new IndexData();
    }
}

IndexManager::~IndexManager() {
    for (int i = 0; i < VERSION_NUM; i++) {
        delete _index_data[i];
    }
}

int IndexManager::update(const char* index_conf_fname) {
    time_t last_time = _latest_modi_time;   
    time_t curr_time = get_ftime(index_conf_fname);


    if (last_time != curr_time) {
        char index_data_dir_name[MAX_PATH_LEN];
        R_ASSERT(get_data_dir(index_conf_fname, index_data_dir_name) == 0, -1);

        struct timeval start;
        struct timeval end;
        LOG(INFO) << "load index data from " << index_data_dir_name;

        gettimeofday(&start, NULL);
        int new_version = (_latest_version + 1) % VERSION_NUM;
        
        R_ASSERT(_index_data[new_version]->load(index_data_dir_name) == 0, -1);

        gettimeofday(&end, NULL);

        LOG(INFO) << "load index data over! cost[" << TIMEDIFF(start, end) << "] version[" << new_version << "]";

        _latest_modi_time = curr_time;
        _latest_version = new_version;

        //R_ASSERT(save_index_version_sign() == 0, -1);TODO

    }

    return 0;
}

int IndexManager::get_data_dir(const char* conf_fname, char* dir_fname) const {
    FILE* fp = fopen(conf_fname, "r");
    R_ASSERT(fp != NULL, -1);

    fscanf(fp, "%s", dir_fname);
    fclose(fp);

    return 0;
}

time_t IndexManager::get_ftime(const char* fname) {
    struct stat file_stat;
    if (stat(fname, &file_stat) < 0) {
        return 0;
    }

    return file_stat.st_mtime;
}

}
