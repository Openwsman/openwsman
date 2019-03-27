#ifndef __WSMAN_EPR_H
#define __WSMAN_EPR_H

#include <string>
#include "wsman-epr.h"

using namespace std;

namespace WsmanClientNamespace {

class WsmanEPR {
private:
	epr_t *epr;

public:
	WsmanEPR(const string &eprstring);
	WsmanEPR(const string &uri, const string &address);
	WsmanEPR(WsmanEPR &epr);
	WsmanEPR& operator=(const WsmanEPR &other);
	~WsmanEPR();

	int addTextSelector(const string &name, const string &value);
	int addEprSelector(const string &name, WsmanEPR &epr);
	int deleteSelector(const string &name);

	bool operator==(const WsmanEPR &rhs) const;
	bool operator!=(const WsmanEPR &rhs) const;

	epr_t *getepr() const;
	operator epr_t*() const;
};

}

#endif
