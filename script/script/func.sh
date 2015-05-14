
function send_warning_message() {
    local tele_list_file=$1
    local msg=$2
    cat $tele_list_file | xargs -i gsmsend -s emp02.baidu.com:15003 {}@"$msg"
} 
