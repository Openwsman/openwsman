

#ifndef  STRINGS_H
#define STRINGS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


char* u_str_clone(char* str);
void* u_clone(void* buf, size_t size);
char *u_strdup_printf(const char *format, ...);
char *u_strdup_vprintf(const char* format, va_list ap);


#ifdef __cplusplus
}
#endif

#endif


