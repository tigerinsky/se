################# basic conf ###########
MYSQL=mysql
PYTHON=python
#base_dir需要是绝对路径
BASE_DIR=""
SQL_DIR="${BASE_DIR}/sql/"
BIN_DIR="${BASE_DIR}/bin/"
SCRIPT_DIR="${BASE_DIR}/script/"
WORK_DIR="${BASE_DIR}/data/work/"
WORK_BAK_DIR="${BASE_DIR}/data/work.bak"
LOG_DIR="${BASE_DIR}/log/"
INDEX_DIR="${BASE_DIR}/index/"

################# DB conf ###########
DB_HOST=""
DB_PORT=""
DB_USER=""
DB_PWD=""
DB_ARGS=""


################# file ###########
RAW="${WORK_DIR}/raw"
TWEET="${WORK_DIR}/tweet"
ZAN="${WORK_DIR}/zan"
COMMENT="${WORK_DIR}/comment"
FLAG="${BASE_DIR}/makeindex_flag"

################# makeindex ###########
MAKEINDEX_DIR="./"
SE_INDEX_CONF=""
SE_VERSION_FILE=""
