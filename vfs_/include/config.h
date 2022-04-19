#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Log.h"
#include "size.h"
#include "Buffer.h"

// ~~~~~~~~~~~ Config ~~~~~~~~~~

/* TODO (for future builds):
    - Clean code for better efficiency, making sure no redundant code exists in src.
    - Add functionality to prompt user with their own system, rather a segment of a singleton when using rfs. (Will allow user to only see their own disks created under their own account).
    - This will avoid data racing as a whole, allow disks to be shared with more than one user. Each remotely logged in user, will see their own version of the vfs.
    - Add a better protocol for edge case requests through server and client. Such as exporting file. (Makes messy code).
    - Add better atmoic locking to ensure that no data racing exists.
    - Segment fread into buffer to reduce ram usage. (As well as other parts that store large amount of bytes into local/remote memory).
    - Ensure project compiles on windows with new features added.
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
