#include <stdio.h>
#include "glog/logging.h"
#include "catalog_data_reader.h"

using std::endl;
namespace tis {
    int CatalogDataReader::load() {
        string file_name = path_ + file_;
        FILE* fp = fopen(file_name.c_str(), "r");
        if (fp == NULL) {
            LOG(ERROR) << "open file " << file_name.c_str() << " file" << endl;
            return -1;
        }

        char buf[MAX_LINE_LEN];
        int len = 0;
        int line_num = 0;
        while (!feof(fp)) {
            line_num++;
            if (fgets(buf, MAX_LINE_LEN, fp) == NULL) { continue; }
            while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n')) {
                buf[len-1] ='\0';
                len--;
            }
            if (len <= 0 || buf[0] == '#') { continue; }
            if (0 != parse_line(buf, MAX_LINE_LEN)) {
                LOG(ERROR) << "line " << line_num << " parse error!" << endl;
            }
        }
        fclose(fp);
        return 0;
    }

    int CatalogDataReader::parse_line(const char* buf, int len) {
        char parent_catalog[MAX_CATALOG_NAME_LEN];
        char catalog[MAX_CATALOG_NAME_LEN];
        int32_t id = 0;
        if (3 == sscanf(buf, "%s\t%s\t%d", parent_catalog, catalog, &id)) {
            string key(parent_catalog);
            key.append(catalog);
            cata_map_[key] = id;
            return 0;
        } else {
            return -1;
        }
    }

    int CatalogDataReader::get_id(const string& parent_catalog,
            const string& catalog, int32_t* id) {
        string key = parent_catalog + catalog;
        CATA_MAP::iterator iter = cata_map_.find(key);
        if (iter != cata_map_.end()) {
            *id = iter->second;
            return 0;
        } else {
            return -1;
        }
    }
}
