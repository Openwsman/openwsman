//----------------------------------------------------------------------------
//
//  Copyright (C) Red Hat, Inc., 2015.
//
//  File:       WsmanOptions.h
//
//  License:    BSD-3-Clause
//
//  Contents:   A C++ interface for client_opt_t
//
//----------------------------------------------------------------------------

#ifndef __H_WSMAN_OPTIONS_H
#define __H_WSMAN_OPTIONS_H

#include <string>

extern "C" {
#include "wsman-api.h"
} 
#include "WsmanFilter.h" // For NameValuePairs

using namespace std;

namespace WsmanClientNamespace {
	class WsmanEPR;

	class WsmanOptions
	{
		private:
			client_opt_t *options;

			WsmanOptions(const WsmanOptions &copy);
			WsmanOptions &operator=(const WsmanOptions &rhs);

		public:
			WsmanOptions();
			WsmanOptions(unsigned long flags);
			~WsmanOptions();

			void setNamespace(const char *namespace_);
			void setNamespace(const string &namespace_);
			void setDeliveryMode(WsmanDeliveryMode delivery_mode);
			void setDeliveryURI(const char *delivery_uri);
			void setDeliveryURI(const string &delivery_uri);
			void setReference(const char *reference);
			void setReference(const string &reference);
			void setExpires(const float expires);
			void setHeartbeatInterval(const float heartbeat_interval);

			void addProperty(const char *key, const char *value);
			void addProperty(const string &key, const string &value);
			void addProperty(const string &key, const WsmanEPR &epr);

			void addSelector(const char *key, const char *value);
			void addSelector(const string &key, const string &value);
			void addSelectors(const NameValuePairs &selectors);
			void addSelectors(const NameValuePairs *selectors);

			void addFlag(unsigned long flag);
			void removeFlag(unsigned long flag);
			unsigned long getFlags() const;

			client_opt_t *getOptions() const;

			operator client_opt_t*() const;
	};
}

#endif // __H_WSMAN_OPTIONS_H
