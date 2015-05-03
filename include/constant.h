#ifndef __CONSTANT_H_
#define __CONSTANT_H_

#include <pthread.h>
#include "../base/server.h"

namespace tis {

typedef struct global_data_t {
    Server g_server;
}global_data;

extern global_data_t g_data;

}

#endif
