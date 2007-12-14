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
 * @author Sumeet Kukreja, Dell Inc.
 */

#ifndef WSMAN_DECLARATIONS_H_
#define WSMAN_DECLARATIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "wsman-soap.h"

            // Server Plugin Interface Declarations

#define START_END_POINTS(t) WsDispatchEndPointInfo t##_EndPoints[] = {

#define END_POINT_IDENTIFY(t, ns)                           \
  { WS_DISP_TYPE_IDENTIFY, NULL, NULL, NULL, NULL,          \
      t##_TypeInfo, (WsProcType)t##_Identify_EP, ns, NULL}

#define END_POINT_TRANSFER_GET(t, ns)                                \
  { WS_DISP_TYPE_GET, NULL, NULL, TRANSFER_ACTION_GET, NULL,         \
      t##_TypeInfo, (WsProcType)t##_Get_EP, ns, NULL}

#define END_POINT_TRANSFER_DELETE(t, ns)                                \
  { WS_DISP_TYPE_DELETE, NULL, NULL, TRANSFER_ACTION_DELETE, NULL,         \
      t##_TypeInfo, (WsProcType)t##_Delete_EP, ns, NULL}

#define END_POINT_TRANSFER_CREATE(t, ns)                                \
  { WS_DISP_TYPE_CREATE, NULL, NULL, TRANSFER_ACTION_CREATE, NULL,         \
      t##_TypeInfo, (WsProcType)t##_Create_EP, ns, NULL}

#define END_POINT_TRANSFER_DIRECT_GET(t, ns)                         \
  { WS_DISP_TYPE_DIRECT_GET, NULL, NULL, TRANSFER_ACTION_GET, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Get_EP, ns, NULL}     

#define END_POINT_TRANSFER_DIRECT_PUT(t, ns)                         \
  { WS_DISP_TYPE_DIRECT_PUT, NULL, NULL, TRANSFER_ACTION_PUT, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Put_EP, ns, NULL}     

#define END_POINT_TRANSFER_DIRECT_DELETE(t, ns)                         \
  { WS_DISP_TYPE_DIRECT_DELETE, NULL, NULL, TRANSFER_ACTION_DELETE, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Delete_EP, ns, NULL}     
    
#define END_POINT_TRANSFER_DIRECT_CREATE(t, ns)                         \
  { WS_DISP_TYPE_DIRECT_CREATE, NULL, NULL, TRANSFER_ACTION_CREATE, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Create_EP, ns, NULL}     

#define END_POINT_TRANSFER_GET_NAMESPACE(t, ns)                         \
  { WS_DISP_TYPE_GET_NAMESPACE, NULL, NULL, TRANSFER_ACTION_GET, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Get_EP, ns, NULL}     

#define END_POINT_TRANSFER_PUT(t, ns)                          \
  { WS_DISP_TYPE_PUT, NULL, NULL, TRANSFER_ACTION_PUT, NULL,   \
      t##_TypeInfo, (WsProcType)t##_Put_EP, ns, NULL}

#define END_POINT_ENUMERATE(t, ns)                                   \
  { WS_DISP_TYPE_ENUMERATE, NULL, NULL, ENUM_ACTION_ENUMERATE, NULL, \
      t##_TypeInfo, (WsProcType)t##_Enumerate_EP, ns, NULL}

#define END_POINT_RELEASE(t, ns)                                  \
  { WS_DISP_TYPE_RELEASE, NULL, NULL, ENUM_ACTION_RELEASE, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Release_EP, ns, NULL}

#define END_POINT_PULL(t, ns)                               \
  { WS_DISP_TYPE_PULL, NULL, NULL, ENUM_ACTION_PULL, NULL,  \
      t##_TypeInfo, (WsProcType)t##_Pull_EP, ns, NULL}

#define END_POINT_DIRECT_PULL(t, ns)                              \
  { WS_DISP_TYPE_DIRECT_PULL, NULL, NULL, ENUM_ACTION_PULL, NULL, \
      t##_TypeInfo, (WsProcType)t##_Pull_EP, ns, NULL}    

#define END_POINT_SUBSCRIBE(t,ns)	\
{WS_DISP_TYPE_SUBSCRIBE,NULL,NULL,EVT_ACTION_SUBSCRIBE, NULL, \
 	t##_TypeInfo,(WsProcType)t##_Subscribe_EP,ns,NULL}

#define END_POINT_UNSUBSCRIBE(t,ns)   \
{WS_DISP_TYPE_UNSUBSCRIBE,NULL,NULL,EVT_ACTION_UNSUBSCRIBE, NULL,    \
	t##_TypeInfo,(WsProcType)t##_UnSubscribe_EP,ns,NULL}

#define END_POINT_RENEW(t,ns)	\
{WS_DISP_TYPE_RENEW,NULL,NULL,EVT_ACTION_RENEW, NULL,	\
	t##_TypeInfo,(WsProcType)t##_Renew_EP,ns,NULL}


#define END_POINT_PRIVATE_EP(t, ns)                \
  { WS_DISP_TYPE_PRIVATE, NULL, NULL, a, NULL,           \
      t##_TypeInfo, (WsProcType)t##_##m##_EP, ns, NULL }

#define END_POINT_CUSTOM_METHOD(t, ns)                      \
  { WS_DISP_TYPE_PRIVATE, NULL, NULL, NULL, NULL,           \
      t##_TypeInfo, (WsProcType)t##_Custom_EP, ns, NULL }


#define END_POINT_LAST  { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

#define FINISH_END_POINTS(t) END_POINT_LAST }



        // Server Plugin NamesSpaces Array

#define START_NAMESPACES(t) WsSupportedNamespaces t##_Namespaces[] = {
#define ADD_NAMESPACE( ns , prefix )            \
  {ns, prefix }
#define NAMESPACE_LAST  { NULL , NULL }
#define FINISH_NAMESPACES(t) NAMESPACE_LAST }


        // Server Plugin EndPoints Array
#define DECLARE_EP_ARRAY(t)\
extern WsDispatchEndPointInfo t##_EndPoints[]



#ifdef __cplusplus
}
#endif

#endif
