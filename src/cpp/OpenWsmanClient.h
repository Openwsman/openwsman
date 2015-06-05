//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2007.
//            (C) Red Hat, Inc, 2015.
//
//  File:       OpenWsmanClient.h
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
			OpenWsmanClient(
				const string &host = string("localhost"),
				const int port = 80,
				const string &path = string("/wsman"),
				const string &scheme = string("http"),
				const string &auth_method = string("digest"),
				const string &username = string(),
				const string &password = string(),
				// search for a client proxy address include proxy port
				const string &proxy = string(),
				// search for a client proxy user name
				const string &proxy_username = string(),
				// search for a client proxy password
				const string &proxy_password = string()
#ifdef _WIN32
				// determines which cert store to search
				,const bool local = false,
				// search for a client cert with this name
				const string &cert = string(),
				// search for a client cert with this oid
				const string &oid = string()
#endif
			       );


			// Destructor.
			virtual ~OpenWsmanClient();

			// Creates a new instance of a resource.
			string Create(
				const string &resourceUri,
				const string &data) const;

			// Identify.
			string Identify() const;

			// Delete a resource.
			void Delete(
				const string &resourceUri,
				const NameValuePairs *s = NULL) const;

			// Enumerate resource.
			void Enumerate(
				const string &resourceUri,
				vector<string> &enumRes,
				const NameValuePairs *s = NULL) const;
			void Enumerate(
				const string &resourceUri,
				WsmanFilter & filter,
				vector<string> &enumRes) const;
			void Enumerate(
				const string &resourceUri,
				vector<string> &enumRes,
				const WsmanOptions &options,
				const WsmanFilter &filter = WsmanFilter()) const;

			// Retrieve a resource.
			string Get(
				const string &resourceUri,
				const WsmanOptions &options) const;
			string Get(
				const string &resourceUri,
				const NameValuePairs *s = NULL) const;

			// Update a resource.
			string Put(
				const string &resourceUri,
				const string &content,
				const NameValuePairs *s = NULL) const;

			// Invokes a method and returns the results of the method call.
			string Invoke(
				const string &resourceUri,
				const string &methodName,
				const WsmanOptions &options) const;
			string Invoke(
				const string &resourceUri,
				const string &methodName,
				const string &content,
				const WsmanOptions &options) const;
			string Invoke(
				const string &resourceUri,
				const string &methodName,
				const string &content,
				const NameValuePairs *s = NULL) const;

			// Submit a subscription
			string Subscribe(
				const string &resourceUri,
				const SubscribeInfo &info,
				string &subsContext) const;

			// Renew a subscription
			string Renew(
				const string &resourceUri,
				const string &subsContext,
				float expire,
				const NameValuePairs *s = NULL) const;

			// Terminate a subscription
			void Unsubscribe(
				const string &resourceUri,
				const string &subsContext,
				const NameValuePairs *s = NULL) const;

			// Set auth method
			void SetAuth(const string &auth_method = string("digest"));
			
			// Set timeout method
			void SetTimeout(unsigned long mtime);
			// Set user name
			void SetUserName(const string &user_name);

			// Set passsword
			void SetPassword(const string &password);

			// Set encoding
			void SetEncoding(const string &encoding);

			// Set/Get CIM namespace
			void SetNamespace(const string &ns);
			string GetNamespace() const;

			void SetProxy(
				const string &proxy = string(),
				const string &proxy_username = string(),
				const string &proxy_password = string());
#ifdef _WIN32
			// Set client certificate params
			void SetClientCert(
				const string &caOid = string(),
				const string &caName = string(),
				const bool localCert = false);
#else
			// Set server certificate params
			void SetServerCert(
				const string &cainfo = string(),
				const string &capath = string());

			// Set client certificates params
			void SetClientCert(
				const string &cert,
				const string &key);
#endif
	};
} // namespace WsmanClient
#endif
