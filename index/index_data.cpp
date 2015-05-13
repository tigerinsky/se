#include "index_data.h"
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "glog/logging.h"
#include "define.h"
#include "file_group_reader.h"
#include "proto/brief.pb.h"
#include "struct.h"

namespace tis {

IndexData::IndexData() {
    _index_reader = NULL;
    _hit_reader = NULL;
    _term_data = NULL;
    _brief_data = NULL;
}

IndexData::~IndexData() {
    if (_index_reader) {
        _index_reader->close(); 
        delete _index_reader;
    }
    if (_hit_reader) {
        _hit_reader->close(); 
        delete _hit_reader;
    }
    if (_term_data) delete _term_data;
    if (_brief_data) delete _brief_data;
}

int IndexData::_load_index(const char* data_path) {
    int ret = -1;
    _index_reader = new(std::nothrow) FileGroupReader;
    if (!_index_reader) {
        LOG(ERROR) << "index data: new index reader error";
        goto fail;
    }
    ret = _index_reader->load(data_path, INDEX_PREFIX);
    if (ret) {
        LOG(ERROR) << "index data: index reader load error, ret["<< ret <<"]";
        goto fail;
    }
    _hit_reader = new(std::nothrow) FileGroupReader;
    if (!_hit_reader) {
        LOG(ERROR) << "index data: new hit reader error";
        goto fail;
    }
    ret = _hit_reader->load(data_path, HIT_PREFIX);
    if (ret) {
        LOG(ERROR) << "index data: hit reader load error, ret["<< ret <<"]";
        goto fail;
    }
    return 0;
fail:
    if (_index_reader) {
        delete _index_reader;
        _index_reader = NULL;
    }
    if (_hit_reader) {
        delete _hit_reader;
        _hit_reader = NULL;
    }
    return 1;
}

int IndexData::_load_term(const char* data_path) {
    int ret = -1;
    char buff[512];
    struct stat st;
    FILE* fp = NULL;
    _term_data = new(std::nothrow) term_data_t;
    if (!_term_data) {
        LOG(ERROR) << "index data: new term data error"; 
        goto fail;
    }
    snprintf(buff, 512, "%s/%s", data_path, TERM_FILE);
    fp = fopen(buff, "r");
    if (!fp) {
        LOG(ERROR) << "index data: open term file error, path["<<buff<<"]"; 
        goto fail;
    }
    ret = fstat(fileno(fp), &st);
    if (ret) {
        LOG(ERROR) << "index data: fstat error, errno["<<errno<<"]"; 
        goto fail;
    }
    if (st.st_size % sizeof(*_term_data)) {
        LOG(ERROR) << "index data: term file size error"; 
        goto fail;
    }
    _term_data->term = (term_t*)malloc(st.st_size);
    if (!_term_data->term) {
        LOG(ERROR) << "index data: malloc term data error, size["<<st.st_size<<"]";
        goto fail;
    }
    ret = fread(_term_data->term, st.st_size, 1, fp);
    if (1 != ret) {
        LOG(ERROR) << "index data: read term file error, errno["<<errno<<"]"; 
        goto fail;
    }
    _term_data->num = st.st_size / sizeof(*_term_data); 
    (void)fclose(fp);
    return 0;
fail:
    if (fp) fclose(fp);
    if (_term_data) {
        delete _term_data;
        _term_data = NULL;
    }
    return 1;
}

int IndexData::_load_brief(const char* data_path) {
    const int BUFF_SIZE = 102400;
    int ret = -1;
    char buff[BUFF_SIZE];
    snprintf(buff, BUFF_SIZE, "%s/%s", data_path, BRIEF_FILE);
    FILE* fp = fopen(buff, "r");
    if (!fp) {
        LOG(ERROR) << "index data: open brief file error, path["<<buff<<"]";
        goto fail;
    }
    _brief_data = new(std::nothrow) brief_data_t;
    if (!_brief_data) {
        LOG(ERROR) << "index data: new brief data error";
        goto fail; 
    }
    int size; 
    while (true) {
        ret = fread(&size, sizeof(size), 1, fp);
        if (0 == ret) {
            if (feof(fp)) {
                break; 
            } else {
                LOG(ERROR) << "index data: read brief error, errno["<<errno<<"]";
                goto fail;
            }
        }
        if (size > BUFF_SIZE) {
            LOG(ERROR) << "index data: brief size too large, size["<<size<<"]"; 
            goto fail;
        }
        ret = fread(buff, size, 1, fp);
        if (1 != ret) {
            LOG(ERROR) << "index data: read brief error, errno["<<errno<<"]"; 
            goto fail;
        }
        Brief* brief = new(std::nothrow) Brief;
        if (!brief) {
            LOG(ERROR) << "index data: new brief error"; 
            goto fail;
        }
        if (!brief->ParseFromArray(buff, size)) {
            LOG(ERROR) << "index data: deserialize brief error"; 
            goto fail;
        }
        _brief_data->brief.push_back(brief);
    }
    (void) fclose(fp);
    return 0;
fail:
    if (fp) fclose(fp);
    if (_brief_data) { 
        delete _brief_data;
        _brief_data = NULL;
    }
    return 1;
}

int IndexData::load(const char* data_path) {
    if (!data_path) return 1;
    if (_load_index(data_path)) return 2;  
    if (_load_term(data_path)) return 3;  
    if (_load_brief(data_path)) return 4;
    _index_path = data_path;
    return 0;
}

const term_t* IndexData::locate_term(uint64_t sign) const {
    uint32_t low = 0;
    uint32_t high = _term_data->num - 1;
    uint32_t mid = (low + high) / 2;
    term_t* term = _term_data->term;
    while (low < high) {
        mid = (low + high) / 2; 
        if (term[mid].sign > sign) {
            if (0 == mid) {
                break; 
            }
            high = mid - 1; 
        } else if (term[mid].sign < sign) {
            low = mid + 1; 
        } else {
            return term + mid; 
        }
    }
    return NULL;
}

const index_t* IndexData::load_index(const term_t* term) const {
    return (index_t*)_index_reader->read(term->file_no, term->offset);
}

const hit_t* IndexData::load_hit(const index_t* index) const {
    return (hit_t*)_hit_reader->read(index->hit_info.file_no, index->hit_info.off);
}

const Brief* IndexData::get_brief(uint32_t obj_id) const {
    if (obj_id >= _brief_data->brief.size()) {
        return NULL; 
    }
    return _brief_data->brief[obj_id];
}
}
