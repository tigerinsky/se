#include <stdio.h>
#include "glog/logging.h"
#include "tag_data_reader.h"

using std::endl;
namespace tis {
    int TagDataReader::load() {
        string file_name = path_ + "/" + file_;
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

    int TagDataReader::parse_line(const char* buf, int len) {
        char catalog[MAX_CATALOG_NAME_LEN];
        char tag_name[MAX_TAG_NAME_LEN];
        char tag_value[MAX_TAG_VALUE_LEN];
        int32_t id = 0;
        if (3 == sscanf(buf, "%s\t%s\t%s\t%d", catalog, tag_name, tag_value,&id)) {
            string key(catalog);
            key.append(tag_name);
            key.append(tag_value);
            tag_map_[key] = id;
            return 0;
        } else {
            return -1;
        }
    }

    int TagDataReader::get_id(const string& catalog,
            const string& tag_name, const string& tag_value,
            int32_t* id) {
        string key = catalog + tag_name + tag_value;
        TAG_MAP::iterator iter = tag_map_.find(key);
        if (iter != tag_map_.end()) {
            *id = iter->second;
            return 0;
        } else {
            return -1;
        }
    }
}
