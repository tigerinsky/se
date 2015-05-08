#include "makeindex.h"
#include <string>
#include <vector>
#include "glog/logging.h"
#include "index_maker.h"
#include "flag.h"
#include "define.h"
#include "proto/brief.pb.h"

namespace tis {

const int DEFAULT_BUFFER_SIZE = 1024;

IndexMaker* g_index_maker = NULL;
FILE* g_input_fp = NULL;
FILE* g_brief_fp = NULL;
char* g_field_buffer = NULL;
int g_field_buffer_size = 0;
std::string g_proto_buff;

static int init () {
    char path[512];
    g_index_maker = new(std::nothrow) IndexMaker;
    if (!g_index_maker) {
        return 1; 
    }
    if (g_index_maker->init()) {
        return 2; 
    }

    g_input_fp = fopen(FLAGS_input.c_str(), "r");
    if (!g_input_fp) {
        return 3; 
    }

    snprintf(path, 511, "%s/%s", FLAGS_output.c_str(), BRIEF_FILE);
    g_brief_fp = fopen(path, "w");
    if (!g_brief_fp) {
        return 4; 
    }

    g_field_buffer = (char*)malloc(DEFAULT_BUFFER_SIZE);
    if (!g_field_buffer) {
        return 5; 
    }
    g_field_buffer_size = DEFAULT_BUFFER_SIZE;
    return 0;
}

static int next_field(const char** str, char c) {
    const char* p = *str;
    if (NULL == p) {
        return 1; 
    }
    while (true) {
        if (c == *p || '\0' == *p) {
            int len = p - *str + 1; 
            if (g_field_buffer_size < len) { 
                free(g_field_buffer);
                g_field_buffer = (char*)malloc(len);
                if (!g_field_buffer) {
                    return 2; 
                }
                g_field_buffer_size = len;
            }
            memcpy(g_field_buffer, *str, len);
            g_field_buffer[len - 1] = '\0';
            if ('\0' == *p) {
                *str = NULL;
            } else {
                *str = p + 1;
            }
            break;
        } else {
            ++p; 
        }
    }
    return 0;
}

static void split(const char* s, char sep, std::vector<std::string>& list) {
    const char* begin = s;
    const char* end = s;
    while (true) {
        if ('\0' == *end) {
            break; 
        }
        if (sep == *end) {
            if (begin != end) {
                list.push_back(std::string(begin, 0, end - begin)); 
            } 
            begin = end + 1;
        }
        ++end;
    }
}

static int write_brief(Brief& brief) {
    if (!brief.SerializeToString(&g_proto_buff)) {
        return 1;
    }
    int size = g_proto_buff.size();
    if (1 != fwrite(&size, sizeof(size), 1, g_brief_fp)) {
        return 2;
    }
    if (1 != fwrite(g_proto_buff.c_str(), size, 1, g_brief_fp)) {
        return 3; 
    }
    return 0;
}

static int handle_line(const char* line) {
    //id\ttype\tcatalog_id\ttag\tzan_num\tcomment_num\tdesc
    int ret =  -1;
    const char* p = line; 
    Brief brief;

    g_index_maker->start_obj();

    // 1. id 
    if (next_field(&p, '\t')) return 1;
    brief.set_id(atoi(g_field_buffer));
    // 2. type
    if (next_field(&p, '\t')) return 2;
    brief.set_type(atoi(g_field_buffer));
    // 3. catalog_id
    if (next_field(&p, '\t')) return 3;
    int catalog_id = atoi(g_field_buffer);
    brief.set_catalog_id(catalog_id); 
    //todo add catalog_str && parent_catalog_str
    // 4. tag_id 
    if (next_field(&p, '\t')) return 4;
    std::vector<std::string> list;
    split(g_field_buffer, ' ', list);
    for (auto ite = list.begin(); ite != list.end(); ++ite) {
        brief.add_tag_id(atoi(ite->c_str())); 
        //todo add tag str
    }
    // 5. zan_num
    if (next_field(&p, '\t')) return 5;
    brief.set_zan_num(atoi(g_field_buffer));
    // 6. comment_num
    if (next_field(&p, '\t')) return 6;
    brief.set_comment_num(atoi(g_field_buffer));
    // 7. des
    if (next_field(&p, '\t')) return 7;
    ret = g_index_maker->add_field("description", g_field_buffer);
    if (ret) {
        LOG(WARNING) << "makeindex: add desc field error";
        return 7;
    }
    //
    ret = write_brief(brief);
    if (ret) {
        LOG(WARNING) << "makeindex: write brief error"; 
        return 8;
    }
    if (g_index_maker->finish_obj()) {
        return 9; 
    }
    return 0;
}

int makeindex() {
    int ret = 0;
    char buff[10240];
    ret = init();
    if (ret) {
        LOG(WARNING) << "makeindex: init error, ret["<<ret<<"]";
        return 1;
    }
    while (true) {
        if (!fgets(buff, 10240, g_input_fp)) {
            if (feof(g_input_fp)) {
                break; 
            } else {
                return 2; 
            }  
        }
        int len = strlen(buff);
        if ('\n' == buff[len - 1]) {
            buff[len - 1]  = '\0';
        }
        ret = handle_line(buff);
        if (ret) {
            LOG(WARNING) << "makeindex: handle line error, line["
                << buff << "] ret[" << ret << "]";
            return 3;
        }
    }
    if (g_index_maker->flush()) {
        return 4; 
    }
    (void)fclose(g_brief_fp);
    (void)fclose(g_input_fp);
    return 0;
}

}
