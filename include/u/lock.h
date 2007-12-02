
#ifndef LOCKING_H
#define LOCKING_H

#if defined (__FreeBSD__)  || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
/* Provide the Linux initializers for MacOS X */
#define PTHREAD_MUTEX_RECURSIVE_NP                                      PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP           { 0x4d555458, \
                                                           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                                                                                                                     0x20 } }
#endif


void u_unlock(void* data);
void u_destroy_lock(void* data);
void u_lock(void* data);
int u_try_lock(void* data);
void u_init_lock(void *data);


#endif
