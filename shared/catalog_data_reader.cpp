#include <stdio.h>
#include <algorithm>
#include "glog/logging.h"
#include "catalog_data_reader.h"

using std::endl;
namespace tis {
    static bool catalog_cmp(const catalog_t *a, const catalog_t *b) {
        if (a->parent_id != b->parent_id) {
            return (a->parent_id - b->parent_id > 0);
        }
        else {
            return (a->name).compare(b->name) <= 0;
        }
    }

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

        //对catalog_list排序, 计算父节点对应哪些子节点
        if (!catalog_list.empty()) {
            std::sort(catalog_list.begin(), catalog_list.end(), catalog_cmp);
            int size = catalog_list.size();
            catalog_t* last_parent = NULL;

            for (int i = 0; i < size; i++) {
                catalog_t* now = catalog_list[i];
                if (now->parent == NULL)
                    continue;

                if (now->parent == last_parent) {
                    now->parent->children.offset += 1;
                }
                else {
                    now->parent->children.start = i;
                    now->parent->children.offset = 1;
                }

                last_parent = now->parent;

            }

        }


        return 0;
    }

    int CatalogDataReader::parse_line(const char* buf, int len) {
        //char parent_catalog[MAX_CATALOG_NAME_LEN];
        char catalog[MAX_CATALOG_NAME_LEN];
        int32_t catalog_id = 0;
        int32_t parent_catalog_id = 0;
        if (3 == sscanf(buf, "%d\t%s\t%d", &catalog_id, catalog, &parent_catalog_id)) {
            std::string catalog_str(catalog);
            if (parent_catalog_id == 0) { //最上层分类
                catalog_t *ct = new catalog_t;
                ct->id = catalog_id;
                ct->name = catalog_str;
                ct->parent_id = parent_catalog_id;
                ct->children = {-1, -1};
                ct->parent = NULL;
                auto it = level_one_map.find(catalog_str);
                if (it == level_one_map.end()) {//不存在，加到level_one_map中
                    level_one_map[catalog_str] = ct;
                }

                auto id_it = id_cata.find(catalog_id);
                if (id_it == id_cata.end()) {//id map不存在，直接更新
                    id_cata[catalog_id] = ct;
                }
                else {//id map已经存在，更新相应的域，不重新new 对象
                    catalog_t* ct_p = id_it->second;
                    ct_p -> id = catalog_id;
                    ct_p -> name = catalog;
                    ct_p -> parent = NULL;
                    ct_p -> children = {0, 0};
                }
                catalog_list.push_back(ct);//加到分类list中
            }
            else { //非最上层分类
                catalog_t *ct = new catalog_t;
                ct->id = catalog_id;
                ct->name = catalog;
                ct->parent_id = parent_catalog_id;
                auto id_it = id_cata.find(parent_catalog_id);
                if (id_it == id_cata.end()) {//没找到父节点
                    catalog_t *parent_ct = new catalog_t;//新建父节点 
                    parent_ct->id = parent_catalog_id;
                    id_cata[parent_catalog_id] = parent_ct;
                    ct->parent = parent_ct;
                }
                else {//父节点存在id_cata中
                    ct->parent = id_it->second;
                }

                id_it = id_cata.find(catalog_id);
                if (id_it == id_cata.end()) {//如果id不在id_cata中，加进去,理论上catalog_id不能有重复
                    id_cata[catalog_id] = ct;
                }

                catalog_list.push_back(ct);//加到分类list中
            }
            return 0;
        } else {
            return -1;
        }
    }

    int CatalogDataReader::binary_search(const meta& children, const std::string& name) {
        int left = children.start - 1;
        int right = children.start + children.offset;
        int middle;

        while (left + 1 != right) {
            middle = left + (right - left)/2;

            if (catalog_list[middle]->name.compare(name) < 0) {
                left = middle;
            }
            else {
                right = middle;
            }
        }

        if (right >= children.start + children.offset || catalog_list[right]->name.compare(name) != 0)
            return -1;
        return right;
    }

    int CatalogDataReader::get_id_in_level(const meta& children, std::vector<std::string>& left, int32_t* id, int level) {
        bool find = false;
        int size = left.size();
        meta next_children;
        for (int i = 0; i < size; i++) {
            int ret = binary_search(children, left[i]);
            if (ret != -1) {//找到了
                *id = catalog_list[ret]->id;
                find = true;
                next_children = catalog_list[ret]->children;

                std::string temp = left[i];
                left[i] = left[size-1];
                left[size-1] = temp;
                left.pop_back();
                break;
            }
        }

        if (!find)
            return -1;
        if (level == 0)
            return 0;

        return get_id_in_level(next_children, left, id, level-1);
    }

    /**传入切词list，先查找一级分类是否匹配，匹配成功后，剩下的词去匹配下级分类
     * level=0是一级分类,依次类推
     */
    int CatalogDataReader::get_id(const std::vector<std::string>& seg_list, int32_t* id, int level) {

        //1.在最上层分类进行匹配
        bool find_in_level1 = false;
        meta children = {-1, -1};
        std::vector<std::string> left = seg_list;
        int size = seg_list.size();
        for (int i = 0; i < size; i++) {
            auto it = level_one_map.find(seg_list[i]);
            if (it != level_one_map.end()) {//找到了
                find_in_level1 = true;
                children = (it->second)->children;
                *id = (it->second)->id;

                //在left中去掉匹配到的词
                std::string temp = left[i];
                left[i] = left[size-1];
                left[size-1] = temp;
                left.pop_back();
                break;
            }
        }

        if (!find_in_level1)
            return -1;//没找到
        if (level == 0)
            return 0;


        return get_id_in_level(children, left, id, level-1);
    }

    int CatalogDataReader::get_catalog_name(int32_t id, std::string& name) {
        auto it = id_cata.find(id);
        if (it == id_cata.end()) {
            LOG(INFO) << "can not find name, id[" << id <<"]";
            return 1;
        }
        name = it->second->name;

        return 0;

    }

    int CatalogDataReader::get_parent_catalog_name(int32_t id, std::string& name) {
        auto it = id_cata.find(id);
        if (it == id_cata.end()) {
            LOG(WARNING) << "can not find parent name, id[" << id << "]";
            return 1;
        }

        if (it->second->parent == NULL) {
            LOG(WARNING) << "can not find parent, id[" << id << "]";
            return 2;
        }

        name = it->second->parent->name;

        return 0;
    }
}
