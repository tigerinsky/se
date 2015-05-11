#include <stdio.h>
#include "glog/logging.h"
#include "catalog_data_reader.h"

using std::endl;
namespace tis {
    int CatalogDataReader::load(std::string file_name) {
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
            len = strlen(buf);
            while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n')) {
                buf[len-1] ='\0';
                len--;
            }
            //printf("24:%s\n", buf);
            if (len <= 0 || buf[0] == '#') { continue; }
            //printf("26:%s\n", buf);
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
        int32_t catalog_id = 0;
        int32_t parent_catalog_id = 0;
        if (4 == sscanf(buf, "%d\t%s\t%d\t%s", &catalog_id, catalog, &parent_catalog_id, parent_catalog)) {
            std::string catalog_str(catalog);
            std::string parent_catalog_str(parent_catalog);
            if (parent_catalog_id == 0) { //说明本身就是一级分类
                std::unordered_map<std::string, catalog_list_t>::const_iterator it = cata_map_.find(catalog_str);
                if (it == cata_map_.end()) {//不存在，加到cata_map_中
                    catalog_list_t clt;
                    cata_map_[catalog_str] = clt;
                }
            }
            else { //二级分类
                catalog_t ct = {catalog_id, catalog_str, parent_catalog_str};
                id_cata[catalog_id] = ct;
                std::unordered_map<std::string, catalog_list_t>::const_iterator it = cata_map_.find(parent_catalog_str);
                if (it == cata_map_.end()) {
                    catalog_list_t clt;
                    clt.catalog_list.push_back(ct);
                    cata_map_[parent_catalog_str] = clt;
                }
                else {
                    cata_map_[parent_catalog_str].catalog_list.push_back(ct);
                }
            }
            return 0;
        } else {
            return -1;
        }
    }

    /**根据传入的name1和name2，返回匹配的二级分类id.
     * 此处有badcase，如果name1和name2既是一级分类的名字，又是二级分类的名字，暂时不考虑这么蛋疼的事情
     */
    int CatalogDataReader::get_id(const std::string& name1,
            const std::string& name2, int32_t* id) {
        std::unordered_map<std::string, catalog_list_t>::const_iterator it = cata_map_.find(name1);
        if (it != cata_map_.end()) {//name1在一级分类中，去对应的二级分类下找name2
            std::vector<catalog_t> catalog_list = (it->second).catalog_list;
            int size = catalog_list.size();
            for (int i = 0; i < size; i++) {
                if (name2 == catalog_list[i].name) {//找到了,获取二级分类对应的id
                    *id = catalog_list[i].id;
                    return 0;
                }

            }

        }

        it = cata_map_.find(name2);
        if (it != cata_map_.end()) {//name2在一级分类中，去对应的二级分类下找name1
            std::vector<catalog_t> catalog_list = (it->second).catalog_list;
            int size = catalog_list.size();
            for (int i = 0; i < size; i++) {
                if (name1 == catalog_list[i].name) {
                    *id = catalog_list[i].id;
                    return 0;
                }
            }
        }

        return -1;
    }

    int CatalogDataReader::get_catalog_name(int32_t id, std::string& name) {
        std::unordered_map<int32_t, catalog_t>::const_iterator it = id_cata.find(id);
        if (it == id_cata.end()) {
            LOG(INFO) << "can not find name, id[" << id <<"]";
            return 1;
        }
        name = (it->second).name;

        return 0;

    }

    int CatalogDataReader::get_parent_catalog_name(int32_t id, std::string& name) {
        std::unordered_map<int32_t, catalog_t>::const_iterator it = id_cata.find(id);
        if (it == id_cata.end()) {
            LOG(INFO) << "can not find parent name, id[" << id << "]";
            return 1;
        }

        name = (it->second).parent_name;

        return 0;
    }
}
