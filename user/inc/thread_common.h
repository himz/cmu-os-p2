#ifndef _THREAD_COMMON_H_
#define _THREAD_COMMON_H_

/*
 * Exposed THREAD APIs.
 */
inline void thr_set_main_stackH(char *input);
inline void thr_set_main_stackL(char *input);
inline void thr_set_resv_stackH(char *input);
inline void thr_set_resv_stackL(char *input);
inline char * thr_get_main_stackH();
inline char * thr_get_main_stackL();
inline char * thr_get_resv_stackH();
inline char * thr_get_resv_stackL();

#endif
