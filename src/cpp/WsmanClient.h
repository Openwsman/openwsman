//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2003 - 2007.
//
//  File:       WsmanClient.h 
//
//  Contents:   A WS-MAN client interface
//
//----------------------------------------------------------------------------

#ifndef __WSMAN_CLIENT_INTERFACE
#define __WSMAN_CLIENT_INTERFACE

#include <string>
#include <vector>
#include <map>
#include "Exception.h"
#include "WsmanFilter.h"

using namespace std;
using namespace WsmanExceptionNamespace;

namespace WsmanClientNamespace
{
	// WS-MAN Client exceptions
	// Exception thrown when server error occurs
	class WsmanClientException : public GeneralWsmanException
	{
	public:
		WsmanClientException(const char* message,
			unsigned int err = WSMAN_GENERAL_ERROR)
			:GeneralWsmanException(message, err){}
		virtual ~WsmanClientException() throw() {}
	};

	// Exception thrown if the server returned a soap fault
	class WsmanSoapFault : public WsmanClientException
	{
	private:		
		string soapCodeValue;
		string soapSubcodeValue;
		string soapReason;
		string soapDetail;
	public:
		WsmanSoapFault(const char* message,
			const string& faultCode = "",
			const string& faultSubcode = "",
			const string& faultReason = "", 
			const string& faultDetail = "")
			:WsmanClientException(message, WSMAN_SOAP_FAULT)
		{
			SetWsmanFaultFields(faultCode, faultSubcode, faultReason, faultDetail);
		}
		virtual ~WsmanSoapFault() throw() {}		
		void SetWsmanFaultFields(const string& faultCode,
			const string& faultSubcode,
			const string& faultReason,
			const string& faultDetail) throw()
		{
			soapCodeValue = faultCode;
			soapSubcodeValue = faultSubcode;
			soapReason = faultReason;
			soapDetail = faultDetail;
		}
		string GetFaultCode() const throw() {return soapCodeValue;}
		string GetFaultSubcode() const throw() {return soapSubcodeValue;}
		string GetFaultReason() const throw() {return soapReason;}
		string GetFaultDetail() const throw() {return soapDetail;}
	};

	typedef enum {
		WSMAN_DELIVERY_PUSH = 0,
		WSMAN_DELIVERY_PUSHWITHACK,
		WSMAN_DELIVERY_EVENTS,
		WSMAN_DELIVERY_PULL
	}NotificationDeliveryMode;

	typedef struct __SubscribeInfo{
		string filter;
		string dialect;
		string delivery_uri;
		string refenceParam;
		NotificationDeliveryMode delivery_mode;
		NameValuePairs *selectorset;
		float heartbeat_interval;
		float expires;
	}SubscribeInfo;

	// Wsman Client Interface class
	class WsmanClient
	{
	public:
		// Destructor.
		virtual ~WsmanClient(){}

		// Identify
		virtual string Identify() const = 0;
		
		// Creates a new instance of a resource.
		virtual string Create(const string &resourceUri, const string &xmlData) const = 0;

		// Delete a resource.
		virtual void Delete(const string &resourceUri, const NameValuePairs *s = NULL) const = 0;

		// Enumerate a resource. 
		virtual void Enumerate(const string &resourceUri, vector<string> &enumRes, const NameValuePairs *s = NULL) const = 0;

		//Enumerate2
		virtual void Enumerate(const string &resourceUri, WsmanFilter &filter, vector<string> &enumRes) const = 0;

		// Retrieve a resource.
		virtual string Get(const string &resourceUri, const NameValuePairs *s = NULL) const = 0;

		// Update a resource.
		virtual string Put(const string &resourceUri, const string &content, const NameValuePairs *s = NULL) const = 0;

		// Invokes a method and returns the results of the method call.
		virtual string Invoke(const string &resourceUri, const string &methodName, const string &content, const NameValuePairs *s = NULL) const = 0;

		// Submit a subscription
		virtual string Subscribe(const string &resourceUri, const SubscribeInfo &info, string &subsContext) const = 0;

		// Renew a subscription
		virtual string Renew(const string &resourceUri, const string &subsContext, float expire, const NameValuePairs *s = NULL) const = 0;

		// Terminate a subscription
		virtual void Unsubscribe(const string &resourceUri, const string &subsContext, const NameValuePairs *s = NULL) const = 0;
	};
} // namespace WsmanClientNamespace
#endif
