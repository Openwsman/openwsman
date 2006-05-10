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

#ifndef WS_TRANSPORT_H_
#define WS_TRANSPORT_H_




#define SOAP_ENVELOPE_MAX_SIZE						0x8000
#define SOAP_INPUT_MAX_SIZE			(SOAP_ENVELOPE_MAX_SIZE + 0x1000)
#define SOAP_MAX_RESENT_COUNT						10

#define SOAP_PROCESS_TIMEOUT_MAX					40  // dbg 10000
#define SOAP_PROCESS_TIMEOUT_MIN					10
#define SOAP_PROCESS_TIMEOUT_WAKE_UP				100 //dbg ???



#define SOAP_CHANNEL_DEF_TCP_CLIENT_FLAGS\
	(SOAP_CHANNEL_FLAG_OUT | SOAP_CHANNEL_FLAG_IN)

#define SOAP_CHANNEL_DEF_TCP_SERVER_FLAGS\
	(SOAP_CHANNEL_FLAG_IN)

#define SOAP_CHANNEL_DEF_TCP_LISTENER_FLAGS\
	(SOAP_CHANNEL_FLAG_IN | SOAP_CHANNEL_FLAG_LISTENER | SOAP_CHANNEL_KEEP_ALIVE)


#define SOAP_CHANNEL_DEF_INACTIVITY_TIMEOUT		0x10000 //dbg ??? 60000
#define SOAP_CHANNEL_DEF_KEEP_ALIVE_TIMEOUT		30000


#define SOAP_SEND_CALLBACK_REASON_COMPLETED			1
#define SOAP_SEND_CALLBACK_REASON_FAILED			2
#define SOAP_SEND_CALLBACK_REASON_SHUTDOWN			3
#define SOAP_SEND_CALLBACK_REASON_CANCELLED			4
#define SOAP_SEND_CALLBACK_REASON_NO_TRANSPORT		5


#define SOAP_CHANNEL_FLAG_DESTROY_REQUESTED		1
#define SOAP_CHANNEL_FLAG_IN					2
#define SOAP_CHANNEL_FLAG_OUT					4
#define SOAP_CHANNEL_FLAG_INOUT\
	(SOAP_CHANNEL_FLAG_IN | SOAP_CHANNEL_FLAG_OUT)
#define SOAP_CHANNEL_FLAG_LISTENER				8
#define SOAP_CHANNEL_KEEP_ALIVE					0x10
#define SOAP_CHANNEL_ASYNC						0x20
#define SOAP_TRANSPORT_DATAGRAM					0x40
#define SOAP_TRANSPORT_ACCEPTED					0x80
#define SOAP_TRANSPORT_HTTP						0x100
#define SOAP_TRANSPORT_HTTP_CHUNK				0x200
#define SOAP_TRANSPORT_UDP_MCAST				0x800

struct __SOAP_TRANSPORT
{
	void* transportData;
};
typedef struct __SOAP_TRANSPORT SOAP_TRANSPORT;


struct __SOAP_INPUT_BUFFER
{
	int payloadIndex;
	int bufSize;
	int currentSize;
	int payloadSize;	
	void* associatedTransportData;
		
	char* buf;
};
typedef struct __SOAP_INPUT_BUFFER SOAP_INPUT_BUFFER;

struct __SOAP_OUTPUT_BUFFER
{
	char *dataBuf;
	int size; // if negative, don't free buffer pointed to in the node->data
};
typedef struct __SOAP_OUTPUT_BUFFER SOAP_OUTPUT_BUFFER;

struct __SOAP_OUTPUT_CHAIN
{
	DL_Node node;

	unsigned long 	channelId;
	void* 			associatedTransportData;
	char* 			relatesToMsgId;
	char* 			destUrl;
	int 				errorCode;
	char* 			errorMsg;

	void (*callbackProc)(struct __SOAP_OUTPUT_CHAIN* chain,
						 int status);
	void* 			callbackData;

	int 				currentIndex; // in current data buffer
	int 				resendCount;
	int 				resendIndex;
	unsigned long 	lastSend;
	unsigned long 	resentTimeout[SOAP_MAX_RESENT_COUNT];

	//DL_Node* 		nextNodeToSend; // if not NULL ignore timeout, 
							 // otherwise follow timout rules
	//DL_List 			dataBufferList;
	SOAP_OUTPUT_BUFFER*  dataBuffer;
};
typedef struct __SOAP_OUTPUT_CHAIN SOAP_OUTPUT_CHAIN;



struct __SOAP_CHANNEL
{
	DL_Node node;
	unsigned long uniqueId;
	WsmanFaultCodeType FaultCodeType;
	WsmanFaultDetailType FaultDetailType;	
	unsigned long flags;
	
	SOAP_TRANSPORT* transportData;
	//struct __SOAP_FW* soapFW;
	char* peerEprUuid;
	char* listenUrl;
	
	unsigned long lastActivityTicks;
	unsigned long inactivityTimeout; // for partial recv if inputIndex != NULL 
	unsigned long keepAliveTimeout;  // for keep alive, if zero -- infinite

	void (*recvDispatchCallback)(struct __SOAP_CHANNEL* ch);
	void* recvDispatchData;
	SOAP_INPUT_BUFFER* inputBuffer;
	
	char* relatesToMsgId;

	DL_List outputList;
};
typedef struct __SOAP_CHANNEL SOAP_CHANNEL;


unsigned long soap_fw_make_unique_id(SOAP_FW* fw);
void make_listener_callback(void* soapFwPtr, SOAP_TRANSPORT* t, char* bindToUrl);
SOAP_CHANNEL* make_soap_channel(unsigned long uniqueId,
        SOAP_TRANSPORT* transportData,
        void (*recvDispatchCallback)(SOAP_CHANNEL*),
        void* recvDispatchData,
        unsigned long inactivityTimeout,
        unsigned long keepAliveTimeout,
        unsigned long flags);
        
        
        
        
void reset_input_buffer(SOAP_INPUT_BUFFER* buf, int bytesToDiscard);
        
void destroy_soap_channel(SOAP_CHANNEL* ch, int reason);

void process_input(SOAP_CHANNEL* ch, char *buf);
void soap_process(SoapH soap, char* buf);


#endif /*WS_TRANSPORT_H_*/
