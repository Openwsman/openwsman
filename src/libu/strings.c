

#include <u/libu.h>

char* u_str_clone(char* str)
{
    char* buf = NULL;
    if ( str ) {
        int size = strlen(str) + 1;
        buf = u_clone(str, size);
    }
    return buf;
}

void* u_clone(void* buf, int size)
{
    void* clone = NULL;
    if ( buf ) {
        if ( (clone = u_malloc(size)) != NULL )
            memcpy(clone, buf, size);
    }
    return clone;
}


char *
u_strdup_vprintf(const char* format, va_list ap)
{
    va_list ap2;
    int size;
    char* buffer;
    va_copy(ap2, ap);
    size = vsnprintf(NULL, 0, format, ap2)+1;
    va_end(ap2);
    buffer = malloc(size+1);
    if ( !buffer )
        return NULL;
    vsnprintf(buffer, size, format, ap);
    return buffer;
}

char *
u_strdup_printf(const char* format, ...)
{
    char* buffer;
    va_list ap;
    va_start(ap, format);
    buffer = u_strdup_vprintf(format, ap);
    va_end(ap);
    return buffer;
}

void
u_strfreev (char **str_array)
{
    if (str_array)
    {
        int i;
        for(i = 0; str_array[i] != NULL; i++)
            u_free(str_array[i]);
        u_free (str_array);
    }
}


