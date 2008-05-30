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
			// Copy constructor is declared private
			OpenWsmanClient(const OpenWsmanClient& cl);
			// operator = is declared private
			OpenWsmanClient& operator =(const OpenWsmanClient& cl);
		public:
			// Construct from params.
			OpenWsmanClient(const char *host = "localhost",
					const int port = 80,
					const char *path = "/wsman",
					const char *scheme = "http",
					const char *auth_method = "digest",
					const char *username = NULL,
					const char *password = NULL,
					// search for a client proxy address include proxy port
					const char *proxy = NULL,
					// search for a client proxy user name
					const char *proxy_username = NULL,
					// search for a client proxy password
					const char *proxy_password = NULL
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

			// Identify.
			string Identify() const;

			// Delete a resource.
			void Delete(const string &resourceUri, const NameValuePairs *s = NULL) const;

			// Enumerate resource.
			void Enumerate(const string &resourceUri, vector<string> &enumRes, const NameValuePairs *s = NULL) const;

			void Enumerate(const string & resourceUri, WsmanFilter & filter, vector<string> &enumRes) const;

			// Retrieve a resource.
			string Get(const string &resourceUri, const NameValuePairs *s = NULL) const;

			// Update a resource.
			string Put(const string &resourceUri, const string &content, const NameValuePairs *s = NULL) const;

			// Invokes a method and returns the results of the method call.
			string Invoke(const string &resourceUri, const string &methodName, const string &content, const NameValuePairs *s = NULL) const;

			// Submit a subscription
			string Subscribe(const string &resourceUri, const SubscribeInfo &info, string &subsContext) const;

			// Renew a subscription
			string Renew(const string &resourceUri, const string &subsContext, float expire, const NameValuePairs *s = NULL) const;

			// Terminate a subscription
			void Unsubscribe(const string &resourceUri, const string &subsContext, const NameValuePairs *s = NULL) const;

			// Set auth method
			void SetAuth(const char *auth_method = "digest");

			// Set user name
			void SetUserName(const char *user_name);

			// Set passsword
			void SetPassword(const char *password);

			// Set encoding
			void SetEncoding(const char *encoding);
			// Set CIM namespace
			void SetNamespace(const char *ns);

			void SetProxy(const char *proxy=NULL, const char *proxy_username=NULL, const char *proxy_password=NULL);
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
