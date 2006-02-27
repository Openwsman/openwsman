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
 * @author Haihao Xiang
 * @author Anas Nashif
 * @author Eugene Yarmosh
 */

#ifndef __WS_IPMI_LOGRECORD_H__
#define __WS_IPMI_LOGRECORD_H__

#define WS_IPMI_LOGRECORD_RESOURCE_URI \
		"http://schemas.dmtf.org/wsman/2005/06/cimv2.9/Intel_IPMI_LogRecord"

// The resource is modeled as a struct
struct __IpmiLogRecord
{
        char *LogCreationClassName;
        char *LogName;
        char *CreationClassName;
        char *RecordID;
        char *MessageTimestamp;

        char *RecordData;
};
typedef struct __IpmiLogRecord IpmiLogRecord;


// Service endpoint declaration
int IpmiLogRecord_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);
int IpmiLogRecord_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);
int IpmiLogRecord_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);
IpmiLogRecord* IpmiLogRecord_Get_EP(WsContextH cntx);
int IpmiLogRecord_Catch_EP(SoapOpH op, void* appData);


SER_DECLARE_TYPE(IpmiLogRecord);
SER_DECLARE_EP_ARRAY(IpmiLogRecord);
DECLARE_SELECTOR_ARRAY(IpmiLogRecord);

void get_endpoints(GModule *self, void **data);
int init (GModule *self, void **data );
void cleanup( GModule *self, void *data );

#endif // __WS_IPMI_LOGRECORD_H__
