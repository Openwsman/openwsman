/*
 * helpers.c
 *
 * helper functions for Java
 * 
 * Author: Klaus Kaempf <kkaempf@suse.de>
 * 
 */

/*****************************************************************************
* Copyright (C) 2008 Novell Inc. All rights reserved.
* Copyright (C) 2008 SUSE Linux Products GmbH. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*   - Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
* 
*   - Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
* 
*   - Neither the name of Novell Inc. nor of SUSE Linux Products GmbH nor the
*     names of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell Inc. OR SUSE Linux Products GmbH OR
* THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
 


/*
 * callback function if client authentication fails
 *
 */
static void
auth_request_callback( WsManClient *client, wsman_auth_type_t t, char **username, char **password )
{
    *username = NULL;		/* abort authentication */
    return;
}

#ifndef JCALL0
#ifdef __cplusplus
#   define JCALL0(func, jenv) jenv->func()
#   define JCALL1(func, jenv, ar1) jenv->func(ar1)
#   define JCALL2(func, jenv, ar1, ar2) jenv->func(ar1, ar2)
#   define JCALL3(func, jenv, ar1, ar2, ar3) jenv->func(ar1, ar2, ar3)
#   define JCALL4(func, jenv, ar1, ar2, ar3, ar4) \
	jenv->func(ar1, ar2, ar3, ar4)
#   define JCALL7(func, jenv, ar1, ar2, ar3, ar4, ar5, ar6, ar7) \
	jenv->func(ar1, ar2, ar3, ar4, ar5, ar6, ar7)
#else
#   define JCALL0(func, jenv) (*jenv)->func(jenv)
#   define JCALL1(func, jenv, ar1) (*jenv)->func(jenv, ar1)
#   define JCALL2(func, jenv, ar1, ar2) (*jenv)->func(jenv, ar1, ar2)
#   define JCALL3(func, jenv, ar1, ar2, ar3) (*jenv)->func(jenv, ar1, ar2, ar3)
#   define JCALL4(func, jenv, ar1, ar2, ar3, ar4) \
	(*jenv)->func(jenv, ar1, ar2, ar3, ar4)
#   define JCALL7(func, jenv, ar1, ar2, ar3, ar4, ar5, ar6, ar7) \
	(*jenv)->func(jenv, ar1, ar2, ar3, ar4, ar5, ar6, ar7)
#endif
#endif

static jobject hash2value(JNIEnv *jenv, hash_t *hash)
{
	hnode_t *node;
	hscan_t ptr;
	jclass cls = JCALL1(FindClass, jenv, "java/util/HashMap");
	jobject dict = JCALL3(NewObject, jenv, cls,
			      JCALL3(GetMethodID, jenv, cls, "<init>", "(I)V"),
			      (jint) hash_count(hash));
	jmethodID put = JCALL3(GetMethodID, jenv, cls, "put",
			       "(Ljava/lang/Object;Ljava/lang/Object;)"
			       "Ljava/lang/Object;");

	if (!hash)
		return NULL;

	hash_scan_begin(&ptr, hash);

	while ((node = hash_scan_next(&ptr))) {
		const void *key = hnode_getkey(node);
		const void *val = hnode_get(node);

		JCALL4(CallObjectMethod, jenv, dict, put,
		       JCALL1(NewStringUTF, jenv, key),
		       JCALL1(NewStringUTF, jenv, val));
	}

	return dict;
}
