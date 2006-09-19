

#ifndef  STRINGS_H
#define STRINGS_H
char* u_str_clone(char* str);
void* u_clone(void* buf, int size);
char *u_strdup_printf(const char *format, ...);
char *u_strdup_vprintf(const char* format, va_list ap);
void u_strfreev (char **str_array);
#endif


