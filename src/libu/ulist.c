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

u_list_t* u_list_append(u_list_t *list, void *data)
{
	u_list_t	*new,	*last;

	new = u_list_alloc();
	last = u_list_last(list);
	if (last) {
		last->next = new;
	}
	new->prev = last;
	new->data = data;

	return new;
}

u_list_t* u_list_prepend(u_list_t *list, void *data)
{
	u_list_t	*new,	*first;

	new = u_list_alloc();
	first = u_list_first(list);
	if (first) {
		first->prev = new;
	}
	new->next = first;
	new->data = data;

	return new;
}

u_list_t* u_list_remove(u_list_t *list, void *data)
{
	u_list_t	*tmp = NULL;

	list = u_list_first(list);
	while (list) {
		if (list->data == data) {
			if (list->next) {
				(list->next)->prev = list->prev;
				tmp = list->next;
			}
			if (list->prev) {
				(list->prev)->next = list->next;
				tmp = list->prev;
			}

			u_list_free(list);
			break;
		}
		list = list->next;
	}

	return u_list_first(tmp);
}	

void u_list_remove_all(u_list_t *list)
{
	u_list_t	*prev;

	list = u_list_first(list);
	while (list) {
		prev = list;
		list = list->next;
		u_free(prev);
	}
}
