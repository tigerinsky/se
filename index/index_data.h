#ifndef __INDEX_DATA_H_
#define __INDEX_DATA_H_

#include <string>
#include <vector>

namespace tis {

class FileGroupReader;
class Brief;
struct term_t;
struct index_t;
struct hit_t;

typedef struct brief_data_t {
    std::vector<Brief*> brief;

    ~brief_data_t() {
        for (auto ite : brief) {
            delete ite; 
        } 
    }
} brief_data_t;

typedef struct term_data_t {
    term_t* term;
    uint32_t num;

    term_data_t() {
        term = NULL; 
        num = 0;
    }

    ~term_data_t() {
        if (term) free(term);
    }
} term_data_t;

class IndexData {
public:
    IndexData();
    virtual ~IndexData();

    const char* get_index_path() const { return _index_path.c_str(); }
    int load(const char* data_path);
    const term_t* locate_term(uint64_t sign) const;
    const index_t* load_index(const term_t* term) const;
    const hit_t* load_hit(const index_t* index) const;
    const Brief* get_brief(uint32_t obj_id) const;

private:
    int _load_index(const char* data_path);
    int _load_term(const char* data_path);
    int _load_brief(const char* data_path);

private:
    std::string _index_path;
    FileGroupReader* _index_reader;
    FileGroupReader* _hit_reader;
    term_data_t* _term_data;
    brief_data_t* _brief_data;
};

}

#endif
