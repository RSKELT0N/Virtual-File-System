#ifndef _SIZE_H_
#define _SIZE_H_

#define B(__size__)               (__size__)
#define KB(__size__)              ((unsigned long)B(__size__)  * (unsigned long)1024)
#define MB(__size__)              ((unsigned long)KB(__size__) * (unsigned long)1024)
#define GB(__size__)              ((unsigned long)MB(__size__) * (unsigned long)1024)
#define CLUSTER_ADDR(__CLUSTER__) (KB(2) * __CLUSTER__)

#endif // _SIZE_H_
