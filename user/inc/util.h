#ifndef _UTIL_H_
#define _UTIL_H_

#define SUCCESS 0
#define ERROR   -1

/*
 * Exposed utility APIs.
 */
char* util_get_esp();
char* util_get_ebp();
int   util_get_msb(void *num); /* num is opaque data */

#endif
