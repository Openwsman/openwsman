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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>

#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/types.h>


#include <assert.h>
#include <sys/ioctl.h>
#include <netdb.h>
// #include <net/if_arp.h>
#include <sys/utsname.h>

#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include "ws_utilities.h"
#include "wsman-debug.h"

/*
#ifndef __USE_UNIX98
extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind)
__THROW;
#endif
*/

static long mac_addr_sys ( u_char *addr)
{
    struct ifreq ifr;
    struct ifreq *IFR;
    struct ifconf ifc;
    char buf[1024];
    int s, i;
    int ok = 0;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s==-1) {
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    ioctl(s, SIOCGIFCONF, &ifc);

    IFR = ifc.ifc_req;
    for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++) {

        strcpy(ifr.ifr_name, IFR->ifr_name);
        if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
                    ok = 1;
                    break;
                }
            }
        }
    }

    close(s);
    if (ok) {
        bcopy( ifr.ifr_hwaddr.sa_data, addr, 6);
    }
    else {
        return -1;
    }
    return 0;
}

int soap_get_uuid (char* buf, int size, int bNoPrefix) 
{

    static int clock_sequence = 1;
    u_int64_t longTimeVal;
    int timeLow = 0;
    int timeMid = 0;
    int timeHigh = 0;
    unsigned char mac[6];
    unsigned char uuid[16];
    int i;
    char* ptr = buf;
    struct timeval tv;
    int max_length = SIZE_OF_UUID_STRING;

    if ( !bNoPrefix ) max_length += 5;      // space for "uuid:"
    if ( size < max_length )
        return 0;

    if ( buf == NULL )
        return 0;

    // get time data
    gettimeofday( &tv, NULL );
    longTimeVal = (u_int64_t)1000000 * (u_int64_t)tv.tv_sec + (u_int64_t)tv.tv_usec;
    timeLow = longTimeVal & 0xffffffff;     // lower 32 bits
    timeMid = (longTimeVal >> 32) & 0xffff; // middle 16 bits
    timeHigh = (longTimeVal >> 32) & 0xfff; // upper 12 bits

    // update clock sequence number
    clock_sequence++;

    // get mac address
    if ( mac_addr_sys( mac ) < 0 )
        return 0;

    // now assemble UUID from fragments above
    for( i = 0; i < 6; i++ )
        uuid[i] = mac[i];                       // mac address

    uuid[6] = clock_sequence & 0xff;                // clock seq. low
    uuid[7] = 0x80 | ((clock_sequence >> 8) & 0x3f);// clock seq. high and variant
    uuid[8] = timeHigh & 0xff;                      // time high/version low bits
    uuid[9] = timeHigh >> 8 | 0x10;                 // time high/version high bits
    *(short int*)(uuid+10) = (short int)timeMid;
    *(int*)(uuid+12) = timeLow;

    // sha1 the uuid above, to mix things up
    //icrypto_sha1_calc( uuid, 16, sha1uuid );

    // convert these bytes into chars at output


    if ( !bNoPrefix )
    {
        sprintf( ptr, "uuid:" );
        ptr += 5;
    }

    sprintf( ptr, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[15], uuid[14], uuid[13], uuid[12],
            uuid[11], uuid[10], uuid[9], uuid[8],
            uuid[7], uuid[6], uuid[5], uuid[4],
            uuid[3], uuid[2], uuid[1], uuid[0] );

    return 1;

}


static GMutex *fw_mutex = NULL;

void soap_initialize_lock(void *data)
{
    fw_mutex = (GMutex *)data;
    g_assert(fw_mutex != NULL );
    fw_mutex = g_mutex_new();
    wsman_debug(WSMAN_DEBUG_LEVEL_DEBUG,"lock initialize");
    /*
    pthread_mutexattr_t attrVal;
    pthread_mutexattr_init( &attrVal );
    pthread_mutexattr_settype( &attrVal, PTHREAD_MUTEX_RECURSIVE_NP );
    if ( data != NULL )
        pthread_mutex_init((pthread_mutex_t*)data, &attrVal);
    // return data;
    */
}

gboolean soap_fw_trylock(void* data)
{
    gboolean try = FALSE;
    if ( data )
    {
        g_mutex_trylock(fw_mutex);
        //pthread_mutex_lock( (pthread_mutex_t*)data );
    }
    return try;
}

void soap_fw_lock(void* data)
{
    if ( data )
    {
        g_mutex_lock(fw_mutex);
        //pthread_mutex_lock( (pthread_mutex_t*)data );
    }
}


void soap_destroy_lock(void* data)
{
    if ( data )
    {
        g_mutex_free(fw_mutex);
    }
}

void soap_fw_unlock(void* data)
{
    if ( data )
    {	
        g_mutex_unlock(fw_mutex);
        //pthread_mutex_unlock((pthread_mutex_t*)data);
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


GHashTable* parse_query (char *query)
{
    GHashTable *args;
    gchar **vec, **l;
    gchar *key, *val;
    args = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    if (query && query[0])
    {
        vec = g_strsplit (query, "&", 0);
        for (l = vec; *l; l++) {
            key = g_strdup (*l);
            val = strchr (key, '=');
            if(val) {
                *val = 0;
                val++;
            }
            g_hash_table_replace (args, key, val);
        }
        g_strfreev (vec);
    }
    return args;
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

char* skip_white_spaces(char* ptr)
{
    if ( ptr )
        while(isspace(*ptr))
            ptr++;
    return ptr;
}




