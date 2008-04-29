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
	WsmanEPR(const string &eprstring) {
		epr = epr_from_string(eprstring.c_str());
	}
	WsmanEPR(const string &uri, const string &address) {
		epr = epr_create(uri.c_str(), NULL, address.c_str());
	}
	WsmanEPR(WsmanEPR &epr) {
		this->epr = epr_copy(epr.getepr());
	}
	~WsmanEPR() {
		epr_destroy(this->epr);
	};
	int addTextSelector(const string &name, const string &value) {
		return epr_add_selector_text(this->epr, name.c_str(), value.c_str());
	}
	int addEprSelector(const string &name, WsmanEPR &epr) {
		return epr_add_selector_epr(this->epr, name.c_str(), epr.getepr());
	}
	int deleteSelector(const string &name) {
		return epr_delete_selector(this->epr, name.c_str());
	}
	int compare(WsmanEPR *epr1, WsmanEPR *epr2) {
		return epr_cmp(epr1->getepr(), epr2->getepr());
	}
	epr_t *getepr() {
		return this->epr;
	}
};

}

#endif
