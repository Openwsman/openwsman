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
 * @author Vadim Revyakin
 */

#ifndef _WSMAN_NAMESPACES_H_
#define _WSMAN_NAMESPACES_H_


#define XML_NS_SOAP_1_1             "http://schemas.xmlsoap.org/soap/envelope"
#define XML_NS_SOAP_1_2             "http://www.w3.org/2003/05/soap-envelope"


#define XML_NS_XML_NAMESPACES       "http://www.w3.org/XML/1998/namespace"  
#define XML_NS_ADDRESSING           "http://schemas.xmlsoap.org/ws/2004/08/addressing"
#define XML_NS_DISCOVERY            "http://schemas.xmlsoap.org/ws/2004/10/discovery"
#define XML_NS_EVENTING             "http://schemas.xmlsoap.org/ws/2004/08/eventing"
#define XML_NS_ENUMERATION          "http://schemas.xmlsoap.org/ws/2004/09/enumeration"
#define XML_NS_TRANSFER             "http://schemas.xmlsoap.org/ws/2004/09/transfer"
#define XML_NS_XML_SCHEMA           "http://www.w3.org/2001/XMLSchema"
#define XML_NS_SCHEMA_INSTANCE      "http://www.w3.org/2001/XMLSchema-instance"


#define XML_NS_OPENWSMAN            "http://schema.openwsman.org/2006/openwsman"

#define XML_NS_CIM_SCHEMA           "http://schemas.dmtf.org/wbem/wscim/1/common"
#define XML_NS_CIM_CLASS            "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"
#define XML_NS_CIM_BINDING          "http://schemas.dmtf.org/wbem/wsman/1/cimbinding.xsd"


// WS-Management
#define XML_NS_WS_MAN               "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
#define XML_NS_WSMAN_FAULT_DETAIL   "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail"

#define XML_NS_WS_MAN_CAT           "http://schemas.xmlsoap.org/ws/2005/06/wsmancat"

#define XML_NS_WSMAN_ID             "http://schemas.dmtf.org/wbem/wsman/identity/1/wsmanidentity.xsd"



#endif

