
#ifndef LOCKING_H
#define LOCKING_H
void u_unlock(void* data);
void u_destroy_lock(void* data);
void u_lock(void* data);
int u_try_lock(void* data);
void u_init_lock(void *data);


#endif
