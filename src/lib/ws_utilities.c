/*******************************************************************************
* Copyright (C) 2004-2006 Intel Corp. All rights reserved.
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
 * @author Anas Nashif
 * @author Eugene Yarmosh
 */
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <uuid/uuid.h>

#include <assert.h>

#include "ws_utilities.h"

#ifndef __USE_UNIX98
extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind)
	     __THROW;
#endif

void soap_initialize_lock(void *data)
{
        pthread_mutexattr_t attrVal;
        // void* data = soap_alloc(sizeof(pthread_mutex_t), 0);
        pthread_mutexattr_init( &attrVal );
        pthread_mutexattr_settype( &attrVal, PTHREAD_MUTEX_RECURSIVE_NP );
        if ( data != NULL )
                pthread_mutex_init((pthread_mutex_t*)data, &attrVal);
        // return data;
}


void soap_get_uuid (char* buf, int size, int bNoPrefix) {
	uuid_t uuid;   
    char   str[37];
    
    char* ptr = buf;
    
	if ( !bNoPrefix )
	{
		sprintf( ptr, "uuid:" );
		ptr += 5;
	}
   	uuid_generate(uuid);
    uuid_unparse(uuid, str);
    sprintf( ptr, "%s", str);	
}


void soap_fw_lock(void* data)
{
	if ( data )
	{
		//printf("\nfwEntryCount = %i\n", fwEntryCount);
		pthread_mutex_lock( (pthread_mutex_t*)data );
		
	}
}



void soap_fw_unlock(void* data)
{
	if ( data )
	{	
		pthread_mutex_unlock((pthread_mutex_t*)data);
	}
}


// SoapCloneString
char* soap_clone_string(char* str)
{
	char* buf = NULL;
	
	if ( str )
	{
		int size = strlen(str) + 1;
		buf = soap_clone(str, size);
	}
	return buf;
}

// SoapClone
void* soap_clone(void* buf, int size)
{
	void* clone = NULL;
	
	if ( buf )
	{
		if ( (clone = soap_alloc(size, 0)) != NULL )
			memcpy(clone, buf, size);
	}
	return clone;
}


// soap_alloc
void* soap_alloc(int size, int zeroInit)
{
	void* ptr = malloc(size);
	if ( ptr && zeroInit )
		memset(ptr, 0, size);
	return ptr;
}

void soap_free(void* ptr)
{
	if ( ptr )
		free(ptr);
}


// DL List


DL_Node* DL_MakeNode(DL_List* list, void* dataBuf)
{
    DL_Node * node = soap_alloc(sizeof(DL_Node), 1);
    if ( node )
    {
        node->dataBuf = dataBuf;
        if ( list )
            DL_AddTail(list, node);
    }

    return node;
}

void DL_AddHead(DL_List* list, DL_Node* node)
{
    assert(!node->list);
    list->count++;
    if ( list->head != NULL )
        list->head->prev = node;

    node->prev = NULL;
    node->next = list->head;
    node->list = list;

    list->head = node;

    if ( list->tail == NULL )
        list->tail = node;
}


void DL_AddTail(DL_List* list, DL_Node* node)
{
    assert(!node->list);
    list->count++;

    if ( list->head == NULL )
        list->head = node;

    if ( list->tail != NULL )
        list->tail->next = node;

    node->prev = list->tail;
    node->next = NULL;
    node->list = list;

    list->tail = node;
}


void DL_AddAfter(DL_Node* node_to_add, DL_Node* add_after)
{
    assert(!node_to_add->list);
    assert(add_after->list);

    node_to_add->list = add_after->list;
    assert(node_to_add->list);

    node_to_add->list->count++;

    node_to_add->next = add_after->next;

    if ( node_to_add->next != NULL )
        node_to_add->next->prev = node_to_add;

    add_after->next = node_to_add;
    node_to_add->prev = add_after;

    if ( node_to_add->list->tail == add_after )
        node_to_add->list->tail = node_to_add;
}


void DL_AddBefore(DL_Node* node_to_add, DL_Node* add_before)
{
    assert(!node_to_add->list);
    assert(add_before->list);

    node_to_add->list = add_before->list;
    assert(node_to_add->list);

    node_to_add->list->count++;

    node_to_add->prev = add_before->prev;

    if ( node_to_add->prev != NULL )
        node_to_add->prev->next = node_to_add;

    add_before->prev = node_to_add;
    node_to_add->next = add_before;

    if ( node_to_add->list->head == add_before )
        node_to_add->list->head = node_to_add;
}

DL_Node* DL_RemoveHead(DL_List* list)
{
    DL_Node* node = NULL;
    if ( list && list->head )
    {
        node = list->head;
        node->list->count--;
        if ( (list->head = node->next) != NULL )
            list->head->prev = NULL;

        if ( list->tail == node )
            list->tail = NULL;

        node->list = NULL;
        node->next = node->prev = NULL;
    }
    return node;
}


DL_Node* DL_RemoveTail(DL_List* list)
{
    DL_Node* node = NULL;
    if ( list && list->tail )
    {
        node = list->tail;
        node->list->count--;

        if ( (list->tail = node->prev) != NULL )
            list->tail->next = NULL;

        if ( list->head == node )
            list->head = NULL;

        node->list = NULL;
        node->next = node->prev = NULL;
    }
    return node;
}

