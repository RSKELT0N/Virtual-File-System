#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "size.h"

// ~~~~~~~~~~~ Config ~~~~~~~~~~

/* TODO:
    - Create protocol around transfer with client -> server.
    - add mutex for functions that can be accessed by multiple users.
    - Ensure no buffer is overflowed, data send through socket to client/server is fragmented and attached to user buffer.
    - Validate protocol works with small and large files, ensure that files can be sent over network, to append to file.
    - Validate that no data racing can take place and multiple users can access a mounted disk at once with mutex locks
     in place and correct buffering within the correct area.
    - Create strcuture around data to be sent within a packet through the network for fragmentation within the server -> client. Vice versa.

*/

#define _DEBUG_ 0

// IFS
#define CFG_USER_SPACE_SIZE  KB(16)
#define CFG_CLUSTER_SIZE     KB(2)

// RFS
#define CFG_SOCK_OPEN        (uint8_t)1
#define CFG_SOCK_CLOSE       (uint8_t)0
#define CFG_DEFAULT_PORT     (uint32_t)60000
#define CFG_SOCK_LISTEN_AMT  (uint32_t)1
#define BUFFER_SIZE          (size_t)1024

// ~~~~~~~~~~~ end ~~~~~~~~~~~~

#endif // _CONFIG_H
