/*******************************************************************************
* Copyright (C) 2004-2007 Intel Corp. All rights reserved.
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
 * @author Liang Hou
 */
#ifndef WSMAN_CIMINDICATION_PROCESSOR_H_
#define WSMAN_CIMINDICATION_PROCESSOR_H_

#include "u/buf.h"
#include "wsman-soap.h"

#define CIMXML_CIM "CIM"
#define CIMXML_CIMVERSION "CIMVERSION"
#define CIMXML_DTDVERSION "DTDVERSION"
#define CIMXML_MESSAGE "MESSAGE"
#define CIMXML_ID "ID"
#define CIMXML_PROTOCOLVERSION "PROTOCOLVERSION"
#define CIMXML_SIMPLEEXPREQ "SIMPLEEXPREQ"
#define CIMXML_SIMPLEEXPRSP "SIMPLEEXPRSP"
#define CIMXML_MULTIEXPREQ "MULTIEXPREQ"
#define CIMXML_MULTIEXPRSQ "MULTIEXPRSQ"
#define CIMXML_EXPMETHODCALL "EXPMETHODCALL"
#define CIMXML_EXPMETHODRESPONSE "EXPMETHODRESPONSE"
#define CIMXML_EXPPARAMVALUE "EXPPARAMVALUE"
#define CIMXML_IRETURNVALUE "IRETURNVALUE"
#define CIMXML_INSTANCE "INSTANCE"
#define CIMXML_PROPERTY "PROPERTY"
#define CIMXML_NAME "NAME"
#define CIMXML_VALUE "VALUE"
#define CIMXML_TYPE "TYPE"
#define CIMXML_CLASSNAME "CLASSNAME"
#define CIMXML_PROPERTYARRAY "PROPERTY.ARRAY"
#define CIMXML_VALUEARRAY "VALUE.ARRAY"

typedef struct {
        SoapH soap;
        char *uuid;
}cimxml_context;

typedef enum {
	CIMXML_STATUS_OK,
	CIMXML_STATUS_UNSUPPORTED_PROTOCOL_VERSIOIN,
	CIMXML_STATUS_MULTIPLE_REQUESTS_UNSUPPORTED,
	CIMXML_STATUS_UNSUPPORTED_CIM_VERSION,
	CIMXML_STATUS_UNSUPPORTED_DTD_VERSION,
	CIMXML_STATUS_REQUEST_NOT_VALID,
	CIMXML_STATUS_REQUEST_NOT_WELL_FORMED,
	CIMXML_STATUS_REQUEST_NOT_LOOSELY_VALID,
	CIMXML_STATUS_HEADER_MISMATCH,
	CIMXML_STATUS_UNSUPPORTED_OPERATION
}CIMXMLKnownStatusCode;

typedef struct {
	CIMXMLKnownStatusCode code;
	char *fault_msg;
}CimxmlStatus;

typedef struct {
	char	*charset;
	CimxmlStatus status;
	u_buf_t     *request;
	u_buf_t     *response;
	WsmanKnownStatusCode http_code;
}CimxmlMessage;

CimxmlMessage *cimxml_message_new(void);
void cimxml_message_destroy(CimxmlMessage *msg);
void CIM_Indication_call(cimxml_context *cntx, CimxmlMessage *message, void *opaqueData);

#endif

