namespace cpp tis
namespace php se
namespace py se

struct TagAttr {
    1: required string name,
    2: required string value,
}

struct NumericAttr {
    1: required string name,
    2: required i16 value_type,//取值类型,1:int,2:double
    3: required string value_low,
    4: required string value_high,
}

struct SeRequest {
    1: required string query,
    2: required i32 pn,
    3: required i32 rn,
    4: optional list<TagAttr> tags,
    5: optional list<NumericAttr> numbers,
}

struct Item {
    1: required i32 id,
    2: optional i32 img_num,
    3: optional i32 zan_num,
    4: optional i32 uid,
    5: optional string show_img,
    6: optional list<i32> tag_id,//标签id
}

struct SeResponse {
    1: required i32 erro,
    2: optional i32 type,//结果类型,1:素材;2:广场
    3: optional list<Item> content,
}

struct EchoRequest {
    1: optional string req,
}

struct EchoResponse {
    1: optional string res,
}

service SeServer {
    SeResponse search(1:SeRequest request),
    EchoResponse heartbeat(1:EchoRequest request),
}

