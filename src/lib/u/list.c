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
 * $Id: list.c,v 1.19.2.1 2000/04/17 01:07:21 kaz Exp $
 * $Name: kazlib_1_20 $
 */
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#define LIST_IMPLEMENTATION
#include "u/libu.h"

#define next list_next
#define prev list_prev
#define data list_data

#define fre list_free
#define size list_size

#define nilnode list_nilnode
#define nodecount list_nodecount
#define maxcount list_maxcount

#define list_nil(L)		(&(L)->nilnode)
#define list_first_priv(L)	((L)->nilnode.next)
#define list_last_priv(L)	((L)->nilnode.prev)
#define lnode_next(N)		((N)->next)
#define lnode_prev(N)		((N)->prev)


/*
 * Initialize a list object supplied by the client such that it becomes a valid
 * empty list. If the list is to be ``unbounded'', the maxcount should be
 * specified as LISTCOUNT_T_MAX, or, alternately, as -1. The value zero
 * is not permitted.
 */

static list_t *list_init(list_t *list, listcount_t maxcount)
{
    assert (maxcount != 0);
    list->nilnode.next = &list->nilnode;
    list->nilnode.prev = &list->nilnode;
    list->nodecount = 0;
    list->maxcount = maxcount;
    return list;
}

/*
 * Dynamically allocate a list object using malloc(), and initialize it so that
 * it is a valid empty list. If the list is to be ``unbounded'', the maxcount
 * should be specified as LISTCOUNT_T_MAX, or, alternately, as -1.
 */

list_t *list_create(listcount_t maxcount)
{
    list_t *new = malloc(sizeof *new);
    if (new) {
	assert (maxcount != 0);
	new->nilnode.next = &new->nilnode;
	new->nilnode.prev = &new->nilnode;
	new->nodecount = 0;
	new->maxcount = maxcount;
    }
    return new;
}

/*
 * Destroy a dynamically allocated list object.
 * The client must remove the nodes first.
 */

void list_destroy(list_t *list)
{
    assert (list_isempty(list));
    free(list);
}

/*
 * Free all of the nodes of a list. The list must contain only
 * dynamically allocated nodes. After this call, the list
 * is empty.
 */

void list_destroy_nodes(list_t *list)
{
    lnode_t *lnode = list_first_priv(list), *nil = list_nil(list), *tmp;

    while (lnode != nil) {
	tmp = lnode->next;
	lnode->next = NULL;
	lnode->prev = NULL;
        u_free(lnode->list_data);
	lnode_destroy(lnode);
	lnode = tmp;
    }

    list_init(list, list->maxcount);
}


/*
 * Insert the node ``new'' into the list immediately after ``this'' node.
 */

void list_ins_after(list_t *list, lnode_t *new, lnode_t *this)
{
    lnode_t *that = this->next;

    assert (new != NULL);
    assert (!list_contains(list, new));
    assert (!lnode_is_in_a_list(new));
    assert (this == list_nil(list) || list_contains(list, this));
    assert (list->nodecount + 1 > list->nodecount);

    new->prev = this;
    new->next = that;
    that->prev = new;
    this->next = new;
    list->nodecount++;

    assert (list->nodecount <= list->maxcount);
}

/*
 * Insert the node ``new'' into the list immediately before ``this'' node.
 */

void list_ins_before(list_t *list, lnode_t *new, lnode_t *this)
{
    lnode_t *that = this->prev;

    assert (new != NULL);
    assert (!list_contains(list, new));
    assert (!lnode_is_in_a_list(new));
    assert (this == list_nil(list) || list_contains(list, this));
    assert (list->nodecount + 1 > list->nodecount);

    new->next = this;
    new->prev = that;
    that->next = new;
    this->prev = new;
    list->nodecount++;

    assert (list->nodecount <= list->maxcount);
}

/*
 * Delete the given node from the list.
 */

lnode_t *list_delete(list_t *list, lnode_t *del)
{
    lnode_t *next = del->next;
    lnode_t *prev = del->prev;

    assert (list_contains(list, del));

    prev->next = next;
    next->prev = prev;
    list->nodecount--;

    del->next = del->prev = NULL;

    return del;
}

/*
 * Delete the given node from the list. Return next node
 */

