#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Log.h"
#include "size.h"
#include "Buffer.h"

// ~~~~~~~~~~~ Config ~~~~~~~~~~

/* TODO:
    - add mutex for functions that can be accessed by multiple users.
    - Validate that no data racing can take place and multiple users can access a mounted disk at once with mutex locks
     in place and correct buffering within the correct area.
    - Stop simple errors such as no clusters left, cant find external file. Copying large file.
    - Fix RFS commands for removing, needs to say '!= 4' for checking validation. Also need to check for hybrid commands so it shows invalid command over wrong context.
*/

#define BUFFER     (*Buffer::get_buffer())
#define BUFFER_MAX MB(10)

#define _DEBUG_ 0

// IFS
#define CFG_USER_SPACE_SIZE    MB(200)
#define CFG_CLUSTER_SIZE       KB(2)

// RFS
#define CFG_SOCK_OPEN         (uint8_t)1
#define CFG_SOCK_CLOSE        (uint8_t)0
#define CFG_DEFAULT_PORT      (uint32_t)60000
#define CFG_SOCK_LISTEN_AMT   (uint32_t)5

#define CFG_PACKET_SIZE       (size_t)1024
#define CFG_FLAGS_BUFFER_SIZE (size_t)50
#define CFG_PAYLOAD_SIZE      (size_t)250

// ~~~~~~~~~~~ end ~~~~~~~~~~~~

#endif // _CONFIG_H
