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
#include <glib.h>

#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"

#include "ws_transport.h"
#include "ws_dispatcher.h"

#include "wsman-debug.h"







unsigned long soap_fw_make_unique_id(SOAP_FW* fw)
{
    unsigned long val;

    soap_fw_lock(fw);

    if ( !++fw->uniqueIdCounter )
        fw->uniqueIdCounter++;
    val = fw->uniqueIdCounter;

    soap_fw_unlock(fw);
    return val;
}


void make_listener_callback(void* soapFwPtr, SOAP_TRANSPORT* t, char* bindToUrl)
{
    SOAP_FW* fw = (SOAP_FW*)soapFwPtr;
    SOAP_CHANNEL* ch;

    ch = make_soap_channel(soap_fw_make_unique_id(fw),
            NULL,
            NULL, //DispatchAccept,
            fw,
            SOAP_CHANNEL_DEF_INACTIVITY_TIMEOUT,
            0,
            SOAP_CHANNEL_DEF_TCP_LISTENER_FLAGS);
    if ( ch )
    {
        soap_fw_lock(fw);
        DL_AddTail(&fw->channelList, &ch->node); 
        soap_fw_unlock(fw);
    }
}


// DestroySoapChannel
void destroy_soap_channel(SOAP_CHANNEL* ch, int reason)
{
    
    ch->flags |= SOAP_CHANNEL_FLAG_DESTROY_REQUESTED;
    if ( ch->recvDispatchCallback )
        ch->recvDispatchCallback(ch);

/*
    while( !DL_IsEmpty(&ch->outputList) )
    {
        DL_Node* node = DL_RemoveHead(&ch->outputList);
        destroy_output_chain((SOAP_OUTPUT_CHAIN*)node, reason); 
    }
*/    

    // SoapDestroyTransport(ch->transportData);
    // DestroyInputBuffer(ch->inputBuffer);

    soap_free(ch->relatesToMsgId);
    soap_free(ch->peerEprUuid);    
    soap_free(ch);
}


// MakeSoapChannel
SOAP_CHANNEL* make_soap_channel(unsigned long uniqueId,
        SOAP_TRANSPORT* transportData,
        void (*recvDispatchCallback)(SOAP_CHANNEL*),
        void* recvDispatchData,                
        unsigned long inactivityTimeout,
        unsigned long keepAliveTimeout,
        unsigned long flags)
{
    SOAP_CHANNEL* ch = (SOAP_CHANNEL*)soap_alloc(sizeof(SOAP_CHANNEL), 1);

    if ( ch != NULL )
    {
        ch->uniqueId = uniqueId;
        ch->flags = flags;

        ch->transportData = transportData;
        // ch->lastActivityTicks = SoapGetTicks();
        ch->inactivityTimeout = inactivityTimeout;
        ch->keepAliveTimeout = keepAliveTimeout;

        ch->recvDispatchCallback = recvDispatchCallback;
        ch->recvDispatchData = recvDispatchData;

        ch->inputBuffer = 
            (SOAP_INPUT_BUFFER*)soap_alloc(sizeof(SOAP_INPUT_BUFFER), 1);

        if ( ch->inputBuffer == NULL 
                ||
                (ch->inputBuffer->buf = soap_alloc(SOAP_INPUT_MAX_SIZE, 0)) == NULL )
        {
            destroy_soap_channel(ch, SOAP_SEND_CALLBACK_REASON_CANCELLED);
            ch = NULL;
        }
        else
        {
            ch->inputBuffer->bufSize = SOAP_INPUT_MAX_SIZE;
            reset_input_buffer(ch->inputBuffer, 0);
        }
    }

    if (ch)
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Created channel %i\n", ch->uniqueId);
    }
    return ch;
}


// ResetInputBuffer
void reset_input_buffer(SOAP_INPUT_BUFFER* buf, int bytesToDiscard)
{
    buf->payloadIndex = (-1);
    buf->payloadSize = (-1);
  

    if ( bytesToDiscard > 0 && bytesToDiscard < buf->currentSize )
    {
        memcpy(buf->buf, 
                &buf->buf[bytesToDiscard], 
                buf->currentSize - bytesToDiscard);
        buf->currentSize -= bytesToDiscard;
    }
    else
        buf->currentSize = 0;

    buf->buf[buf->currentSize] = 0;
}
