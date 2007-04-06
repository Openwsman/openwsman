#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <u/libu.h>



#define ASCIILINESZ	1024

/* null strings will be bound to the null char* */
static char null_cstr[1] = { 0 };
static char* null = null_cstr;


/* internal string struct */
struct u_string_s
{
    char *data;
    size_t data_sz, data_len, shift_cnt;
};

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

void u_strfreev(char **str_array)
{
	if (str_array) {
		int i;
		for (i = 0; str_array[i] != NULL; i++)
			u_free(str_array[i]);
		u_free(str_array);
	}
}





/**
 * \brief  Remove leading and trailing blanks
 *
 * Remove leading and trailing blanks from the given string
 *
 * \param s     string object
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_trim(u_string_t *s)
{
    if(s->data_len)
    {
        u_trim(s->data);

        s->data_len = strlen(s->data);
    }

    return 0;
}

/**
 * \brief  Set the length of a string (shortening it)
 * 
 *
 * \param s     string object
 * \param len   on success \a s will be \a len chars long
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_set_length(u_string_t *s, size_t len)
{
    dbg_err_if(len > s->data_len);

    if(len < s->data_len)
    {
        s->data_len = len;
        s->data[len] = 0;
    }

    return 0;
err:
    return ~0;
}

/**
 * \brief  Return the string length
 *
 * Return the length of the given string.
 *
 * \param s     string object
 *
 * \return the string length
 */

__INLINE__ size_t u_string_len(u_string_t *s)
{
    return s->data_len;
}

/**
 * \brief  Return the string value
 *
 * Return the const char* value of the given string object. Such const char*
 * value cannot be modified, realloc'd or free'd.
 *
 * \param s     string object
 *
 * \return the string value or NULL if the string is empty
 */
__INLINE__ const char *u_string_c(u_string_t *s)
{
    return s->data;
}

/**
 * \brief  Copy the value of a string to another
 *
 * Copy \a src string to \a dst string. 
 *
 * \param dst   destination string
 * \param src   source string
 *
 * \return \c 0 on success, not zero on failure
 */
__INLINE__ int u_string_copy(u_string_t *dst, u_string_t *src)
{
    u_string_clear(dst);
    return u_string_append(dst, src->data, src->data_len);
}

/**
 * \brief  Clear a string
 *
 * Totally erase the content of the given string.
 *
 * \param s     string object
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_clear(u_string_t *s)
{
    /* clear the string but not deallocate the buffer */
    if(s->data_sz)
    {
        s->data[0] = 0;
        s->data_len = 0;
    }

    return 0;
}

/**
 * \brief  Create a new string
 *
 * Create a new string object and save its pointer to \a *ps.
 *
 * If \a buf is not NULL (and \a len > 0) the string will be initialized with
 * the content of \a buf.
 *
 * \param buf   initial string value
 * \param len   length of \a buf
 * \param ps    on success will get the new string object
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_create(const char *buf, size_t len, u_string_t **ps)
{
    u_string_t *s = NULL;

    s = u_zalloc(sizeof(u_string_t));
    dbg_err_if(s == NULL);

    s->data = null;

    if(buf)
        dbg_err_if(u_string_append(s, buf, len));

    *ps = s;

    return 0;
err:
    dbg_strerror(errno);
    return ~0;
}


/**
 * \brief  Free a string
 *
 * Release all resources and free the given string object.
 *
 * \param s     string object
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_free(u_string_t *s)
{
    if(s)
    {
        if(s->data_sz)
            U_FREE(s->data);
        U_FREE(s);
    }
    return 0;
}


/**
 * \brief  Set the value of a string
 *
 * Set the value of \a s to \a buf.
 *
 * \param s     string object
 * \param buf   the value that will be copied to \a s
 * \param len   length of \a buf
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_set(u_string_t *s, const char *buf, size_t len)
{
    u_string_clear(s);
    return u_string_append(s, buf, len);
}

/**
 * \brief  Append a char* to a string
 *
 * Append a char* value to the given string. 
 *
 * \param s     string object
 * \param buf   the value that will be appended to \a s
 * \param len   length of \a buf
 *
 * \return \c 0 on success, not zero on failure
 */
