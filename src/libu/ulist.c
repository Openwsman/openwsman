/*******************************************************************************
* Copyright (C) 2006 Intel Corp. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Intel Corp. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Denis Sadykov
 */

#include "u/libu.h"

u_list_t* u_list_alloc(void)
{
	return u_zalloc(sizeof(u_list_t));
}

void u_list_free(u_list_t *list)
{
	if (list)
		u_free(list);
}

u_list_t* u_list_first(u_list_t *list)
{
	while (list != NULL && list->prev != NULL) {
		list = list->prev;
	}
	return list;
}

u_list_t* u_list_last(u_list_t *list)
{
	while (list != NULL && list->next != NULL) {
		list = list->next;
	}
	return list;
}

u_list_t* u_list_next(u_list_t *list)
{
	if (list)
		list = list->next;

	return list;
}

u_list_t* u_list_previous(u_list_t *list)
{
	if (list)
		list = list->prev;

	return list;
}

u_list_t* u_list_append(u_list_t *list, void *data)
{
	u_list_t	*new;

	new = u_list_alloc();
	list = u_list_last(list);
	if (list) {
		list->next = new;
	}
	new->prev = list;
	new->data = data;

	return u_list_first(new);
}

u_list_t* u_list_prepend(u_list_t *list, void *data)
{
	u_list_t	*new;

	new = u_list_alloc();
	list = u_list_first(list);
	if (list) {
		list->prev = new;
	}
	new->next = list;
	new->data = data;

	return new;
}

u_list_t* u_list_find(u_list_t *list, void *data)
{
	list = u_list_first(list);
	while (list) {
		if (list->data == data)
			break;
		list = list->next;
	}

	return list;
}

unsigned int u_list_position(u_list_t *list, u_list_t *link)
{
	unsigned int	ind;

	list = u_list_first(list);
	for (ind = 0; list; ind++) {
		if (list == link)
			break;
		list = list->next;
	}

	return (list) ? ind : -1;
}

u_list_t* u_list_remove_link(u_list_t *list, u_list_t *link)
{
	if (u_list_position(list, link) == -1) {
		if (link)
			link->prev = link->next = NULL;
		return u_list_first(list);
	}

	list = NULL;
	if (link->prev) {
		(link->prev)->next = link->next;
		list = link->prev;
		link->prev = NULL;
	}
	if (link->next) {
		(link->next)->prev = link->prev;
		list = link->next;
		link->next = NULL;
	}

	return u_list_first(list);
}

u_list_t* u_list_delete_link(u_list_t *list, u_list_t *link)
{
	list = u_list_remove_link(list, link);

	if (link && link->data) {
		u_free(link->data);
		link->data = NULL;
	}

	return list;
}

unsigned int u_list_length(u_list_t *list)
{
	unsigned int	length = 0;

	list = u_list_first(list);
	while (list) {
		length++;
		list = u_list_next(list);
	}

	return length;
}

u_list_t* u_list_concat(u_list_t *list1, u_list_t *list2)
{
	if (!list1) {
		if (list2) {
			list2->next = list2->prev = NULL;
		}
		return u_list_first(list2);
	}
	if (!list2) {
		if (list1) {
			list1->next = list1->prev = NULL;
		}
		return u_list_first(list1);
	}

	list2 = u_list_first(list2);
	list1 = u_list_last(list1);

	list1->next = list2;
	list2->prev = list1;

	return u_list_first(list1);
}
