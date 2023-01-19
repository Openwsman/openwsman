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

#include <cstdlib>
#include "WsmanEPR.h"
#include "WsmanFilter.h"

using namespace WsmanClientNamespace;

WsmanFilter::WsmanFilter(const WsmanFilter &filter)
	: filter(filter_copy(filter.getFilter()))
{
}

WsmanFilter::WsmanFilter(const string &dialect, const string &query)
	: filter(filter_create_simple(dialect.c_str(), query.c_str()))
{
}

WsmanFilter::WsmanFilter(const NameValuePairs *s)
	: filter(NULL)
{
	addSelectors(s);
}

WsmanFilter& WsmanFilter::operator=(const WsmanFilter &other)
{
	if (this != &other)
		filter = filter_copy(other.getFilter());
	return *this;
}

WsmanFilter::WsmanFilter(
	const WsmanEPR &epr,
	enum WsmanAssocType assocType,
	const string &assocClass,
	const string &resultClass,
	const string &role,
	const string &resultRole,
	const vector<string> &resultProp)
{
	int i = 0;
	char **props = NULL;

	if (!resultProp.empty()) {
		props = new char* [resultProp.size()];
		vector<string>::const_iterator itr;
		for (itr = resultProp.begin(); itr != resultProp.end(); itr++, i++) {
			props[i] = const_cast<char*>(itr->c_str());
		}
	}

	filter = filter_create_assoc(
		epr,
		static_cast<int>(assocType),
		(assocClass.empty() ? NULL : assocClass.c_str()),
		(resultClass.empty()? NULL : resultClass.c_str()),
		(role.empty() ? NULL : role.c_str()),
		(resultRole.empty() ? NULL : resultRole.c_str()),
		props, i);

	 delete [] props;
}

WsmanFilter::~WsmanFilter()
{
	if (filter) {
		filter_destroy(filter);
		filter = NULL;
	}
}

filter_t *WsmanFilter::makeFilterSelector()
{
	if (!filter)
		filter = filter_create_selector(NULL);
	return filter;
}

int WsmanFilter::addSelector(const char *key, const char *value)
{
	return filter_add_selector(makeFilterSelector(), key, value);
}

int WsmanFilter::addSelector(const string &name, const string &value)
{
	return addSelector(name.c_str(), value.c_str());
}

void WsmanFilter::addSelectors(const NameValuePairs &selectors)
{
	NameValuePairs::const_iterator it;
	for (it = selectors.begin(); it != selectors.end(); ++it)
		addSelector(it->first, it->second);
}

void WsmanFilter::addSelectors(const NameValuePairs *selectors)
{
	if (!selectors)
		return;

	addSelectors(*selectors);
}

filter_t *WsmanFilter::getFilter() const
{
	return filter;
}

WsmanFilter::operator filter_t*() const
{
	return getFilter();
}