int u_string_append(u_string_t *s, const char *buf, size_t len)
{
    char *ndata;
    size_t nsz, min;

    if(!len)
        return 0; /* nothing to do */

    /* if there's not enough space on pc->data alloc a bigger buffer */
    if(s->data_len + len + 1 > s->data_sz)
    {
        min = s->data_len + len + 1; /* min required buffer length */
        nsz = s->data_sz;
        while(nsz <= min)
            nsz += (BLOCK_SIZE << s->shift_cnt++);
        if(s->data == null)
            s->data = NULL;
        ndata = (char*) u_realloc(s->data, nsz);
        dbg_err_if(ndata == NULL);
        s->data = ndata;
        s->data_sz = nsz;
    }

    /* append this chunk to the data buffer */
    strncpy(s->data + s->data_len, buf, len);
    s->data_len += len;
    s->data[s->data_len] = 0;
    
    return 0;
err:
    return ~0;
}

/**
  @brief	Convert a string to lowercase.
  @param	s	String to convert.
  @return	ptr to statically allocated string.

  This function returns a pointer to a statically allocated string
  containing a lowercased version of the input string. Do not free
  or modify the returned string! Since the returned string is statically
  allocated, it will be modified at each function call (not re-entrant).
 */
char * u_strlwc(char * s)
{
    static char l[ASCIILINESZ+1];
    int i ;

    if (s==NULL) return NULL ;
    memset(l, 0, ASCIILINESZ+1);
    i=0 ;
    while (s[i] && i<ASCIILINESZ) {
        l[i] = (char)tolower((int)s[i]);
        i++ ;
    }
    l[ASCIILINESZ]=(char)0;
    return l ;
}


/**
  @brief	Convert a string to uppercase.
  @param	s	String to convert.
  @return	ptr to statically allocated string.

  This function returns a pointer to a statically allocated string
  containing an uppercased version of the input string. Do not free
  or modify the returned string! Since the returned string is statically
  allocated, it will be modified at each function call (not re-entrant).
 */

char * u_strupc(char * s)
{
    static char l[ASCIILINESZ+1];
    int i ;

    if (s==NULL) return NULL ;
    memset(l, 0, ASCIILINESZ+1);
    i=0 ;
    while (s[i] && i<ASCIILINESZ) {
        l[i] = (char)toupper((int)s[i]);
        i++ ;
    }
    l[ASCIILINESZ]=(char)0;
    return l ;
}



/**
  @brief	Skip blanks until the first non-blank character.
  @param	s	String to parse.
  @return	Pointer to char inside given string.

  This function returns a pointer to the first non-blank character in the
  given string.
 */

char * u_strskp(char * s)
{
    char * skip = s;
	if (s==NULL) return NULL ;
    while (isspace((int)*skip) && *skip) skip++;
    return skip ;
} 



/**
  @brief	Remove blanks at the end of a string.
  @param	s	String to parse.
  @return	ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end of the string have been removed.
  Do not free or modify the returned string! Since the returned string
  is statically allocated, it will be modified at each function call
  (not re-entrant).
 */

char * u_strcrop(char * s)
{
    static char l[ASCIILINESZ+1];
	char * last ;

    if (s==NULL) return NULL ;
    memset(l, 0, ASCIILINESZ+1);
	strcpy(l, s);
	last = l + strlen(l);
	while (last > l) {
		if (!isspace((int)*(last-1)))
			break ;
		last -- ;
	}
	*last = (char)0;
    return l ;
}



/**
  @brief	Remove blanks at the beginning and the end of a string.
  @param	s	String to parse.
  @return	ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end and the beg. of the string have been removed.
  Do not free or modify the returned string! Since the returned string
  is statically allocated, it will be modified at each function call
  (not re-entrant).
 */
char * u_strstrip(char * s)
{
    static char l[ASCIILINESZ+1];
	char * last ;
	
    if (s==NULL) return NULL ;
    
	while (isspace((int)*s) && *s) s++;
	
	memset(l, 0, ASCIILINESZ+1);
	strcpy(l, s);
	last = l + strlen(l);
	while (last > l) {
		if (!isspace((int)*(last-1)))
			break ;
		last -- ;
	}
	*last = (char)0;

	return (char*)l ;
}
