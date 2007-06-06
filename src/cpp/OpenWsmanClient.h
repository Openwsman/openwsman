//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2007.
//
//  File:       OpenWsmanClient.cpp
//
//  Contents:   An implementation of the WsmanClient interface using openwsman
//
//----------------------------------------------------------------------------

#ifndef __OPEN_WSMAN_CLIENT_H
#define __OPEN_WSMAN_CLIENT_H

#include "WsmanClient.h"

struct _WsManClient;
typedef struct _WsManClient WsManClient; // FW declaration of struct
struct WsManClientData;	

namespace WsmanClientNamespace
{
	class OpenWsmanClient : public WsmanClient
	{
	private:
		WsManClient* cl;
	public:
		// Construct from params.
		OpenWsmanClient(const char *host = "localhost",
						const int port = 80,
						const char *path = "/wsman",
						const char *scheme = "http",
						const char *auth_method = "digest",
						const char *username = NULL,
		      const char *password = NULL
#ifdef _WIN32
						// determines which cert store to search
		      ,const bool local = false,
						// search for a client cert with this name
						const char *cert = NULL,
						// search for a client cert with this oid
		      const char *oid = NULL
#endif
		      );


		// Destructor.
		virtual ~OpenWsmanClient();

      // Creates a new instance of a resource.
		string Create(const string &resourceUri, const string &data) const;

      // Delete a resource.
		void Delete(const string &resourceUri, const NameValuePairs *s = NULL) const;

      // Enumerate resource. 
		void Enumerate(const string &resourceUri, vector<string> &enumRes, const NameValuePairs *s = NULL) const;

      // Retrieve a resource.
		string Get(const string &resourceUri, const NameValuePairs *s = NULL) const;

      // Update a resource.
		string Put(const string &resourceUri, const string &content, const NameValuePairs *s = NULL) const;

      // Invokes a method and returns the results of the method call.
		string Invoke(const string &resourceUri, const string &methodName, const string &content, const NameValuePairs *s = NULL) const;

	  // Set auth method
	  void SetAuth(const char *auth_method = "digest");

#ifdef _WIN32
	  // Set client certificate params
      void SetClientCert(const char *caOid=NULL, const char *caName=NULL, const bool localCert=false);
#else
      // Set server certificate params
      void SetServerCert(const char *cainfo=NULL, const char *capath=NULL);
      
      // Set client certificates params
      void SetClientCert(const char *cert, const char *key);
#endif
	};
} // namespace WsmanClient
#endif