DL_List* DL_RemoveNode(DL_Node* node)
{
    DL_List* list = node->list;	

    if ( list )
    {
        list->count--;
        if ( list->head == node )
            list->head = node->next;
        if ( list->tail == node )
            list->tail = node->prev;

        if ( node->next )
            node->next->prev = node->prev;
        if ( node->prev )
            node->prev->next = node->next;
        node->list = NULL;
        node->next = node->prev = NULL;
    }

    return list;
}

DL_List* DL_FreeNode(DL_Node* node)
{
    DL_List* list = DL_RemoveNode(node);	
    soap_free(node);

    return list;
}

int DL_GetCount(DL_List* list)
{
    return list->count;
}

int DL_IsEmpty(DL_List* list)
{
    return (!list->count);
}


DL_Node* DL_GetHead(DL_List* list)
{
    return (!list) ? NULL : list->head;
}

DL_Node* DL_GetTail(DL_List* list)
{
    return (!list) ? NULL : list->tail;
}

DL_Node* DL_GetNext(DL_Node* node)
{
    return (!node) ? NULL : node->next;
}

DL_Node* DL_GetPrev(DL_Node* node)
{
    return (!node) ? NULL : node->prev;
}

void* DL_GetListOwner(DL_List* list)
{
    return (!list) ? NULL : list->listOwner;
}

void* DL_GetNodeData(DL_Node* node)
{
    return (!node) ? NULL : node->dataBuf;
}

DL_List* DL_GetList(DL_Node* node)
{
    return (!node) ? NULL : node->list;
}


void DL_RemoveAndDestroyAllNodes(DL_List* list, int freeDataBuf)
{
    if ( list )
    {
        DL_Node* node;
        while((node = DL_RemoveHead(list)) != NULL )
        {
            if ( freeDataBuf )
                soap_free(node->dataBuf);
            soap_free(node);
        }
    }
}

void DL_RemoveAndDestroyAllNodesCallback(DL_List* list, 
        void (*callback)(void*, DL_Node*),
        void* callbackData)
{
    if ( list )
    {
        DL_Node* node;
        while((node = DL_RemoveHead(list)) != NULL )
        {
            if ( callback != NULL )
                callback(callbackData, node);
            soap_free(node);
        }
    }
}

DL_Node* DL_FindNode(DL_List* list, void* data, int (*proc)(void*, DL_Node*))
{
    DL_Node* node = !list ? NULL : DL_GetHead(list);

    while( node != NULL )
    {
        if ( proc != NULL )
        {
            if ( proc(data, node) != 0 )
                break;
        }
        else
            if ( data == node->dataBuf )
                break;
        node = DL_GetNext(node);
    }

    return NULL;
}

unsigned long soap_get_ticks()
{
        // struct has seconds+us since Epoch in 1970
        // turn these into total ms
        struct timeval tv;
        gettimeofday( &tv, NULL );
        unsigned long tick = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        return tick;
}

int is_time_up(unsigned long lastTicks, unsigned long tm)
{
    if ( (soap_get_ticks() - lastTicks) >= tm )
        return 1;
    return 0;
}

void soap_sleep(unsigned long tm)
{
        int sec = tm / 1000;
        int msec = tm - sec * 1000;
        int nsec = msec * 1000000;

        struct timespec ts;

        ts.tv_sec = sec;
        ts.tv_nsec = nsec;
        nanosleep( &ts, NULL );
}


#define MAX_QUERY_KEYS 100



struct pair_t * parse_query(const char *string, int separator)
{
  int i;
  char *p;
  char *query_string;
  size_t query_len;
  struct pair_t *query;

  if(!string)
    return NULL;

  if((query = (struct pair_t *)malloc(sizeof(struct pair_t)*MAX_QUERY_KEYS))
     == NULL)
  {
    fprintf(stderr, "Could not malloc\n");
    exit (1);
  }

  query_len = strlen(string);
  if((query_string = malloc(query_len + 1)) == NULL)
  {
    fprintf(stderr, "Could not malloc\n");
    exit (1);
  }

  memcpy(query_string, string, query_len + 1);

  for(i = 0, p = query_string; i < MAX_QUERY_KEYS && *p; i++, p++) {
    while(*p == ' ') p++;

    query[i].name = p;
    while(*p != '=' && *p != separator && *p) p++;
    if(*p != '=') {
      *p = 0;
      query[i].value = NULL;
      continue;
    }
    *p++ = 0;
    
    query[i].value = p;
    while(*p != separator && *p) p++;
    *p = 0;
  }

  query[i].name = NULL;

  for(i = 0; query[i].name; i++) {
    uri_unescape(query[i].name);
    uri_unescape(query[i].value);
  }

  return query;
}

int xdigit_to_int(int c)
{
  c = tolower(c);

  if(isdigit(c))
    return c - '0';
  else if(isxdigit(c))
    return c - 'a';
  else
    return -1;
}


void uri_unescape(char *ptr)
{
  char *nptr = ptr;
  int c, d;

  while(*ptr) {
    if(*ptr == '%') {
      if((c = xdigit_to_int(*++ptr)) != -1 &&
     (d = xdigit_to_int(*++ptr)) != -1) {
    *nptr++ = (c << 4) | d;
    ptr++;
      }
    } else if(*ptr == '+') {
      *nptr++ = ' ';
      ptr++;
    } else {
      *nptr++ = *ptr++;
    }
  }
  *nptr = 0;
  return;
}


