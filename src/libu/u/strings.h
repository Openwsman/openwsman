

#ifndef  STRINGS_H
#define STRINGS_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { BLOCK_SIZE = 64 };

struct u_string_s;
typedef struct u_string_s u_string_t;

#define STRING_NULL { NULL, 0, 0, 0 };

int u_string_create(const char *buf, size_t len, u_string_t **ps);
int u_string_append(u_string_t *s, const char *buf, size_t len);
int u_string_set(u_string_t *s, const char *buf, size_t len);
int u_string_clear(u_string_t *s);
int u_string_free(u_string_t *s);
const char *u_string_c(u_string_t *s);
size_t u_string_len(u_string_t *s);
int u_string_copy(u_string_t *dst, u_string_t *src);
int u_string_set_length(u_string_t *s, size_t len); 
int u_string_trim(u_string_t *s);

int u_string_url_encode(u_string_t *s);
int u_string_url_decode(u_string_t *s);

int u_string_html_encode(u_string_t *s);
int u_string_html_decode(u_string_t *s);

int u_string_sql_encode(u_string_t *s);
int u_string_sql_decode(u_string_t *s);



char* u_str_clone(char* str);
void* u_clone(void* buf, size_t size);
char *u_strdup_printf(const char *format, ...);
char *u_strdup_vprintf(const char* format, va_list ap);
void u_strfreev (char **str_array);






char * u_strlwc(char * s);
char * u_strupc(char * s);
char * u_strskp(char * s);
char * u_strcrop(char * s);
char * u_strstrip(char * s) ;




#ifdef __cplusplus
}
#endif

#endif


