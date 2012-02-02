/*
 * perk/helpers.c
 *
 * helper functions to convert between perl and openwsman values
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
 

/* convert openwsman hash_t* to SV* (string pairs) */
static SV *
hash2value( hash_t *hash )
{
    return NULL;
}

/* create hash (h == NULL) or add to hash (h != NULL) from SV* */
static hash_t *
value2hash( hash_t *h, SV *v )
{
    return h;
}


/*
 * callback function if client authentication fails
 *
 */
static void
auth_request_callback( WsManClient *client, wsman_auth_type_t t, char **username, char **password )
{
  /* TODO: find out how to call a Perl function from here */
  
    *username = NULL;		/* abort authentication */
    return;
}