lnode_t *list_delete2(list_t *list, lnode_t *del)
{
    lnode_t *next = del->next;
    lnode_t *prev = del->prev;

    assert (list_contains(list, del));

    prev->next = next;
    next->prev = prev;
    list->nodecount--;

    del->next = del->prev = NULL;
    if (next == &list->nilnode) {
	   return NULL;
    }
    return next;
}

/*
 * For each node in the list, execute the given function. The list,
 * current node and the given context pointer are passed on each
 * call to the function.
 */

void list_process(list_t *list, void *context,
	void (* function)(list_t *list, lnode_t *lnode, void *context))
{
    lnode_t *node = list_first_priv(list), *next, *nil = list_nil(list);

    while (node != nil) {
	/* check for callback function deleting	*/
	/* the next node from under us		*/
	assert (list_contains(list, node));
	next = node->next;
	function(list, node, context);
	node = next;
    }
}

/*
 * Dynamically allocate a list node and assign it the given piece of data.
 */

lnode_t *lnode_create(void *data)
{
    lnode_t *new = malloc(sizeof *new);
    if (new) {
	new->data = data;
	new->next = NULL;
	new->prev = NULL;
    }
    return new;
}

/*
 * Initialize a user-supplied lnode.
 */

lnode_t *lnode_init(lnode_t *lnode, void *data)
{
    lnode->data = data;
    lnode->next = NULL;
    lnode->prev = NULL;
    return lnode;
}

/*
 * Destroy a dynamically allocated node.
 */

void lnode_destroy(lnode_t *lnode)
{
    assert (!lnode_is_in_a_list(lnode));
    free(lnode);
}


/*
 * Determine whether the given list contains the given node.
 * According to this function, a list does not contain its nilnode.
 */

int list_contains(list_t *list, lnode_t *node)
{
    lnode_t *n, *nil = list_nil(list);

    for (n = list_first_priv(list); n != nil; n = lnode_next(n)) {
	if (node == n)
	    return 1;
    }

    return 0;
}


/*
 * Split off a trailing sequence of nodes from the source list and relocate
 * them to the tail of the destination list. The trailing sequence begins
 * with node ``first'' and terminates with the last node of the source
 * list. The nodes are added to the end of the new list in their original
 * order.
 */

void ow_list_transfer(list_t *dest, list_t *source, lnode_t *first)
{
    listcount_t moved = 1;
    lnode_t *last;

    if (first == NULL)
	return;

    assert (list_contains(source, first));

    last = source->nilnode.prev;
	
    source->nilnode.prev = first->prev;
    first->prev->next = list_nil(source);
	
    last->next = list_nil(dest);
    first->prev = dest->nilnode.prev;
    dest->nilnode.prev->next = first;
    dest->nilnode.prev = last;

    while (first != last) {
	first = first->next;
	moved++;
    }
    
    /* assert no weirdness */
    assert (moved <= source->nodecount);

    source->nodecount -= moved;
    dest->nodecount += moved;

    /* assert list sanity */
    assert (list_verify(source));
    assert (list_verify(dest));
}


/*
 * merge source to dest, sorted
 * 
 */

void ow_list_merge(list_t *dest, list_t *source,
	int compare (const void *, const void *))
{
    lnode_t *dn, *sn, *tn;
    lnode_t *d_nil = list_nil(dest), *s_nil = list_nil(source);

    /* Nothing to do if source and destination list are the same. */
    if (dest == source)
	return;

    /* lists must be sorted */
    assert (list_is_sorted(source, compare));
    assert (list_is_sorted(dest, compare));

    dn = list_first_priv(dest);
    sn = list_first_priv(source);

    while (dn != d_nil && sn != s_nil) {
	if (compare(lnode_get(dn), lnode_get(sn)) >= 0) {
		tn = lnode_next(sn);
		list_delete(source, sn);
		list_ins_before(dest, sn, dn);
		sn = tn;
	} else {
		dn = lnode_next(dn);
	}
    }

    if (dn != d_nil) /* sn == s_nil */
	return;

    if (sn != s_nil)
	list_transfer(dest, source, sn);
}


/*
 * sort list according to compare()
 * 
 * quicksort
 * 
 */

