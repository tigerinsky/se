#ifndef __CATALOG_DATA_READER_H_
#define __CATALOG_DATA_READER_H_

#include <unistd.h>
#include <string>
#include <map>
#include "define.h"

using std::string;
using std::map;

namespace tis {
class CatalogDataReader {
    public:
        typedef map<string, int32_t> CATA_MAP;
        CatalogDataReader() {
        }
        ~CatalogDataReader() {
        }

        int init(const char* path, const char* file) {
            path_.assign(path);
            file_.assign(file);
            return 0;
        }

        //each line in file:
        //parent_catalog \t catalog \t catalog_id
        int load();

        int get_id(const string& parent_catalog,
                const string& catalog, int32_t* id);

    private:
        int parse_line(const char* buf, int len);

    private:
        string   path_;
        string   file_;
        CATA_MAP cata_map_;

        DISALLOW_COPY_AND_ASSIGN(CatalogDataReader);
};
}

#endif
