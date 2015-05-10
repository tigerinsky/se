#ifndef __CATALOG_DATA_READER_H_
#define __CATALOG_DATA_READER_H_

#include <unistd.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "define.h"

namespace tis {
typedef struct catalog_t {
    std::string name;
    int32_t id;
}catalog_t;

typedef struct catalog_list_t {
    std::vector<catalog_t> catalog_list;
}catalog_list_t;

class CatalogDataReader {
    public:
        typedef std::unordered_map<std::string, catalog_list_t> CATA_MAP;
        CatalogDataReader() {
        }
        ~CatalogDataReader() {
        }

        /*int init(const char* path, const char* file) {
            path_.assign(path);
            file_.assign(file);
            return 0;
        }*/

        //each line in file:
        //catalog_id\tcatalog_name\tparent_catalog_id\tparent_catalog_name
        int load(std::string file_name);

        int get_id(const std::string& name1,
                const std::string& name2, int32_t* id);

    private:
        int parse_line(const char* buf, int len);

    private:
        CATA_MAP cata_map_;

        DISALLOW_COPY_AND_ASSIGN(CatalogDataReader);
};
}

#endif
