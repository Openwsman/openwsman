/*
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <u/memory.h>

/**
 *  \defgroup alloc Memory
 *  \{
 */

/** \brief Wrapper for malloc(3) */
void *u_malloc (size_t sz)
{
    return malloc(sz);
}

/** \brief Wrapper for calloc(3) */
void *u_calloc (size_t cnt, size_t sz)
{
    return calloc(cnt, sz);
}

/** \brief Alloc a contiguous region of \p sz bytes and zero-fill it */
void *u_zalloc (size_t sz)
{
    return calloc(1, sz);
}

/** \brief Wrapper for realloc(3) */
void *u_realloc (void *ptr, size_t sz)
{
    return realloc(ptr, sz);
}

/** \brief Wrapper for free(3), sanity checks the supplied pointer */
void u_free (void *ptr)
{
    if (ptr)
        free(ptr);
}

typedef void* (*memset_t)(void*, int, size_t);

/* To make compiler always execute it */
static volatile memset_t memset_func = memset;

/** \brief Wrapper for free(3), sanity checks the supplied pointer and cleans memory*/
void u_cleanfree(char *ptr)
{
   if (!ptr)
      return;
   memset_func(ptr, 0, strlen(ptr) * sizeof(char));
   free(ptr);
}

/** \brief Wrapper for free(3), sanity checks the supplied pointer and cleans memory*/
void u_cleanfreew(wchar_t *ptr)
{
   if (!ptr)
      return;
   memset_func(ptr, 0, wcslen(ptr) * sizeof(wchar_t));
   free(ptr);
}

/**
 *      \}
 */
