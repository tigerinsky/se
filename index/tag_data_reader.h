#ifndef __TAG_DATA_READER_H_
#define __TAG_DATA_READER_H_

#include <unistd.h>
#include <string>
#include <map>
#include "define.h"

using std::string;
using std::map;

namespace tis {
class TagDataReader {
    public:
        typedef map<string, int32_t> TAG_MAP;
        TagDataReader() {
        }
        ~TagDataReader() {
        }

        int init(const char* path, const char* file) {
            path_.assign(path);
            file_.assign(file);
            return 0;
        }

        //each line in file:
        //catalog \t tag_name \t tag_value \t tag_id
        int load();

        int get_id(const string& catalog, const string& tag_name,
                const string& tag_value, int32_t* id);

    private:
        int parse_line(const char* buf, int len);

    private:
        string   path_;
        string   file_;
        TAG_MAP  tag_map_;

        DISALLOW_COPY_AND_ASSIGN(TagDataReader);
};
}

#endif