void ow_list_sort(list_t *list, int compare(const void *, const void *))
{
    list_t extra;
    listcount_t middle;
    lnode_t *node;

    if (list_count(list) > 1) {
	middle = list_count(list) / 2;
	node = list_first_priv(list);

	list_init(&extra, list_count(list) - middle);

	/* advance node to middle */
	while (middle--)
	    node = lnode_next(node);

	/* transfer 'right' half of list to extra  */
	ow_list_transfer(&extra, list, node);
	/* sort 'left' half */
	ow_list_sort(list, compare);
	/* sort 'right' half */
	ow_list_sort(&extra, compare);
	/* merge sorted halfs */
	ow_list_merge(list, &extra, compare);
    } 
    assert (list_is_sorted(list, compare));
}


lnode_t *list_find(list_t *list, const void *key, int compare(const void *, const void *))
{
    lnode_t *node;

    for (node = list_first_priv(list); node != list_nil(list); node = node->next) {
	if (compare(lnode_get(node), key) == 0)
	    return node;
    }

    return 0;
}


/*
 * Return 1 if the list is in sorted order, 0 otherwise
 */

int list_is_sorted(list_t *list, int compare(const void *, const void *))
{
    lnode_t *node, *next, *nil;

    next = nil = list_nil(list);
    node = list_first_priv(list);

    if (node != nil)
	next = lnode_next(node);

    for (; next != nil; node = next, next = lnode_next(next)) {
	if (compare(lnode_get(node), lnode_get(next)) > 0)
	    return 0;
    }

    return 1;
}

/*
 * Get rid of macro functions definitions so they don't interfere
 * with the actual definitions
 */

#undef list_isempty
#undef list_isfull
#undef list_append
#undef list_prepend
#undef list_first
#undef list_last
#undef list_next
#undef list_prev
#undef list_count
#undef list_del_first
#undef list_del_last
#undef lnode_put
#undef lnode_get

/*
 * Return 1 if the list is empty, 0 otherwise
 */

int ow_list_isempty(list_t *list)
{
    if (list == NULL) {
        return 1;
    }
    return list->nodecount == 0;
}

/*
 * Return 1 if the list is full, 0 otherwise
 * Permitted only on bounded lists.
 */

int ow_list_isfull(list_t *list)
{
    if (list == NULL) {
        return 0;
    }
    return list->nodecount == list->maxcount;
}


/*
 * Add the given node at the end of the list
 */

void ow_list_append(list_t *list, lnode_t *node)
{
    list_ins_before(list, node, &list->nilnode);
}

/*
 * Add the given node at the beginning of the list.
 */

void ow_list_prepend(list_t *list, lnode_t *node)
{
    list_ins_after(list, node, &list->nilnode);
}

/*
 * Retrieve the first node of the list
 */

lnode_t *ow_list_first(list_t *list)
{
    if (list == NULL) {
        return NULL;
    }
    if (list->nilnode.next == &list->nilnode) {
	   return NULL;
    }
    return list->nilnode.next;
}

/*
 * Retrieve the last node of the list
 */

lnode_t *ow_list_last(list_t *list)
{
    if (list->nilnode.prev == &list->nilnode)
	return NULL;
    return list->nilnode.prev;
}

/*
 * Retrieve the count of nodes in the list
 */

listcount_t ow_list_count(list_t *list)
{
    return list->nodecount;
}

/*
 * Remove the first node from the list and return it.
 */

lnode_t *ow_list_del_first(list_t *list)
{
    return list_delete(list, list->nilnode.next);
}

/*
 * Remove the last node from the list and return it.
 */

lnode_t *ow_list_del_last(list_t *list)
{
    return list_delete(list, list->nilnode.prev);
}


/*
 * Associate a data item with the given node.
 */

void ow_lnode_put(lnode_t *lnode, void *data)
{
    lnode->data = data;
}

/*
 * Retrieve the data item associated with the node.
 */

void *ow_lnode_get(lnode_t *lnode)
{
    return lnode->data;
}

/*
 * Retrieve the node's successor. If there is no successor,
 * NULL is returned.
 */

lnode_t *ow_list_next(list_t *list, lnode_t *lnode)
{
    assert (list_contains(list, lnode));

    if (lnode->next == list_nil(list))
	return NULL;
    return lnode->next;
}

