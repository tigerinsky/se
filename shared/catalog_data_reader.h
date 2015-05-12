#ifndef __CATALOG_DATA_READER_H_
#define __CATALOG_DATA_READER_H_

#include <unistd.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "define.h"

namespace tis {
typedef struct meta {
    int32_t start;
    int32_t offset;
} meta;

typedef struct catalog_t {
    int32_t id;
    std::string name;
    int32_t parent_id;
    struct catalog_t* parent;
    meta children;
}catalog_t;

typedef struct catalog_list_t {
    std::vector<catalog_t> catalog_list;
}catalog_list_t;

class CatalogDataReader {
    public:
        CatalogDataReader() {
        }
        ~CatalogDataReader() {
        }

        //each line in file:
        //catalog_id\tcatalog_name\tparent_catalog_id
        int load(std::string file_name);

        int get_id(const std::vector<std::string>& seg_list, int32_t* id, int level);

        int get_catalog_name(int32_t id, std::string& name);

        int get_parent_catalog_name(int32_t id, std::string& name);

    private:
        int parse_line(const char* buf, int len);
        int binary_search(const meta& children, const std::string& name);
        int get_id_in_level(const meta& children, std::vector<std::string>& left, int32_t* id, int level);

    private:
        std::unordered_map<std::string, catalog_t*> level_one_map;
        std::unordered_map<int32_t, catalog_t*> id_cata;//id:catalog_t
        std::vector<catalog_t*> catalog_list;

        DISALLOW_COPY_AND_ASSIGN(CatalogDataReader);
};
}

#endif
