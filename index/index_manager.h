#ifndef __INDEX_MANAGER_H_
#define __INDEX_MANAGER_H_

namespace tis {
class IndexData;

class IndexManager {
public:
    IndexManager();
    ~IndexManager();

    int update(const char* fname);//update index
    
    int sync_version();//sync current thread version pointer to latest version data

    bool get_latest_index_version(int64_t &s_time, int64_t &e_time, uint64_t &ver);//get latest index version info

    bool get_thread_index_version(int64_t &s_time, int64_t &e_time, uint64_t &ver);

    //TODO get data detail

private:
    int get_data_dir(const char* conf_fname, char* dir_fname) const;
    int save_index_version_sign();

    time_t get_ftime(const char* fname);

private:
    static const int VERSION_NUM = 2;
    IndexData* _index_data[VERSION_NUM];
    time_t     _latest_modi_time; 
    int        _latest_version; 
    static __thread IndexData* _version_ptr;
};//clas IndexManager

}//namespace tis

#endif
