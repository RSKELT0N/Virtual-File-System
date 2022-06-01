#ifndef _SIZE_H_
#define _SIZE_H_

#define B(__size__)               (__size__)
#define KB(__size__)              ((unsigned long)B(__size__)  * (unsigned long)1024)
#define MB(__size__)              ((unsigned long)KB(__size__) * (unsigned long)1024)
#define GB(__size__)              ((unsigned long)MB(__size__) * (unsigned long)1024)

#endif // _SIZE_H_
