#ifndef __H_WSMAN_FILTER_H
#define __H_WSMAN_FILTER_H

#include <map>
#include <string>
#include <vector>
#include "wsman-filter.h"

using namespace std;
typedef std::map<std::string, std::string> NameValuePairs;
typedef std::map<std::string, std::string>::const_iterator PairsIterator;


namespace WsmanClientNamespace {
        class WsmanEPR;

	enum WsmanAssocType {WSMAN_ASSOCIATED = 0, WSMAN_ASSOCIATOR};

	class WsmanFilter
        {
		private:
			filter_t *filter;

                        filter_t *makeFilterSelector();

		public:
			WsmanFilter(const WsmanFilter &filter);
			WsmanFilter& operator=(const WsmanFilter &other);
			WsmanFilter(const string &dialect, const string &query);
			WsmanFilter(const NameValuePairs *s = NULL);
                        WsmanFilter(
                            const WsmanEPR &epr,
                            enum WsmanAssocType assocType,
                            const string &assocClass = string(),
                            const string &resultClass = string(),
                            const string &role = string(),
                            const string &resultRole = string(),
                            const vector<string> &resultProp = vector<string>());
			virtual ~WsmanFilter();

                        int addSelector(const char *key, const char *value);
			int addSelector(const string &name, const string &value);

                        void addSelectors(const NameValuePairs &selectors);
                        void addSelectors(const NameValuePairs *s);

			filter_t *getFilter() const;

                        operator filter_t*() const;
	};
}

#endif
