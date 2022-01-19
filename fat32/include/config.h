#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Log.h"
#include "size.h"
#include "Buffer.h"

// ~~~~~~~~~~~ Config ~~~~~~~~~~

/* TODO:
    - add mutex for functions that can be accessed by multiple users.
    - Ensure no buffer is overflowed, data send through socket to client/server is fragmented and attached to user buffer.
    - Validate that no data racing can take place and multiple users can access a mounted disk at once with mutex locks
     in place and correct buffering within the correct area.
    - Stop circular include within server and client with VFS.
    - Stop simple errors such as no clusters left, cant find external file. Copying large file.
    - Everything that is printed will be moved to buffer and be used by: Client, Server, Terminal.
*/

#define BUFFER (*Buffer::get_buffer())

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
