#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "size.h"

// ~~~~~~~~~~~ Config ~~~~~~~~~~

/* TODO:
    - Create protocol around transfer with client -> server.
    - return data as strings to send, towards clients.
    - run server::run() on thread to accept client.
    - define client socket for server connection, define.
    - reformat cli structure for rfs and ifs within vfs.
*/

#ifndef _WIN32
#define LINUX__
#endif

#define _DEBUG_ 0

// IFS
#define CFG_USER_SPACE_SIZE  KB(16)
#define CFG_CLUSTER_SIZE     KB(2)

// RFS
#define CFG_SOCK_OPEN        (uint8_t)1
#define CFG_SOCK_CLOSE       (uint8_t)0
#define CFG_DEFAULT_PORT     (uint32_t)60000
#define CFG_SOCK_LISTEN_AMT  (uint32_t)1

// ~~~~~~~~~~~ end ~~~~~~~~~~~~

#endif // _CONFIG_H
