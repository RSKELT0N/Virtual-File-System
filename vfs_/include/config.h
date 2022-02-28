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
*/

#define BUFFER     (*Buffer::get_buffer())

#define _DEBUG_ 0

// IFS
#define CFG_USER_SPACE_SIZE       (GB(3))
#define CFG_CLUSTER_SIZE          (CFG_USER_SPACE_SIZE / (KB(2)))

// RFS
#define CFG_SOCK_OPEN             (uint8_t)1
#define CFG_SOCK_CLOSE            (uint8_t)0
#define CFG_DEFAULT_PORT          (uint32_t)52222
#define CFG_SOCK_LISTEN_AMT       (uint32_t)5

#define CFG_PACKET_SIGNATURE      (char*)"[_VFS_]\0"
#define CFG_PACKET_SIGNATURE_SIZE (strlen(CFG_PACKET_SIGNATURE))

#define CFG_FLAGS_BUFFER_SIZE     (size_t)100
#define CFG_PAYLOAD_SIZE          (KB(1))
#define CFG_PACKET_HASH_SIZE      (size_t)10
#define CFG_INVALID_PROTOCOL      LOG_str(Log::SERVER, "Invalid protocol, closing connection..")

// ~~~~~~~~~~~ end ~~~~~~~~~~~~

#endif // _CONFIG_H
