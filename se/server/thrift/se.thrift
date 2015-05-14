namespace cpp tis
namespace php se
namespace py se

struct TagAttr {
    1: required string name,
}

struct NumericAttr {
    1: required string name,
    3: required i32 low,
    4: required i32 high,
}

struct SearchCondition {
    1: required list<NumericAttr> num_filter,
    2: required list<TagAttr> tag_filter,
}

struct TagGroup {
    1: required string name,
    2: required list<string> tag,
}

struct Catalog {
    1: required i32 id,
    2: required string name,
    3: required list<TagGroup> tag_group
}

struct SeResponse {
    1: required i32 err_no,
    2: required i32 total_num,
    3: required list<i32> id,
    4: required SearchCondition search_condition,
    5: required Catalog catalog,
}

struct SeRequest {
    1: required string query,
    2: optional i32 pn,
    3: optional i32 rn,
    4: optional i32 type = 1,  // 1. 素材 2. 广场 
    5: optional i32 catalog = -1,
    6: optional list<string> tag,
}

service SeServer {
    SeResponse search(1:SeRequest request),
}

