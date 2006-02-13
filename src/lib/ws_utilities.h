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

#ifndef WS_UTILITIES_H_
#define WS_UTILITIES_H_




#ifdef __linux__
#define _stricmp(x,y) strcasecmp(x, y) 
#define stricmp(x,y) strcasecmp(x, y) 
#define _strnicmp(x,y,n) strncasecmp(x,y,n) 
//#define itoa(x, buf, r) sprintf(buf, (r == 10) ? "%i" : "%X", x)
#elif  defined(_WIN32)
#define stricmp(x, y) _stricmp(x, y)
#define itoa(x, buf, r) _itoa(x, buf, r)
#endif 




// Double linked list
struct __DL_Node
{
	struct __DL_Node* next;
	struct __DL_Node* prev;
	struct __DL_List * list;
	void* dataBuf;
};

struct __DL_List
{
	struct __DL_Node* head;
	struct __DL_Node* tail;
	int count;
	void * listOwner;
	//int listOwnerType;
};

typedef struct __DL_Node DL_Node;
typedef struct __DL_List DL_List;



/*
 * Parsing Query String from URI
 */
struct pair_t {
      char *name;
      char *value;
};

int xdigit_to_int(int c);
void uri_unescape(char *ptr);
struct pair_t * parse_query(const char *string, int separator);

/**/


void soap_get_uuid (char* buf, int size, int bNoPrefix);
void soap_fw_unlock(void* data);
void soap_fw_lock(void* data);

void soap_initialize_lock(void *);
char *soap_clone_string(char *str);
void *soap_clone(void *buf, int size);
void *soap_alloc(int size, int zeroInit);
void soap_free(void *ptr);


DL_Node *DL_MakeNode(DL_List *list, void *dataBuf);
void DL_AddHead(DL_List *list, DL_Node *node);
void DL_AddTail(DL_List *list, DL_Node *node);
void DL_AddAfter(DL_Node *node_to_add, DL_Node *add_after);
void DL_AddBefore(DL_Node *node_to_add, DL_Node *add_before);
DL_Node *DL_RemoveHead(DL_List *list);
DL_Node *DL_RemoveTail(DL_List *list);
DL_List *DL_RemoveNode(DL_Node *node);
DL_List *DL_FreeNode(DL_Node *node);
int DL_GetCount(DL_List *list);
int DL_IsEmpty(DL_List *list);
DL_Node *DL_GetHead(DL_List *list);
DL_Node *DL_GetTail(DL_List *list);
DL_Node *DL_GetNext(DL_Node *node);
DL_Node *DL_GetPrev(DL_Node *node);
void *DL_GetListOwner(DL_List *list);
void *DL_GetNodeData(DL_Node *node);
DL_List *DL_GetList(DL_Node *node);
void DL_RemoveAndDestroyAllNodes(DL_List *list, int freeDataBuf);
void DL_RemoveAndDestroyAllNodesCallback(DL_List *list, void (*callback)(void *, DL_Node *), void *callbackData);
DL_Node *DL_FindNode(DL_List *list, void *data, int (*proc)(void *, DL_Node *));


unsigned long soap_get_ticks(void);
int is_time_up(unsigned long lastTicks, unsigned long tm);
void soap_sleep(unsigned long tm);

#endif /*WS_UTILITIES_H_*/

