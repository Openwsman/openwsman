//----------------------------------------------------------------------------
//
//  Copyright (C) Red Hat, Inc., 2015.
//
//  File:       WsmanOptions.cpp
//
//  License:    BSD-3-Clause
//
//  Contents:   A C++ interface for client_opt_t
//
//----------------------------------------------------------------------------

#include <cstring>
extern "C" {
#include "wsman-client-api.h"
}
#include "WsmanEPR.h"
#include "WsmanOptions.h"

using namespace WsmanClientNamespace;

WsmanOptions::WsmanOptions()
	: options(wsmc_options_init())
{
	if (!options)
		throw std::bad_alloc();
}

WsmanOptions::WsmanOptions(unsigned long flags)
    : options(wsmc_options_init())
{
	if (options)
		options->flags = flags;
	else
		throw std::bad_alloc();
}

WsmanOptions::~WsmanOptions()
{
	if (options) {
		wsmc_options_destroy(options);
		options = NULL;
	}
}

void WsmanOptions::setNamespace(const char *namespace_)
{
	if (strlen(namespace_) == 0)
		return;
	wsmc_set_cim_ns(namespace_, options);
}

void WsmanOptions::setNamespace(const string &namespace_)
{
	setNamespace(namespace_.c_str());
}

void WsmanOptions::setDeliveryMode(WsmanDeliveryMode delivery_mode)
{
	wsmc_set_delivery_mode(delivery_mode, options);
}

void WsmanOptions::setDeliveryURI(const char *delivery_uri)
{
	wsmc_set_delivery_uri(delivery_uri, options);
}

void WsmanOptions::setDeliveryURI(const string &delivery_uri)
{
	setDeliveryURI(delivery_uri.c_str());
}

void WsmanOptions::setReference(const char *reference)
{
	wsmc_set_reference(reference, options);
}

void WsmanOptions::setReference(const string &reference)
{
	setReference(reference.c_str());
}

void WsmanOptions::setExpires(const float expires)
{
	options->expires = expires;
}

void WsmanOptions::setHeartbeatInterval(const float heartbeat_interval)
{
	options->heartbeat_interval = heartbeat_interval;
}

void WsmanOptions::addProperty(const char *key, const char *value)
{
	wsmc_add_property(options, key, value);
}

void WsmanOptions::addProperty(const string &key, const string &value)
{
	addProperty(key.c_str(), value.c_str());
}

void WsmanOptions::addProperty(const string &key, const WsmanEPR &epr)
{
	wsmc_add_property_epr(options, key.c_str(), epr);
}

void WsmanOptions::addSelector(const char *key, const char *value)
{
	wsmc_add_selector(options, key, value);
}

void WsmanOptions::addSelector(const string &key, const string &value)
{
	if (value.length() == 0)
		return;
	addSelector(key.c_str(), value.c_str());
}

void WsmanOptions::addSelectors(const NameValuePairs &selectors)
{
	NameValuePairs::const_iterator it;
	for (it = selectors.begin(); it != selectors.end(); ++it)
		addSelector(it->first, it->second);
}

void WsmanOptions::addSelectors(const NameValuePairs *selectors)
{
	if (!selectors)
		return;

	addSelectors(*selectors);
}

void WsmanOptions::addFlag(unsigned long flag)
{
	options->flags |= flag;
}

void WsmanOptions::removeFlag(unsigned long flag)
{
	options->flags &= ~flag;
}

unsigned long WsmanOptions::getFlags() const
{
	return options->flags;
}

client_opt_t *WsmanOptions::getOptions() const
{
	return options;
}

WsmanOptions::operator client_opt_t*() const
{
	return getOptions();
}
