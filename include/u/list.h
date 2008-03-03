/*
 * List Abstract Data Type
 * Copyright (C) 1997 Kaz Kylheku <kaz@ashi.footprints.net>
 *
 * Free Software License:
 *
 * All rights are reserved by the author, with the following exceptions:
 * Permission is granted to freely reproduce and distribute this software,
 * possibly in exchange for a fee, provided that this copyright notice appears
 * intact. Permission is also granted to adapt this software to produce
 * derivative works, as long as the modified versions carry this copyright
 * notice and additional notices stating that the work has been modified.
 * This source code may be translated into executable form and incorporated
 * into proprietary software; there is no requirement for such software to
 * contain a copyright notice related to this source.
 *
 * $Id: list.h,v 1.19 1999/11/14 20:46:19 kaz Exp $
 * $Name: kazlib_1_20 $
 */

#ifndef LIST_H
#define LIST_H

#include <limits.h>

#ifdef KAZLIB_SIDEEFFECT_DEBUG
#include "sfx.h"
#define LIST_SFX_CHECK(E) SFX_CHECK(E)
#else
#define LIST_SFX_CHECK(E) (E)
#endif

/*
 * Blurb for inclusion into C++ translation units
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long listcount_t;
#define LISTCOUNT_T_MAX ULONG_MAX

typedef struct lnode_t {
    #if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
    struct lnode_t *list_next;
    struct lnode_t *list_prev;
    void *list_data;
    #else
    int list_dummy;
    #endif
} lnode_t;

typedef struct lnodepool_t {
    #if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
    struct lnode_t *list_pool;
    struct lnode_t *list_free;
    listcount_t list_size;
    #else
    int list_dummy;
    #endif
} lnodepool_t;

typedef struct list_t {
    #if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
    lnode_t list_nilnode;
    listcount_t list_nodecount;
    listcount_t list_maxcount;
    #else
    int list_dummy;
    #endif
} list_t;

lnode_t *ow_lnode_create(void *);
lnode_t *ow_lnode_init(lnode_t *, void *);
void ow_lnode_destroy(lnode_t *);
void ow_lnode_put(lnode_t *, void *);
void *ow_lnode_get(lnode_t *);
int ow_lnode_is_in_a_list(lnode_t *);

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define lnode_put(N, D)		((N)->list_data = (D))
#define lnode_get(N)		((N)->list_data)
#else
#define lnode_put ow_lnode_put
#define lnode_get ow_lnode_get
#endif

lnodepool_t *ow_lnode_pool_init(lnodepool_t *, lnode_t *, listcount_t);
lnodepool_t *ow_lnode_pool_create(listcount_t);
void ow_lnode_pool_destroy(lnodepool_t *);
lnode_t *ow_lnode_borrow(lnodepool_t *, void *);
void ow_lnode_return(lnodepool_t *, lnode_t *);
int ow_lnode_pool_isempty(lnodepool_t *);
int ow_lnode_pool_isfrom(lnodepool_t *, lnode_t *);


list_t *ow_list_create(listcount_t);
/* list_t *ow_list_init(list_t *list, listcount_t maxcount); */
void ow_list_destroy(list_t *);
void ow_list_destroy_nodes(list_t *);
void ow_list_return_nodes(list_t *, lnodepool_t *);

listcount_t ow_list_count(list_t *);
int ow_list_isempty(list_t *);
int ow_list_isfull(list_t *);
int ow_list_contains(list_t *, lnode_t *);

void ow_list_append(list_t *, lnode_t *);
void ow_list_prepend(list_t *, lnode_t *);
void ow_list_ins_before(list_t *, lnode_t *, lnode_t *);
void ow_list_ins_after(list_t *, lnode_t *, lnode_t *);

lnode_t *ow_list_first(list_t *);
lnode_t *ow_list_last(list_t *);
lnode_t *ow_list_next(list_t *, lnode_t *);
lnode_t *ow_list_prev(list_t *, lnode_t *);