/*
 * Retrieve the node's predecessor. See comment for lnode_next().
 */

lnode_t *ow_list_prev(list_t *list, lnode_t *lnode)
{
    assert (list_contains(list, lnode));

    if (lnode->prev == list_nil(list))
	return NULL;
    return lnode->prev;
}

/*
 * Return 1 if the lnode is in some list, otherwise return 0.
 */

int lnode_is_in_a_list(lnode_t *lnode)
{
    return (lnode->next != NULL || lnode->prev != NULL);
}


int list_verify(list_t *list)
{
    lnode_t *node = list_first_priv(list), *nil = list_nil(list);
    listcount_t count = ow_list_count(list);

    if (node->prev != nil)
	return 0;

    if (count > list->maxcount)
	return 0;

    while (node != nil && count--) {
	if (node->next->prev != node)
	    return 0;
	node = node->next;
    }

    if (count != 0 || node != nil)
	return 0;

    return 1;
}

#ifdef KAZLIB_TEST_MAIN

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

typedef char input_t[256];

static int tokenize(char *string, ...)
{
    char **tokptr;
    va_list arglist;
    int tokcount = 0;

    va_start(arglist, string);
    tokptr = va_arg(arglist, char **);
    while (tokptr) {
	while (*string && isspace((unsigned char) *string))
	    string++;
	if (!*string)
	    break;
	*tokptr = string;
	while (*string && !isspace((unsigned char) *string))
	    string++;
	tokptr = va_arg(arglist, char **);
	tokcount++;
	if (!*string)
	    break;
	*string++ = 0;
    }
    va_end(arglist);

    return tokcount;
}

static int comparef(const void *key1, const void *key2)
{
    return strcmp(key1, key2);
}

static char *dupstring(char *str)
{
    int sz = strlen(str) + 1;
    char *new = malloc(sz);
    if (new)
	memcpy(new, str, sz);
    return new;
}

int main(void)
{
    input_t in;
    list_t *l = list_create(LISTCOUNT_T_MAX);
    lnode_t *ln;
    char *tok1, *val;
    int prompt = 0;

    char *help =
	"a <val>                append value to list\n"
	"d <val>                delete value from list\n"
	"l <val>                lookup value in list\n"
	"s                      sort list\n"
	"c                      show number of entries\n"
	"t                      dump whole list\n"
	"p                      turn prompt on\n"
	"q                      quit";

    if (!l)
	puts("list_create failed");

    for (;;) {
	if (prompt)
	    putchar('>');
	fflush(stdout);

	if (!fgets(in, sizeof(input_t), stdin))
	    break;

	switch(in[0]) {
	    case '?':
		puts(help);
		break;
	    case 'a':
		if (tokenize(in+1, &tok1, (char **) 0) != 1) {
		    puts("what?");
		    break;
		}
		val = dupstring(tok1);
		ln = lnode_create(val);

		if (!val || !ln) {
		    puts("allocation failure");
		    if (ln)
			lnode_destroy(ln);
		    free(val);
		    break;
		}

		list_append(l, ln);
		break;
	    case 'd':
		if (tokenize(in+1, &tok1, (char **) 0) != 1) {
		    puts("what?");
		    break;
		}
		ln = list_find(l, tok1, comparef);
		if (!ln) {
		    puts("list_find failed");
		    break;
		}
		list_delete(l, ln);
		val = lnode_get(ln);
		lnode_destroy(ln);
		free(val);
		break;
	    case 'l':
		if (tokenize(in+1, &tok1, (char **) 0) != 1) {
		    puts("what?");
		    break;
		}
		ln = list_find(l, tok1, comparef);
		if (!ln)
		    puts("list_find failed");
		else
		    puts("found");
		break;
	    case 's':
		list_sort(l, comparef);
		break;
	    case 'c':
		printf("%lu\n", (unsigned long) list_count(l));
		break;
	    case 't':
		for (ln = list_first(l); ln != 0; ln = list_next(l, ln))
		    puts(lnode_get(ln));
		break;
	    case 'q':
		exit(0);
		break;
	    case '\0':
		break;
	    case 'p':
		prompt = 1;
		break;
	    default:
		putchar('?');
		putchar('\n');
		break;
	}
    }

    return 0;
}

#endif	/* defined TEST_MAIN */
