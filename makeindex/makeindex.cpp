#include "makeindex.h"
#include <string>
#include <vector>
#include <algorithm>
#include "glog/logging.h"
#include "index_maker.h"
#include "flag.h"
#include "define.h"
#include "proto/brief.pb.h"
#include "../shared/catalog_data_reader.h"
#include "../shared/tag_data_reader.h"
#include "json.h"
#include "exception.h"

using namespace tis::json;

namespace tis {

const int DEFAULT_BUFFER_SIZE = 1024;

IndexMaker* g_index_maker = NULL;
FILE* g_input_fp = NULL;
FILE* g_brief_fp = NULL;
char* g_field_buffer = NULL;
int g_field_buffer_size = 0;
std::string g_proto_buff;
CatalogDataReader* g_catalog_reader = NULL;
TagDataReader* g_tag_reader = NULL;


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

    g_catalog_reader = new(std::nothrow) CatalogDataReader;
    if (!g_catalog_reader) {
        return 6;
    }
    int ret = g_catalog_reader->load(FLAGS_catalog_dict);
    if (ret != 0) {
        return 7;
    }

    g_tag_reader = new (std::nothrow) TagDataReader;
    if (!g_tag_reader) {
        return 8;
    }
    ret =  g_tag_reader->load(FLAGS_tag_dict);
    if (ret != 0) {
        return 9;
    }

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
    if (begin != end) {
        list.push_back(std::string(begin, 0, end - begin)); 
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
    int ret =  -1;
    const char* p = line; 
    Brief brief;

    g_index_maker->start_obj();
    try {
        Json* json = new(std::nothrow) Json;
        JsonObj* obj = json->deserialize(p);
        JsonMap& map = *((JsonMap*)obj);

        //1:id
        int64_t id = 0;
        id = ((JsonNumber*)(map["tid"]))->to_int64();
        brief.set_id(id);
        //2:type
        int type = ((JsonNumber*)(map["type"]))->to_int64();
        brief.set_type(type);
        //3: f_catalog && s_catalog
        const char* f_catalog = ((JsonString*)(map["f_catalog"]))->c_str();
        const char* s_catalog = ((JsonString*)(map["s_catalog"]))->c_str();

        int catalog_id = g_catalog_reader->get_catalog_id(f_catalog, s_catalog);
        if (catalog_id > 0) {
            brief.set_catalog_id(catalog_id);
            if (g_index_maker->add_field("catalog", f_catalog)) {
                LOG(WARNING) << "makeindex: add catalog field error, catalog["
                            << f_catalog << "]";
                return 3;
            }

            if (g_index_maker->add_field("catalog", s_catalog)) {
                LOG(WARNING) << "makeindex: add catalog field error, catalog["
                            << s_catalog << "]";
                return 3;
            }

        } else {
            brief.set_catalog_id(0);
        }

        //4: tag_id
        JsonArray *tag_array = (JsonArray*)map["tag"];
        std::vector<int32_t> tag_id;
        for (int i = 0; i < tag_array->size(); i++) {
            JsonString* tag = (JsonString*)(tag_array->get(i));
            if (g_index_maker->add_field("tag", tag->c_str())) {
                LOG(WARNING) << "makeindex: add tag field error, tag[" << tag << "]";
                return 4;
            }

            int32_t id = 0; 
            ret = g_tag_reader->get_id(tag->c_str(), &id);
            if (!ret) {
                tag_id.push_back(id);
            }
        }

        std::sort(tag_id.begin(), tag_id.end());
        for (auto& ite : tag_id) {
            brief.add_tag_id(ite);
        }

        //5:zan_num
        JsonNumber* zan_num = (JsonNumber*)map["zan_num"];
        brief.set_zan_num(zan_num->to_int64());

        //6:comment_num
        JsonNumber* comment_num = (JsonNumber*)map["comment_num"];
        brief.set_comment_num(comment_num->to_int64());

        //7:des
        JsonArray *desc_array = (JsonArray*)map["desc"];
        for (int i = 0; i < desc_array->size(); i++) {
            JsonString* desc = (JsonString*)(desc_array->get(i));
            ret = g_index_maker->add_field("description", desc->c_str());
            if (ret) {
                LOG(WARNING) << "makindex: add desc field error, desc[" << desc << "]";
                return 7;
            }
        }
    } catch (JsonException& ex) {
        LOG(WARNING) << "handle line exception[" << ex.what() << "]";

        return -1;
    }

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
    int count = 0;
    int fail_count = 0;
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
            ++fail_count;
            continue;
        }
        ++count;
    }
    if (fail_count * 1.0 / count > 0.1) {
        LOG(ERROR) << "makeindex: too many fail record, fail["<<fail_count<<"]";
//return 3; 
    }
    if (g_index_maker->flush()) {
        return 4; 
    }
    LOG(INFO)<<"makeindex: makeindex finish, s_num["<<count<<"] f_num["<<fail_count<<"]";
    (void)fclose(g_brief_fp);
    (void)fclose(g_input_fp);
    return 0;
}

}
