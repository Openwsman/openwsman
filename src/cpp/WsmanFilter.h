#ifndef __H_WSMAN_FILTER_H
#define __H_WSMAN_FILTER_H

#include <string>
#include <vector>
#include <malloc.h>
#include "WsmanEPR.h"
#include "wsman-filter.h"

using namespace std;
typedef std::map<std::string, std::string> NameValuePairs;
typedef std::map<std::string, std::string>::const_iterator PairsIterator;

namespace WsmanClientNamespace {
	enum WsmanAssocType {WSMAN_ASSOCIATED = 0, WSMAN_ASSOCIATOR};
	class WsmanFilter {
		private:
			filter_t *filter;
		public:
			WsmanFilter(WsmanFilter &filter) {
				this->filter = filter_copy(filter.getfilter());
			}
			WsmanFilter(const string &dialect, const string &query) {
				filter = filter_create_simple(dialect.c_str(), query.c_str());
			}
			WsmanFilter(const NameValuePairs *s = NULL) {
				filter = filter_create_selector(NULL);
				if(s) {
					// Add selectors.
					for (PairsIterator p = s->begin(); p != s->end(); ++p) {
						if(p->second != "")
							filter_add_selector(filter,
									(char *)p->first.c_str(), (char *)p->second.c_str());
					}
				}
			}
			WsmanFilter(WsmanEPR *epr, enum WsmanAssocType assocType, const string *assocClass = NULL,
				const string *resultClass = NULL, const string *role = NULL, const string *resultRole = NULL,
				vector<string> *resultProp = NULL){
				int i = 0;
				char **props = NULL;
				if(resultProp) {
					props = (char **)malloc(sizeof(char *)*resultProp->size());
					vector<string>::iterator itr;
					for(itr = resultProp->begin(); itr != resultProp->end(); itr++, i++) {
						props[i] = (char *)itr->c_str();
					}
				}
				filter = filter_create_assoc(epr->getepr(), (int)assocType,
					(assocClass == NULL? NULL : assocClass->c_str()),
					(resultClass == NULL? NULL : resultClass->c_str()),
					(role == NULL? NULL : role->c_str()),
					(resultRole == NULL? NULL : resultRole->c_str()),
					props, i);
				free(props);
			}
			virtual ~WsmanFilter() {
				filter_destroy(filter);
			}
			int addSelector(const string &name, const string &value) {
				return filter_add_selector(this->filter, name.c_str(), value.c_str());
			}
			filter_t *getfilter() {
				return filter;
			}

	};
}

#endif
