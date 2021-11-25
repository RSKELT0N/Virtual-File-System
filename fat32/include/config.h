#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "size.h"

// ~~~~~~~~~~~ Config ~~~~~~~~~~

/* TODO:
    - Add a FS header, with IFS and RFS as derived from FS.
    - Create server/client on RFS as derived classes.
    - Create protocol around transfer with client -> server.
    - return data as strings to send, towards clients.
    - issue when moving directory to another within root folder.
*/

#define _DEBUG_ 1

#define CFG_USER_SPACE_SIZE  KB(20)
#define CFG_CLUSTER_SIZE     KB(1)

// ~~~~~~~~~~~ end ~~~~~~~~~~~~

#endif // _CONFIG_H