lnode_t *ow_list_del_first(list_t *);
lnode_t *ow_list_del_last(list_t *);
lnode_t *ow_list_delete(list_t *, lnode_t *);
lnode_t *ow_list_delete2(list_t *, lnode_t *);

void ow_list_process(list_t *, void *, void (*)(list_t *, lnode_t *, void *));

int ow_list_verify(list_t *);

#define lnode_create ow_lnode_create
#define lnode_init ow_lnode_init
#define lnode_destroy ow_lnode_destroy

#define lnode_is_in_a_list ow_lnode_is_in_a_list
#define lnode_pool_init ow_lnode_pool_init
#define lnode_pool_create ow_lnode_pool_create
#define lnode_pool_destroy ow_lnode_pool_destroy
#define lnode_borrow ow_lnode_borrow
#define lnode_return ow_lnode_return

#define lnode_pool_isfrom ow_lnode_pool_isfrom

#define list_create ow_list_create
#define list_init ow_list_init
#define list_destroy ow_list_destroy
#define list_destroy_nodes ow_list_destroy_nodes
#define list_return_nodes ow_list_return_nodes


#define list_contains ow_list_contains


#define list_ins_before ow_list_ins_before
#define list_ins_after ow_list_ins_after


#define list_delete ow_list_delete
#define list_delete2 ow_list_delete2

#define list_process ow_list_process
#define list_verify ow_list_verify


#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define lnode_pool_isempty(P)	((P)->list_free == 0)
#define list_count(L)		((L)->list_nodecount)
#define list_isempty(L)		((L)->list_nodecount == 0)
#define list_isfull(L)		(LIST_SFX_CHECK(L)->list_nodecount == (L)->list_maxcount)
#define list_next(L, N)		(LIST_SFX_CHECK(N)->list_next == &(L)->list_nilnode ? NULL : (N)->list_next)
#define list_prev(L, N)		(LIST_SFX_CHECK(N)->list_prev == &(L)->list_nilnode ? NULL : (N)->list_prev)
#define list_first(L)		list_next(LIST_SFX_CHECK(L), &(L)->list_nilnode)
#define list_last(L)		list_prev(LIST_SFX_CHECK(L), &(L)->list_nilnode)
#else
#define lnode_pool_isempty ow_lnode_pool_isempty
#define list_count ow_list_count
#define list_isempty ow_list_isempty
#define list_isfull ow_list_isfull
#define list_first ow_list_first
#define list_last ow_list_last
#define list_next ow_list_next
#define list_prev ow_list_prev
#endif

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define list_append(L, N)	list_ins_before(LIST_SFX_CHECK(L), N, &(L)->list_nilnode)
#define list_prepend(L, N)	list_ins_after(LIST_SFX_CHECK(L), N, &(L)->list_nilnode)
#define list_del_first(L)	list_delete(LIST_SFX_CHECK(L), list_first(L))
#define list_del_last(L)	list_delete(LIST_SFX_CHECK(L), list_last(L))
#else
#define list_append ow_list_append
#define list_prepend ow_list_prepend
#define list_del_first ow_list_del_first
#define list_del_last ow_list_del_last
#endif

/* destination list on the left, source on the right */

void ow_list_extract(list_t *, list_t *, lnode_t *, lnode_t *);
void ow_list_transfer(list_t *, list_t *, lnode_t *first);
void ow_list_merge(list_t *, list_t *, int (const void *, const void *));
void ow_list_sort(list_t *, int (const void *, const void *));
lnode_t *ow_list_find(list_t *, const void *, int (const void *, const void *));
int ow_list_is_sorted(list_t *, int (const void *, const void *));

#define list_extract ow_list_extract
#define list_transfer ow_list_transfer
#define list_merge ow_list_merge
#define list_sort ow_list_sort
#define list_find ow_list_find
#define list_is_sorted ow_list_is_sorted

#ifdef __cplusplus
}
#endif

#endif
