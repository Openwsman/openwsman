//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2003 - 2006.
//
//  File:       OpenWsmanClient.h 
//
//  Contents:   A wrapper class for openweman
//
//----------------------------------------------------------------------------

#ifndef __OPEN_WSMAN_CLIENT_H
#define __OPEN_WSMAN_CLIENT_H

#include <string>
#include <vector>
#include <map>

using namespace std;

struct _WsManClient;
typedef struct _WsManClient WsManClient; // FW declaration of struct
struct WsManClientData;	

namespace WsmanClientNamespace
{
	/// <summary>
	/// Class for holding selectors.
	/// </summary>
	typedef std::map<std::string, std::string>				NameValuePairs;
	typedef std::map<std::string, std::string>::iterator	PairsIterator;

	// Exception thrown when transport error occurs
	class TransportException : public std::exception
	{
	private:
		int transferErrorCode;
	public:
		TransportException(int error, string what = "") : transferErrorCode(error){}
		virtual const char* what() const throw();
		~TransportException() throw(){}
		int GetTransportReturnCode() {return transferErrorCode;}
	};

	
	// Exception thrown when http error occurs
	class HttpException : public exception
	{
	private:
		long errorCode;
	public:
		HttpException(long error, string what = "") : errorCode(error){}
		  virtual const char* what() const throw();
		  ~HttpException() throw(){}
		  long GetErrorCode() {return errorCode;}
	};

	// Exception thrown when server error occurs
	class WsmanException : public exception
	{
	private:
		long httpCode;
		string codeValue;
		string subcodeValue;
		string reason;
		string detail;
	public:
		WsmanException(long errorC, string what = "") : 
		  httpCode(errorC), codeValue(""),
			  subcodeValue(""), reason(""), detail(""){}
		  virtual const char* what() const throw();
		  ~WsmanException() throw(){}
		  long GetHttpReturnCode() {return httpCode;}
		  void SetWsmanFaultFields(string& faultCode, string& faultSubcode,
			  string& faultReason, string& faultDetail)
		  {
			  codeValue = faultCode;
			  subcodeValue = faultSubcode;
			  reason = faultReason;
			  detail = faultDetail;
		  }
		  string GetFaultCode(){return codeValue;}
		  string GetFaultSubcode(){return subcodeValue;}
		  string GetFaultReason(){return reason;}
		  string GetFaultDetail(){return detail;}
	};

	class WsmanClient
	{
	private:
		WsManClient* cl;
		void GetWsmanFault(string xml, WsmanException &e);
		string ExtractPayload(string xml);
	public:
		// Construct from params.
		WsmanClient(const char *endpoint,
			const	char	*cert = NULL,
			const char *oid = NULL,			
			const char *auth_method = "digest");

		// Destructor.
		~WsmanClient();

		/// Creates a new instance of a resource.
		string Create(const string &resourceUri, const string &data);

		/// Delete a resource.
		void Delete(const string &resourceUri, NameValuePairs &s);

		/// Enumerate resource. 
		void Enumerate(const string &resourceUri, vector<string> &enumRes);

		/// Retrieve a resource.
		string Get(const string &resourceUri, NameValuePairs *s);

		/// Update a resource.
		string Put(const string &resourceUri, const string &content, NameValuePairs &s);

		/// Invokes a method and returns the results of the method call.
		string Invoke(const string &resourceUri, const string &methodName, const string &content, NameValuePairs &s);

		/// Initialize the transport layer, this method should be called only once before using this class.
		static void Init();
	};
}; // namespace WsmanClient
#endif
