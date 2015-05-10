namespace cpp tis
namespace php se
namespace py se

struct TagAttr {
    1: optional string name,
}

struct NumericAttr {
    1: optional string name,
    3: optional i32 low,
    4: optional i32 high,
}

struct SearchCondition {
    1: optional list<NumericAttr> num_filter,
    2: optional list<TagAttr> tag_filter,
}

struct TagGroup {
    1: optional string name,
    2: optional list<string> tag,
}

struct Catalog {
    1: optional i32 id,
    2: optional string name,
    3: optional list<TagGroup> tag_group
}

struct SeResponse {
    1: required i32 err_no,
    2: optional list<i32> id,
    3: optional SearchCondition search_condition,
    4: optional Catalog catalog,
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

