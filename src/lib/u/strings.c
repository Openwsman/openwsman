#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <u/libu.h>


char *u_str_clone(char *str)
{
	char *buf = NULL;
	if (str) {
		size_t size = strlen(str) + 1;
		buf = u_clone(str, size);
	}
	return buf;
}

void *u_clone(void *buf, size_t size)
{
	void *clone = NULL;
	if (buf) {
		if ((clone = u_malloc(size)) != NULL)
			memcpy(clone, buf, size);
	}
	return clone;
}


char *u_strdup_vprintf(const char *format, va_list ap)
{
	va_list ap2;
	int size;
	char *buffer;
	VA_COPY(ap2, ap);
	size = vsnprintf(NULL, 0, format, ap2) + 1;
	va_end(ap2);
	buffer = malloc(size + 1);
	if (!buffer)
		return NULL;
	vsnprintf(buffer, size, format, ap);
	return buffer;
}

char *u_strdup_printf(const char *format, ...)
{
	char *buffer;
	va_list ap;
	va_start(ap, format);
	buffer = u_strdup_vprintf(format, ap);
	va_end(ap);
	return buffer;
}


