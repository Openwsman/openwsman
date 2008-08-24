/*******************************************************************************
* Copyright (C) 2008 Centrify Corp. All rights reserved.
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

#ifdef SHTTPD_GSS

#include "defs.h"

#include <gssapi/gssapi_generic.h>

#define THE_END "\tContent-Type: application/octet-stream"
#define FAKE_CT "Content-Type: application/soap+xml\r\n"
#define FAKE_CL "Content-Length: "
#define ENCRYPTED_BOUNDARY "-- Encrypted Boundary\r\n"
#define KERB_CONTENT_TYPE      "\tContent-Type: application/HTTP-Kerberos-session-encrypted\r\n"
#define ORIGINAL_CONTENT "\tOriginalContent: type=application/soap+xml;charset=UTF-16;Length="
#define STREAM_CONTENT_TYPE      "\tContent-Type: application/octet-stream\r\n"

void displayError(char *msg, OM_uint32 maj_stat, OM_uint32 min_stat);
int getCreds(char *service_name, gss_cred_id_t *server_creds);
int do_gss(struct conn *c);
int gss_encrypt(struct shttpd_arg *arg, char *input, int inlen, char **output, int *outlen);
char* gss_decrypt(struct shttpd_arg *arg, char *data, int len);
void getGssName(struct conn *c, char **user);

static void doDisplay(char *m,OM_uint32 code,int type)
{
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc msg;
    OM_uint32 msg_ctx;

    msg_ctx = 0;
    while (1)
    {
        maj_stat = gss_display_status(&min_stat, code,
                                      type, GSS_C_NULL_OID,
                                      &msg_ctx, &msg);
        DBG(("GSS-API error %s: %s\n", m,
             (char *)msg.value));
        (void) gss_release_buffer(&min_stat, &msg);

        if (!msg_ctx)
            break;
    }
}
void displayError(char *msg, OM_uint32 maj_stat, OM_uint32 min_stat)
{
    doDisplay(msg, maj_stat, GSS_C_GSS_CODE);
    doDisplay(msg, min_stat, GSS_C_MECH_CODE);
}

int getCreds(char *service_name, gss_cred_id_t *server_creds)
{
    gss_buffer_desc name_buf;
    gss_name_t server_name;
    OM_uint32 maj_stat, min_stat;

    name_buf.value = service_name;
    name_buf.length = strlen(name_buf.value) + 1;
    maj_stat = gss_import_name(&min_stat, &name_buf,
                               (gss_OID) GSS_C_NT_HOSTBASED_SERVICE, &server_name);
    if (maj_stat != GSS_S_COMPLETE)
    {
        displayError("importing name", maj_stat, min_stat);
        return -1;
    }

    maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
                                GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
                                server_creds, NULL, NULL);
    if (maj_stat != GSS_S_COMPLETE)
    {
        displayError("acquiring credentials", maj_stat, min_stat);
        return -1;
    }

    (void) gss_release_name(&min_stat, &server_name);

    return 0;
}

static int connectContext(
                                   void *buff, int len,
                                   gss_cred_id_t server_creds,
                                   gss_ctx_id_t *context,
                                   gss_buffer_t client_name,
                                   gss_buffer_t reply,
                                   OM_uint32 *ret_flags)
{
    gss_buffer_desc  recv_tok;
    gss_name_t client;
    gss_OID doid;
    OM_uint32 maj_stat, min_stat, acc_sec_min_stat;

    recv_tok.length = len;
    recv_tok.value = buff;

    maj_stat = gss_accept_sec_context(&acc_sec_min_stat,
                                      context,
                                      server_creds,
                                      &recv_tok,
                                      GSS_C_NO_CHANNEL_BINDINGS,
                                      &client,
                                      &doid,
                                      reply,
                                      ret_flags,
                                      0,
                                      0);


    if (maj_stat != GSS_S_COMPLETE)
    {
        displayError("accept",maj_stat,min_stat);
        return maj_stat;
    }
    maj_stat = gss_display_name(&min_stat, client, client_name, &doid);
    if(maj_stat == 0)
    {
        DBG(("Accept principal = %.*s", client_name->length, client_name->value));
        maj_stat = gss_release_name(&min_stat, &client);
    }
    return maj_stat;
}

int do_gss(struct conn *c)
{

    gss_cred_id_t gsscreds;
    gss_buffer_desc client_name;
    gss_buffer_desc reply;
    OM_uint32 retflags;
    if(getCreds("host@localhost", &gsscreds) != 0)
        return 0;

    // now find the beginning and end of the base64 encoded stuff

    struct vec  *auth_vec = &c->ch.auth.v_vec;
    char *p = (char *) auth_vec->ptr + 9;
    while ((*p == ' ') || (*p == '\t'))
    {
        p++;
    }
    char *pp = p;
    while ((*p != ' ') && (*p != '\t') && (*p != '\r')
           && (*p != '\n') && (*p != 0))
    {
        p++;
    }

    if (pp == p)
    {
        return 0;
    }
    *p = 0;

    // pp = start, p = end. Now decode

    char decbuf[p - pp]; // this is too long but we dont care
    int l = ws_base64_decode(pp, p - pp, decbuf, 4095);

    int ret = connectContext(decbuf, l, gsscreds, &c->gss_ctx, &client_name, &reply, &retflags);
    if (ret != 0)
    {
        send_server_error(c, 400, "Bad request");
        return 0;
    }

    // encode the reply
    char repbuf[reply.length * 2]; // again, too large but we dont care

    ws_base64_encode(reply.value, reply.length, repbuf);


    io_clear(&c->loc.io);
    c->loc.headers_len = c->loc.io.head = snprintf(c->loc.io.buf,
                                                      c->loc.io.size, "HTTP/1.1 200 \r\nWWW-Authenticate: Kerberos %s\r\n\r\n",
                                                      repbuf);
    c->status = 200;
    OM_uint32 min_stat;
    gss_release_buffer(&min_stat, &reply);
    c->loc.flags |= FLAG_RESPONSE_COMPLETE;
    return 2;
}


int gss_encrypt(struct shttpd_arg *arg, char *input, int inlen, char **output, int *outlen)
{
    struct conn *c = (struct conn *)arg->priv;
    OM_uint32 min_stat;
    gss_buffer_desc in_buf;
    gss_buffer_desc out_buf;
    int state;

    // convert to UTF-16

    char utf16[inlen * 2 + 2];
    char *u = utf16;
    *u++ = '\xff';
    *u++ = '\xfe';
    int i;
    for (i = 0; i < inlen; i++)
    {
        *u++ = input[i];
        *u++ = 0;
    }
    int msglen = inlen * 2 + 2; // the data converted to UTF16
    msglen += sizeof(ENCRYPTED_BOUNDARY);
    msglen += sizeof(KERB_CONTENT_TYPE);
    msglen += sizeof(ORIGINAL_CONTENT);
    msglen += 10; // computed length
    msglen += sizeof(STREAM_CONTENT_TYPE);
    msglen += sizeof(ENCRYPTED_BOUNDARY);
    msglen += 200; // encryption overhead (checksums, flags, confounders etc

    char *payload = (char*)malloc(msglen);
    if(!payload)
        return -1;
    char *p = payload;

    memcpy(p, ENCRYPTED_BOUNDARY, sizeof(ENCRYPTED_BOUNDARY) - 1);
    p += sizeof(ENCRYPTED_BOUNDARY) - 1;
    memcpy(p, KERB_CONTENT_TYPE, sizeof(KERB_CONTENT_TYPE) -1 );
    p += sizeof(KERB_CONTENT_TYPE) - 1;
    memcpy(p, ORIGINAL_CONTENT, sizeof(ORIGINAL_CONTENT) - 1);
    p += sizeof(ORIGINAL_CONTENT) - 1 ;
    p += snprintf(p, 30, "%d\r\n", inlen * 2 + 2 + 1); // windows wants this to be the size of the encrypted data
    // and gss adds an extra pad byte
    memcpy(p, ENCRYPTED_BOUNDARY, sizeof(ENCRYPTED_BOUNDARY) - 1);
    p += sizeof(ENCRYPTED_BOUNDARY) - 1;
    memcpy(p, STREAM_CONTENT_TYPE, sizeof(STREAM_CONTENT_TYPE) -1 );
    p += sizeof(STREAM_CONTENT_TYPE) - 1;

    in_buf.value = utf16;
    in_buf.length = inlen * 2 + 2;


    OM_uint32 maj_stat = gss_wrap(&min_stat, c->gss_ctx, 1, GSS_C_QOP_DEFAULT,
                                  &in_buf, &state, &out_buf);
    if (maj_stat != GSS_S_COMPLETE)
    {
        displayError("wrapping message", maj_stat, min_stat);
        return -1;
    }
    // thsi is the encryption space overhead
    *p++ = out_buf.length - in_buf.length - 1; // not quite sure why I need -1 but I do. Its all to do with the gss pad byte
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    memcpy(p, out_buf.value, out_buf.length);
    p += out_buf.length;

    gss_release_buffer(&min_stat, &out_buf);
    memcpy(p, ENCRYPTED_BOUNDARY, sizeof(ENCRYPTED_BOUNDARY) - 1);
    p += sizeof(ENCRYPTED_BOUNDARY) - 1;

    *output = payload;
    *outlen = p - payload;
    return 1; // OK
}

void getGssName(struct conn *c, char **user)
{
    OM_uint32 majStat, minStat;
    gss_name_t client;
    gss_OID doid;
    gss_buffer_desc client_name;
    *user = 0;
    majStat = gss_inquire_context(&minStat, c->gss_ctx, &client, 0,0,0,0,0,0);
    if (majStat != GSS_S_COMPLETE)
    {
        displayError("inq context", majStat, minStat);
        return;
    }
    majStat = gss_display_name(&minStat, client, &client_name, &doid);
    if (majStat != GSS_S_COMPLETE)
    {
        displayError("get display name", majStat, minStat);
        return;
    }
    *user = u_strndup(client_name.value, client_name.length);
    majStat = gss_release_name(&minStat, &client);
}

char* gss_decrypt(struct shttpd_arg *arg, char *data, int len)
{
    struct conn *c = (struct conn *)arg->priv;
    OM_uint32 maj_stat, min_stat;
    int conf_state;
    gss_buffer_desc enc;
    gss_buffer_desc plain;
    DBG(("Decypting message %d", len));
    char line[1000];
    char *in = data;
    int found = 0;
    while ((in - data) < len)
    {
        char *p = line;
        while ((*p++ = *in++) != '\n' && (p - line < sizeof(line) - 1));
        *p = 0;
        //DBG(("->%s", line));
        if (!memcmp(line, THE_END, sizeof(THE_END) - 1 ))
        {
            found = 1;
                break;
        }
    }
    if(!found)
    {
        DBG(("Bad enc message format"));
        return strdup("");
    }
    in += 4; // skip 2f000000, this is the GSS coding overhead
    char *p = in;
    found = 0;
    while (p - data < len)
    {
        if (!memcmp(p,"-- Enc", 6))
        {
            found = 1;
            break;
        }
        p++;
    }
    if(!found)
    {
        return strdup("");
    }
    enc.length= p - in;
    enc.value = in;

    maj_stat = gss_unwrap(&min_stat, c->gss_ctx, &enc, &plain,
                          &conf_state, (gss_qop_t *) NULL);
    if (maj_stat != GSS_S_COMPLETE)
    {
        displayError("unsealing message", maj_stat, min_stat);
        return strdup("");
    }

    char * pl = plain.value;

    // naive UTF-16 to ascii conversion

    char utf8[plain.length / 2 + 1];
    p = utf8;
    int j;
    pl += 2; // fffe utf-16 lead in
    for (j = 0; j < (plain.length - 2) / 2; j++)
    {
        *p++ = *pl++;
        pl++;
    }
    *p = 0;
    gss_release_buffer(&min_stat, &plain);
    return( strdup(utf8));
}
#endif
