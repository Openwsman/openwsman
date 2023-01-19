//----------------------------------------------------------------------------
//
//  Copyright (C) Red Hat, Inc., 2015.
//
//  File:       WsmanFilter.cpp
//
//  License:    BSD-3-Clause
//
//  Contents:   A C++ interface for filter_t
//
//----------------------------------------------------------------------------

#include "WsmanEPR.h"

using namespace WsmanClientNamespace;

WsmanEPR::WsmanEPR(const string &eprstring)
	: epr(epr_from_string(eprstring.c_str()))
{
}

WsmanEPR::WsmanEPR(const string &uri, const string &address)
	: epr(epr_create(uri.c_str(), NULL, address.c_str()))
{
}

WsmanEPR::WsmanEPR(WsmanEPR &epr)
	: epr(epr_copy(epr.getepr()))
{
}

WsmanEPR& WsmanEPR::operator=(const WsmanEPR &other)
{
	if (this != &other)
		epr = epr_copy(other.getepr());
	return *this;
}

WsmanEPR::~WsmanEPR()
{
	if (epr) {
		epr_destroy(epr);
		epr = NULL;
	}
}

int WsmanEPR::addTextSelector(const string &name, const string &value)
{
	return epr_add_selector_text(epr, name.c_str(), value.c_str());
}

int WsmanEPR::addEprSelector(const string &name, WsmanEPR &epr)
{
	return epr_add_selector_epr(epr, name.c_str(), epr.getepr());
}

int WsmanEPR::deleteSelector(const string &name)
{
	return epr_delete_selector(epr, name.c_str());
}

bool WsmanEPR::operator==(const WsmanEPR &rhs) const
{
	return epr_cmp(epr, rhs.epr) == 0;
}

bool WsmanEPR::operator!=(const WsmanEPR &rhs) const
{
	return !(*this == rhs);
}

epr_t *WsmanEPR::getepr() const
{
	return epr;
}

WsmanEPR::operator epr_t*() const
{
	return getepr();
}
