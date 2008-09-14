/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 * @author Liang Hou
 * @author Sumeet Kukreja, Dell Inc.
 */
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <CimClientLib/cmci.h>
#include <CimClientLib/native.h>
#include "u/libu.h"


#include "wsman-xml-api.h"
#include "wsman-client-api.h"
#include "wsman-soap.h"
#include "wsman-soap-envelope.h"
#include "wsman-xml.h"
#include "wsman-epr.h"

#include "sfcc-interface.h"
#include "cim-interface.h"
#include "cim_data.h"

#define SYSTEMCREATIONCLASSNAME "CIM_ComputerSystem"
#define SYSTEMNAME "localhost.localdomain"

extern char *get_server_port(void);

typedef struct _sfcc_enumcontext {
	CimClientInfo *ecClient;
	CMPIEnumeration *ecEnumeration;
} sfcc_enumcontext;

static char *cim_find_namespace_for_class(CimClientInfo * client,
		WsEnumerateInfo * enumInfo,
		char *classname)
{
	char *ns = NULL;
	char *sub, *target_class = NULL;
	hscan_t hs;
	hnode_t *hn;
	if (strcmp(client->requested_class, "*")  &&
			enumInfo && (enumInfo->flags & WSMAN_ENUMINFO_POLY_EXCLUDE)) {
		if ( (enumInfo->flags & WSMAN_ENUMINFO_EPR ) &&
				!(enumInfo->flags & WSMAN_ENUMINFO_OBJEPR))  {
			target_class = classname;
		} else {
			target_class = client->requested_class;
		}
	} else {
		target_class = classname;
	}

	//debug("target class:%s", target_class);
	if (strstr(client->resource_uri, XML_NS_CIM_CLASS) != NULL &&
			(strcmp(client->method, TRANSFER_GET) == 0 ||
			 strcmp(client->method, TRANSFER_DELETE) == 0 ||
			 strcmp(client->method, TRANSFER_PUT) == 0)) {
		ns = u_strdup(client->resource_uri);
		return ns;
	}
	if (target_class && client->namespaces) {
		hash_scan_begin(&hs, client->namespaces);
		while ((hn = hash_scan_next(&hs))) {
			//debug("namespace=%s", (char *) hnode_get(hn));
			if ((sub =  strstr(target_class, (char *) hnode_getkey(hn)))) {
				ns = u_strdup_printf("%s/%s",(char *) hnode_get(hn), target_class);
				//debug("vendor namespace match...");
				break;
			}
		}
	}
	if (!ns)
		ns = u_strdup_printf("%s/%s", XML_NS_CIM_CLASS,
				target_class);
	return ns;
}

void
path2xml(CimClientInfo * client,
		WsXmlNodeH node, char *resource_uri, CMPIValue * val)
{
	int i = 0, numkeys = 0;
	char *_path_res_uri = NULL, *cv = NULL;
	WsXmlNodeH cimns, wsman_selector_set, refparam;

	CMPIObjectPath *objectpath = val->ref;
	CMPIString *namespace = objectpath->ft->getNameSpace(objectpath, NULL);
#if 0
	CMPIString *opstr = CMObjectPathToString(objectpath, NULL);
	debug("objectpath: %s", (char *) opstr->hdl);
	debug("namespace: %s", (char *) namespace->hdl);
#endif
	CMPIString *classname =  objectpath->ft->getClassName(objectpath, NULL);
	numkeys = objectpath->ft->getKeyCount(objectpath, NULL);

	ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_ADDRESS,
			WSA_TO_ANONYMOUS);
	refparam = ws_xml_add_child(node, XML_NS_ADDRESSING,
			WSA_REFERENCE_PARAMETERS,
			NULL);
	_path_res_uri = cim_find_namespace_for_class(client, NULL,
			(char *) classname->hdl);
	ws_xml_add_child_format(refparam, XML_NS_WS_MAN, WSM_RESOURCE_URI,
			"%s", _path_res_uri);
	u_free(_path_res_uri);

	wsman_selector_set = ws_xml_add_child(refparam,
			XML_NS_WS_MAN,
			WSM_SELECTOR_SET,
			NULL);

	for (i = 0; i < numkeys; i++) {
		CMPIString *keyname;
		WsXmlNodeH s = NULL;
		CMPIData data = objectpath->ft->getKeyAt(objectpath, i,
				&keyname, NULL);
		cv = (char *) value2Chars(data.  type, &data.  value);
		s = ws_xml_add_child(wsman_selector_set, XML_NS_WS_MAN,
				WSM_SELECTOR, cv );
		ws_xml_add_node_attr(s, NULL, "Name", (char *) keyname->hdl);
		if (cv)
			u_free(cv);
		if (keyname)
			CMRelease(keyname);
	}
	if (namespace->hdl != NULL) {
		cimns = ws_xml_add_child(wsman_selector_set,
				XML_NS_WS_MAN, WSM_SELECTOR,
				(char *) namespace->hdl);
		ws_xml_add_node_attr(cimns, NULL, "Name",
				CIM_NAMESPACE_SELECTOR);
	}

	if (classname) {
		CMRelease(classname);
	}
	if (namespace) {
		CMRelease(namespace);
	}
}



void
xml2property(CMPIInstance * instance,
		CMPIData data, char *name, char *value)
{

	CMPIStatus rc;
	CMPIType type = data.type;

	if (type & CMPI_ARRAY) {
		//
	} else if (type & CMPI_ENC) {
		switch (type) {
		case CMPI_instance:
			break;
		case CMPI_ref:
			break;
		case CMPI_args:
			break;
		case CMPI_filter:
			break;
		case CMPI_string:
		case CMPI_numericString:
		case CMPI_booleanString:
		case CMPI_dateTimeString:
		case CMPI_classNameString:
			rc = CMSetProperty(instance, name, value,
					CMPI_chars);
			debug("CMSetProperty: %d %s", rc.rc,  (rc.msg)? (char *)rc.msg->hdl : NULL );
			break;
		case CMPI_dateTime:
			break;
		}
	} else if (type & CMPI_SIMPLE) {
		int yes = 0;
		switch (type) {
		case CMPI_boolean:
			if (strcmp(value, "true") == 0)
				yes = 1;
			CMSetProperty(instance, name, (CMPIValue *) & yes,
					CMPI_boolean);
			break;
		case CMPI_char16:
			CMSetProperty(instance, name, value, CMPI_chars);
			break;
		}
	} else if (type & CMPI_INTEGER) {
		unsigned long tmp;
		unsigned long long tmp_ll;
		int val;
		long val_l;
		long long val_ll;
		switch (type) {
		case CMPI_uint8:
			tmp = strtoul(value, NULL, 10);
			CMSetProperty(instance, name, (CMPIValue *) & tmp,
					type);
			break;
		case CMPI_sint8:
			val = atoi(value);
			CMSetProperty(instance, name, (CMPIValue *) & val,
					type);
			break;
		case CMPI_uint16:
			tmp = strtoul(value, NULL, 10);
			CMSetProperty(instance, name, (CMPIValue *) & tmp,
					type);
			break;
		case CMPI_sint16:
			val = atoi(value);
			CMSetProperty(instance, name, (CMPIValue *) & val,
					type);
			break;
		case CMPI_uint32:
			tmp = strtoul(value, NULL, 10);
			CMSetProperty(instance, name, (CMPIValue *) & tmp,
					type);
			break;
		case CMPI_sint32:
			val_l = atol(value);
			CMSetProperty(instance, name,
					(CMPIValue *) & val_l, type);
			break;
		case CMPI_uint64:
			tmp_ll = strtoull(value, NULL, 10);
			CMSetProperty(instance, name, (CMPIValue *) & tmp,
					type);
			break;
		case CMPI_sint64:
			val_ll = atoll(value);
			CMSetProperty(instance, name,
					(CMPIValue *) & val_ll, type);
			break;
		}
	} else if (type & CMPI_REAL) {
		switch (type) {
		case CMPI_real32:
			break;
		case CMPI_real64:
			break;
		}
	}
}

void
property2xml(CimClientInfo * client, CMPIData data,
		const char *name, WsXmlNodeH node, char *resource_uri,
		int frag_type, int is_key)
{

	char *valuestr = NULL;

	if (CMIsArray(data)) {
		WsXmlNodeH nilnode;
		if (( client->flags & FLAG_CIM_SCHEMA_OPT ) == FLAG_CIM_SCHEMA_OPT
				&& data.state == CMPI_nullValue) {
			return;
		} else if (data.type == CMPI_null && data.state == CMPI_nullValue) {
			nilnode = ws_xml_add_child_sort(node, resource_uri, name, NULL);
			ws_xml_add_node_attr(nilnode, XML_NS_SCHEMA_INSTANCE, "nil", "true");
			return;
		}
		CMPIArray *arr = data.value.array;
		CMPIType eletyp = data.type & ~CMPI_ARRAY;
		int j, n;
		if (arr != NULL) {
			n = CMGetArrayCount(arr, NULL);
			for (j = 0; j < n; ++j) {
				CMPIData ele = CMGetArrayElementAt(arr, j, NULL);
				valuestr = value2Chars(eletyp, &ele.value);
				ws_xml_add_child_sort(node, resource_uri, name, valuestr);
				free(valuestr);
			}
		}
	} else {
		if (( client->flags & FLAG_CIM_SCHEMA_OPT ) == FLAG_CIM_SCHEMA_OPT
				&&  data.state == CMPI_nullValue) {
			return;
		} else if (data.type != CMPI_null && data.state != CMPI_nullValue) {
			WsXmlNodeH refpoint = NULL;
			WsXmlNodeH propnode;

			if (data.type == CMPI_ref) {
				refpoint = ws_xml_add_child_sort(node, resource_uri, name, NULL);
				path2xml(client, refpoint, resource_uri, &data.value);
			} else {
				valuestr = value2Chars(data.type, &data.value);
				if(frag_type == 2)
					ws_xml_set_node_text(node, valuestr);
				else if(frag_type == 1) {
					ws_xml_add_child(node, NULL, name, valuestr);
				}
				else {
					propnode = ws_xml_add_child_sort(node, resource_uri, name, valuestr);
					if (is_key == 0 &&
							(client->flags & WSMAN_ENUMINFO_EXT )) {
						ws_xml_add_node_attr(propnode, XML_NS_CIM_SCHEMA, "Key",
								"true");
					}
				}
				if (valuestr)
					u_free(valuestr);
			}
		} else {
			WsXmlNodeH nilnode = ws_xml_add_child_sort(node, resource_uri, name,
					NULL);
			ws_xml_add_node_attr(nilnode, XML_NS_SCHEMA_INSTANCE, "nil", "true");
		}
	}
}


static void cim_add_args(CimClientInfo * client, CMPIObjectPath *op,
		CMPIArgs * argsin)
{
	hscan_t hs;
	hnode_t *hn;
	hash_t * args = client->method_args;
	hash_scan_begin(&hs, args);
	while ((hn = hash_scan_next(&hs))) {
		CMAddArg(argsin, (char *) hnode_getkey(hn),
				(char *) hnode_get(hn), CMPI_chars);
	}
}



char *cim_get_namespace_selector(hash_t * keys)
{
	char *cim_namespace = NULL;
	selector_entry *sentry = NULL;
	hnode_t *hn = hash_lookup(keys, (char *) CIM_NAMESPACE_SELECTOR);
	if (hn) {
		sentry = (selector_entry *) hnode_get(hn);
		if(sentry->type == 1) return NULL;
		cim_namespace = sentry->entry.text;
		hash_delete(keys, hn);
		hnode_destroy(hn);
		u_free(sentry);
		debug("CIM Namespace: %s", cim_namespace);
	}
	return cim_namespace;
}

static int cim_add_keys_from_filter_cb(void *objectpath, const char* key,
		const char *value)
{
	CMPIObjectPath *op = (CMPIObjectPath *)objectpath;
	debug("adding selector %s=%s", key, value );
	if(strcmp(key, CIM_NAMESPACE_SELECTOR) == 0) //it is __cimnamespace
		CMSetNameSpace(op, value);
	else
		CMAddKey(op, key, value, CMPI_chars);
	return 0;
}

static CMPIObjectPath *
cim_epr_to_objectpath(epr_t *epr) {
	CMPIObjectPath * objectpath;
	char *class = NULL;
	if (epr && epr->refparams.uri) {
		debug("uri: %s", epr->refparams.uri);
		class = strrchr(epr->refparams.uri, '/') + 1;
	}
	// FIXME
	objectpath = newCMPIObjectPath(CIM_NAMESPACE, class, NULL);
	wsman_epr_selector_cb(epr, cim_add_keys_from_filter_cb, objectpath);

	debug( "ObjectPath: %s",
			CMGetCharPtr(CMObjectPathToString(objectpath, NULL)));

	return objectpath;

}

static void cim_add_keys(CMPIObjectPath * objectpath, hash_t * keys)
{
	hscan_t hs;
	hnode_t *hn;
	selector_entry *sentry;
	if (keys == NULL) {
		return;
	}
	hash_scan_begin(&hs, keys);
	while ((hn = hash_scan_next(&hs))) {
		sentry = (selector_entry *)hnode_get(hn);
		debug("in cim_add_keys:: text: %s", sentry->entry.text);
		if(sentry->type == 0)
			CMAddKey(objectpath, (char *) hnode_getkey(hn),
					sentry->entry.text, CMPI_chars);
		else {
			CMPIValue value;
			value.ref = cim_epr_to_objectpath(sentry->entry.eprp);;
			CMAddKey(objectpath, (char *) hnode_getkey(hn),
					&value, CMPI_ref);
		}

	}
}

static int
cim_verify_class_keys(CMPIConstClass * class,
		hash_t * keys, WsmanStatus * statusP)
{
	CMPIStatus rc;
	int count, ccount = 0;
	int numproperties, i;
	debug("verify class selectors");

	if (!keys) {
		count = 0;
	} else {
		count = (int) hash_count(keys);
	}
	numproperties = class->ft->getPropertyCount(class, NULL);
	debug("number of prop in class: %d", numproperties);
	for (i = 0; i < numproperties; i++) {
		CMPIString *propertyname;
		CMPIData data;
		class->ft->getPropertyAt(class, i, &propertyname, NULL);
		data = class->ft->getPropertyQualifier(class,
				(char *) propertyname->
				hdl, "Key", &rc);
		if ((data.state != CMPI_nullValue) && (data.value.boolean)) {
			ccount++;
		}
		CMRelease(propertyname);
	}

	debug("selector count from user: %d, in class definition: %d",
			count, ccount);
	if (ccount > count) {
		statusP->fault_code = WSMAN_INVALID_SELECTORS;
		statusP->fault_detail_code =
			WSMAN_DETAIL_INSUFFICIENT_SELECTORS;
		debug("insuffcient selectors");
		goto cleanup;
	} else if (ccount < hash_count(keys)) {
		statusP->fault_code = WSMAN_INVALID_SELECTORS;
		statusP->fault_detail_code =
			WSMAN_DETAIL_UNEXPECTED_SELECTORS;
		debug("invalid selectors");
		goto cleanup;
	}
cleanup:
	return statusP->fault_code;
}

static int
cim_opcmp(CMPIObjectPath * op1, CMPIObjectPath * op2) {
	CMPIStatus rc;
	unsigned int i = 0;
	unsigned int count = CMGetKeyCount(op2, &rc);
	debug("count = %d", count);
	char *cv1,*cv2;
	int match = 0;
	while (i < count) {
		CMPIString *name1 = NULL;
		CMPIData value1 = CMGetKeyAt(op2, i, &name1, &rc);
		if(rc.rc) goto DONE;
		i++;
		char *p1 = CMGetCharsPtr(name1, &rc);
		if(rc.rc)
			goto DONE;
		CMPIData value2 = CMGetKey(op1, p1, &rc);
		if(rc.rc) goto DONE;
		cv1 = value2Chars(value1.type, &value1.value);
		cv2 = value2Chars(value2.type, &value2.value);
		match = strcmp(cv1,cv2);
		u_free(cv1);
		u_free(cv2);
		if(match == 0)
			continue;
		else
			goto DONE;
	}
DONE:
	CMRelease(op1);
	CMRelease(op2);
	if(rc.rc || match)
		return 1;
	else
		return 0;
}

static int
cim_verify_keys(CMPIObjectPath * objectpath, hash_t * keys,
		WsmanStatus * statusP)
{
	CMPIStatus rc;
	hscan_t hs;
	hnode_t *hn;
	int count, opcount;
	char *cv = NULL;

	debug("verify selectors");

	if (!keys) {
		count = 0;
	} else {
		count = (int) hash_count(keys);
	}
	opcount = CMGetKeyCount(objectpath, &rc);
	debug("getKeyCount rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);


	debug("selector count from user: %d, in object path: %d", count,
			opcount);
	if (opcount > count) {
		statusP->fault_code = WSMAN_INVALID_SELECTORS;
		statusP->fault_detail_code = WSMAN_DETAIL_INSUFFICIENT_SELECTORS;
		debug("insuffcient selectors");
		goto cleanup;
	} else if (opcount < hash_count(keys)) {
		statusP->fault_code = WSMAN_INVALID_SELECTORS;
		debug("invalid selectors");
		goto cleanup;
	}

	hash_scan_begin(&hs, keys);

	while ((hn = hash_scan_next(&hs))) {
		CMPIData data = CMGetKey(objectpath, (char *) hnode_getkey(hn), &rc);
		if (rc.rc != 0) {	// key not found
			statusP->fault_code = WSMAN_INVALID_SELECTORS;
			statusP->fault_detail_code = WSMAN_DETAIL_UNEXPECTED_SELECTORS;
			debug("unexpcted selectors");
			break;
		}
		selector_entry *sentry = (selector_entry*)hnode_get(hn);
		if(sentry->type == 0) {
			cv = value2Chars(data.type, &data.value);
			if(cv != NULL && strcmp(cv, sentry->entry.text) == 0) {
				statusP->fault_code = WSMAN_RC_OK;
				statusP->fault_detail_code = WSMAN_DETAIL_OK;
				u_free(cv);
			}
			else {
				statusP->fault_code = WSA_DESTINATION_UNREACHABLE;
				statusP->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
				debug("selector value: [ %s ] not matched", sentry->entry.text);
				u_free(cv);
				break;
			}
		}
		else {
			CMPIObjectPath *objectpath_epr = cim_epr_to_objectpath(sentry->entry.eprp);
			CMPIObjectPath *objectpath_epr2 = CMClone(data.value.ref, NULL);
			if (cim_opcmp(objectpath_epr2, objectpath_epr) == 0) {
				statusP->fault_code = WSMAN_RC_OK;
				statusP->fault_detail_code = WSMAN_DETAIL_OK;
				u_free(cv);
			} else {
				statusP->fault_code = WSA_DESTINATION_UNREACHABLE;
				statusP->fault_detail_code =
					WSMAN_DETAIL_INVALID_RESOURCEURI;
				debug("invalid resource_uri %s != %s", cv,
						(char *)CMGetCharPtr(CMObjectPathToString(objectpath, NULL)));
				u_free(cv);
				break;
			}
		}
	}
cleanup:
	debug("getKey rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	return statusP->fault_code;
}



static CMPIConstClass *cim_get_class(CimClientInfo * client,
		const char *class,
		CMPIFlags flags, WsmanStatus * status)
{
	CMPIObjectPath *op;
	CMPIConstClass *_class;
	CMPIStatus rc;

	CMCIClient *cc = (CMCIClient *) client->cc;
	op = newCMPIObjectPath(client->cim_namespace, class, NULL);
	_class = cc->ft->getClass(cc, op, flags, NULL, &rc);

	debug("getClass() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	cim_to_wsman_status(rc, status);
	if (op)
		CMRelease(op);
	return _class;
}

static int filter_instance(CMPIInstance * instance, WsEnumerateInfo * enumInfo)
{
	filter_t *filter = enumInfo->filter;
	int i;
	int matches = 0;
	Selector *ss = filter->selectorset.selectors;
	if (ss == NULL) {
		debug("epr->refparams.selectors == NULL");
		return 0;
	}
	for (i = 0; i < filter->selectorset.count; i++) {
		Selector *s;
		s = ss + i;
		CMPIData data = instance->ft->getProperty(instance, s->name, NULL);
		char *valuestr = value2Chars(data.type, &data.value);
		if (valuestr && strcmp(s->value, valuestr) == 0 ) {
			matches++;
		}
		u_free(valuestr);
	}
	if (matches == filter->selectorset.count)
		return 1;

	return 0;
}

static void
instance2xml(CimClientInfo * client,
		CMPIInstance * instance, char *fragstr,
		WsXmlNodeH body, WsEnumerateInfo * enumInfo)
{
	int i = 0;
	char *class_namespace = NULL;
	CMPIObjectPath *objectpath;
	CMPIString *classname;
	CMPIConstClass *_class = NULL;
	char *final_class = NULL;
	int numproperties = 0;
	int frag_type = 0;
	int index;
	char *propertystr = NULL;
	WsXmlNodeH xmlr = NULL;

	objectpath = instance->ft->getObjectPath(instance, NULL);
	classname = objectpath->ft->getClassName(objectpath, NULL);
	class_namespace = cim_find_namespace_for_class(client, enumInfo,
			(char *) classname->hdl);

	final_class = u_strdup(strrchr(class_namespace, '/') + 1);

	if(fragstr) {
		xmlr = body;
	}
	else {
		xmlr = ws_xml_add_child(body, class_namespace, final_class, NULL);
	}

	wsman_get_fragment_type(fragstr, &frag_type, &propertystr, &index);

	if (strcmp(client->requested_class, "*")  && enumInfo &&
			(enumInfo->flags & WSMAN_ENUMINFO_POLY_EXCLUDE )) {
		_class = cim_get_class(client, client->requested_class, 0, NULL);
		if (_class)
			numproperties = _class->ft->getPropertyCount(_class, NULL);
	} else {
		numproperties = instance->ft->getPropertyCount(instance, NULL);
	}


	for (i = 0; i < numproperties; i++) {
		CMPIString *propertyname;
		CMPIData data;
		CMPIStatus is_key;
		if (strcmp(client->requested_class, "*")
				&& enumInfo && (enumInfo->flags & WSMAN_ENUMINFO_POLY_EXCLUDE)) {
			_class->ft->getPropertyAt(_class, i, &propertyname,
					NULL);
			data = instance->ft->getProperty(instance, (char *) propertyname->hdl,
					NULL);
		} else {
			data = instance->ft->getPropertyAt(instance, i,
					&propertyname,
					NULL);
		}
		if(propertystr && strcmp(propertystr, propertyname->hdl)) {
			CMRelease(propertyname);
			continue;
		}
		objectpath->ft->getKey(objectpath, (char *) propertyname->hdl, &is_key);
		property2xml(client, data, (char *) propertyname->hdl, xmlr,
				class_namespace, frag_type, is_key.rc);
		CMRelease(propertyname);
	}
	//ws_xml_dump_node_tree(stdout, r );
	struct timeval tv0, tv1;
	long long t0, t1;
	long long ttime = 0;
	gettimeofday(&tv0, NULL);

	gettimeofday(&tv1, NULL);
	t0 = tv0.tv_sec * 10000000 + tv0.tv_usec;
	t1 = tv1.tv_sec * 10000000 + tv1.tv_usec;
	ttime += t1 -t0;

	if (enumInfo && (enumInfo->flags &  WSMAN_ENUMINFO_POLY_EXCLUDE ) ) {
		if (_class) {
			CMRelease(_class);
		}
	}
	if(propertystr)
		u_free(propertystr);
	if (classname)
		CMRelease(classname);
	if (objectpath)
		CMRelease(objectpath);
	if (final_class)
		u_free(final_class);
	if (class_namespace)
		u_free(class_namespace);
}




static CMPIObjectPath *cim_get_op_from_enum(CimClientInfo * client,
		WsmanStatus * statusP)
{
	CMPIStatus rc;
	int match = 0;
	CMPIEnumeration *enumeration;
	CMPIObjectPath *result_op = NULL;
	WsmanStatus statusPP;
	CMPIArray *enumArr = NULL;

	if (client->requested_class)
		debug("class available");

	CMPIObjectPath *objectpath =
		newCMPIObjectPath(client->cim_namespace,
				client->requested_class, NULL);
	enumeration =
		((CMCIClient *) client->cc)->ft->enumInstanceNames(client->cc,
			objectpath,
			&rc);
	debug("enumInstanceNames rc=%d, msg=%s", rc.rc,
			(rc.msg) ? (char *) rc.msg->hdl : NULL);

	if (rc.rc != 0) {
		cim_to_wsman_status(rc, statusP);
		//statusP->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		if (rc.msg)
			CMRelease(rc.msg);
		goto cleanup;
	}

	enumArr = enumeration->ft->toArray(enumeration, NULL);
	int n = CMGetArrayCount(enumArr, NULL);
	wsman_status_init(&statusPP);
	if (n > 0) {
		while (enumeration->ft->hasNext(enumeration, NULL)) {
			CMPIData data = enumeration->ft->getNext(enumeration,
					NULL);
			CMPIObjectPath *op = CMClone(data.value.ref, NULL);
			CMPIString *opstr = CMObjectPathToString(op, NULL);
			debug("objectpath: %s", (char *) opstr->hdl);
			if (cim_verify_keys(op, client->selectors, &statusPP) != 0) {
				if (opstr)
					CMRelease(opstr);
				if (op)
					CMRelease(op);
				continue;
			} else {
				result_op = CMClone(data.value.ref, NULL);
				CMSetNameSpace(result_op,
						client->cim_namespace);
				match = 1;
				if (opstr)
					CMRelease(opstr);
				if (op)
					CMRelease(op);
				break;
			}
			if (opstr)
				CMRelease(opstr);
			if (op)
				CMRelease(op);
		}
		statusP->fault_code = statusPP.fault_code;
		statusP->fault_detail_code = statusPP.fault_detail_code;
	} else {
		statusP->fault_code = WSA_DESTINATION_UNREACHABLE;
		statusP->fault_detail_code =
			WSMAN_DETAIL_INVALID_RESOURCEURI;
	}
	debug("fault: %d %d", statusP->fault_code, statusP->fault_detail_code);

cleanup:

	if (objectpath)
		CMRelease(objectpath);
	if (enumeration)
		CMRelease(enumeration);
	if (match)
		return result_op;
	else
		return NULL;
}



void
cim_enum_instances(CimClientInfo * client,
		WsEnumerateInfo * enumInfo,
		WsmanStatus * status)
{
	CMPIObjectPath *objectpath = NULL;
	CMPIEnumeration *enumeration = NULL;
	CMPIStatus rc;
	CMCIClient *cc = (CMCIClient *) client->cc;
	sfcc_enumcontext *enumcontext;
	filter_t *filter = NULL;
	filter = enumInfo->filter;

	if( (enumInfo->flags & WSMAN_ENUMINFO_REF) ||
			(enumInfo->flags & WSMAN_ENUMINFO_ASSOC )) {
		char *class = NULL;
		epr_t *epr;
		if (filter) {
			epr = (epr_t *)filter->epr;
			class = strrchr(epr->refparams.uri, '/') + 1;
			objectpath = newCMPIObjectPath(client->cim_namespace,
					class, NULL);
			wsman_epr_selector_cb(filter->epr,
					cim_add_keys_from_filter_cb, objectpath);
			debug( "ObjectPath: %s",
					CMGetCharPtr(CMObjectPathToString(objectpath, &rc)));
		} else {
			status->fault_code = WXF_INVALID_REPRESENTATION;
			status->fault_detail_code = WSMAN_DETAIL_OK;
			goto cleanup;
		}
		/*
	} else if(enumInfo->flags & WSMAN_ENUMINFO_SELECTOR) {
		char *cim_namespace;
		cim_namespace = get_cimnamespace_from_selectorset(&filter->selectorset);
		if(cim_namespace == NULL)
			cim_namespace = client->cim_namespace;
		objectpath = newCMPIObjectPath(cim_namespace, client->requested_class, NULL);
		wsman_selectorset_cb(&filter->selectorset,
				cim_add_keys_from_filter_cb, objectpath);
		debug( "ObjectPath: %s",
				CMGetCharPtr(CMObjectPathToString(objectpath, &rc)));*/
	} else {
		objectpath = newCMPIObjectPath(client->cim_namespace,
				client->requested_class, NULL);
	}

	if (enumInfo->flags & WSMAN_ENUMINFO_REF) {
		enumeration = cc->ft->references(cc, objectpath, filter->resultClass,
				filter->role, 0, NULL, &rc);
	} else if (enumInfo->flags & WSMAN_ENUMINFO_ASSOC) {
		enumeration = cc->ft->associators(cc, objectpath, filter->assocClass,
				filter->resultClass,
				filter->role,
				filter->resultRole, 0, NULL, &rc);
	} else if (( enumInfo->flags & WSMAN_ENUMINFO_WQL )) {
		enumeration = cc->ft->execQuery(cc, objectpath, filter->query, "WQL", &rc);
	} else if (( enumInfo->flags & WSMAN_ENUMINFO_CQL )) {
		enumeration = cc->ft->execQuery(cc, objectpath, filter->query, "CQL", &rc);
	} else {
		enumeration = cc->ft->enumInstances(cc, objectpath,
				CMPI_FLAG_DeepInheritance,
				NULL, &rc);
	}

	debug("enumInstances() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);

	if (rc.rc) {
		debug("CMCIClient enumInstances() failed");
		cim_to_wsman_status(rc, status);
		if (rc.msg)
			CMRelease(rc.msg);
		if (objectpath)
			CMRelease(objectpath);
		goto cleanup;
	}
	CMPIArray *enumArr = enumeration->ft->toArray(enumeration, NULL);
	CMPIArray *fenumArr = NULL;
	if (enumInfo->flags & WSMAN_ENUMINFO_SELECTOR) {
		CMPIType t = enumArr->ft->getSimpleType(enumArr, NULL);
		fenumArr = newCMPIArray(0, t , NULL);
		int idx = 0;
		int fidx = 0;
		for (idx = 0; idx<enumArr->ft->getSize(enumArr, NULL); idx++) {
			CMPIData d = enumArr->ft->getElementAt(enumArr, idx, NULL);
			if (filter_instance(d.value.inst, enumInfo)) {
				fenumArr->ft->setElementAt(fenumArr, fidx, &d.value, d.type);
			}
		}
	} else {
		fenumArr = enumArr;
	}

	cim_to_wsman_status(rc, status);
	if (rc.msg)
		CMRelease(rc.msg);
	if (!enumArr) {
		goto cleanup;
	}

	enumInfo->totalItems = cim_enum_totalItems(fenumArr);
	debug("Total items: %d", enumInfo->totalItems);
	enumcontext = u_zalloc(sizeof(sfcc_enumcontext));
	enumcontext->ecClient = client;
	enumcontext->ecEnumeration = enumeration;
	enumInfo->enumResults = fenumArr;
	enumInfo->appEnumContext = enumcontext;

	if (objectpath)
		CMRelease(objectpath);
cleanup:
	return;
}

int
cim_getElementAt(CimClientInfo * client,
		WsEnumerateInfo * enumInfo, WsXmlNodeH itemsNode)
{
	int retval = 1;
	char *fragstr = NULL;

	CMPIArray *results = (CMPIArray *) enumInfo->enumResults;
	CMPIData data = results->ft->getElementAt(results,
			enumInfo->index, NULL);

	CMPIInstance *instance = data.value.inst;
	CMPIObjectPath *objectpath = instance->ft->getObjectPath(instance, NULL);
	CMPIString *classname = objectpath->ft->getClassName(objectpath, NULL);

	if (enumInfo && (enumInfo->flags & WSMAN_ENUMINFO_POLY_NONE ) &&
			(strcmp((char *) classname->hdl, client->requested_class) != 0)) {
		retval = 0;
	}

	fragstr = wsman_get_fragment_string(client->cntx, client->cntx->indoc);
	if(fragstr) {
		itemsNode = ws_xml_add_child(itemsNode, XML_NS_WS_MAN, WSM_XML_FRAGMENT,
				NULL);
	}
	if (retval)
		instance2xml(client, instance, fragstr, itemsNode, enumInfo);
	if (classname)
		CMRelease(classname);
	if (objectpath)
		CMRelease(objectpath);
	return retval;
}





int
cim_getEprAt(CimClientInfo * client,
		WsEnumerateInfo * enumInfo, WsXmlNodeH itemsNode)
{

	int retval = 1;
	char *uri = NULL;
	CMPIArray *results = (CMPIArray *) enumInfo->enumResults;
	CMPIData data = results->ft->getElementAt(results,
			enumInfo->index, NULL);

	CMPIInstance *instance = data.value.inst;
	CMPIObjectPath *objectpath = instance->ft->getObjectPath(instance, NULL);
	CMPIString *classname = objectpath->ft->getClassName(objectpath, NULL);

	if (enumInfo && (enumInfo->flags & WSMAN_ENUMINFO_POLY_NONE)
			&& (strcmp((char *) classname->hdl, client->requested_class) != 0)) {
		retval = 0;
	}
	uri = cim_find_namespace_for_class(client, enumInfo,
			(char *) classname->hdl);
	if (retval) {
		cim_add_epr(client, itemsNode, uri, objectpath);
	}

	u_free(uri);
	if (classname)
		CMRelease(classname);
	if (objectpath)
		CMRelease(objectpath);
	return retval;
}

int
cim_getEprObjAt(CimClientInfo * client,
		WsEnumerateInfo * enumInfo, WsXmlNodeH itemsNode)
{
	int retval = 1;
	char *uri = NULL;
	CMPIArray *results = (CMPIArray *) enumInfo->enumResults;
	CMPIData data =
		results->ft->getElementAt(results, enumInfo->index, NULL);

	CMPIInstance *instance = data.value.inst;
	CMPIObjectPath *objectpath =
		instance->ft->getObjectPath(instance, NULL);
	CMPIString *classname =
		objectpath->ft->getClassName(objectpath, NULL);

	if (enumInfo && (enumInfo->flags & WSMAN_ENUMINFO_POLY_NONE) &&
			(strcmp((char *) classname->hdl, client->requested_class) != 0)) {
		retval = 0;
	}
	uri = cim_find_namespace_for_class(client, enumInfo,
			(char *) classname->hdl);

	if (retval) {
		WsXmlNodeH item =
			ws_xml_add_child(itemsNode, XML_NS_WS_MAN, WSM_ITEM,
					NULL);
		instance2xml(client, instance, NULL, item, enumInfo);
		cim_add_epr(client, item, uri, objectpath);
	}
	u_free(uri);
	if (classname)
		CMRelease(classname);
	if (objectpath)
		CMRelease(objectpath);
	return retval;
}


void
create_instance_from_xml(CMPIInstance * instance,
		CMPIConstClass * class,
		WsXmlNodeH resource, char *fragstr,
		char *resource_uri, WsmanStatus * status)
{
	int i;
	int fragment_flag;
	char *element;
	int index;
	WsXmlNodeH child = NULL;
	char *value = NULL;
	CMPIData data;
	CMPIObjectPath *objectpath =
		instance->ft->getObjectPath(instance, NULL);
	CMPIString *classname =
		objectpath->ft->getClassName(objectpath, NULL);

	int numproperties = 0;
	numproperties = class->ft->getPropertyCount(class, NULL);

	wsman_get_fragment_type(fragstr, &fragment_flag, &element, &index);
	debug("num props=%d", numproperties);
	for (i = 0; i < numproperties; i++) {
		CMPIString *propertyname;
		data = class->ft->getPropertyAt(class,
				i, &propertyname,
				NULL);
		debug("working on property: %s", (char *) propertyname->hdl );
		if(fragment_flag == 0) {
			child = ws_xml_get_child(resource, 0, resource_uri,
					(char *) propertyname->hdl);
			if (child) {
				value = ws_xml_get_node_text(child);

				WsXmlAttrH attr =
					ws_xml_find_node_attr(child,
							XML_NS_SCHEMA_INSTANCE,
							XML_NS_SCHEMA_INSTANCE_NIL);
				char *attr_val = ws_xml_get_attr_value(attr);
				if (attr && attr_val && (strcmp(attr_val, "true") == 0)) {
					continue;
				}
				debug("prop value: %s", value );
				if (value) {
					xml2property(instance, data,
							(char *) propertyname->hdl,
							value);
				}
			} else if (data.type != CMPI_null
					&& data.state != CMPI_nullValue) {
				status->fault_code = WXF_INVALID_REPRESENTATION;
				status->fault_detail_code =
					WSMAN_DETAIL_MISSING_VALUES;
				CMRelease(propertyname);
				break;
			} else {
				warning("cannot handle property");
			}
		}
		else {
			if(strcmp(element, (char *) propertyname->hdl ) == 0) {
				CMRelease(propertyname);
				break;
			}
		}
		CMRelease(propertyname);
	}

	if(fragment_flag) {
		child = ws_xml_get_child(resource, 0, XML_NS_WS_MAN, WSM_XML_FRAGMENT);
		if(i == numproperties) {
			status->fault_code = WSMAN_INVALID_SELECTORS;
			status->fault_detail_code = WSMAN_DETAIL_UNEXPECTED_SELECTORS;
		}
		else {
			if(fragment_flag == 1) {
				child = ws_xml_get_child(resource, 0, NULL, element);
				if(child == NULL) {
					status->fault_code = WSMAN_INVALID_SELECTORS;
					status->fault_detail_code = WSMAN_DETAIL_UNEXPECTED_SELECTORS;
					goto cleanup;
				}
				value = ws_xml_get_node_text(child);
			}
			else
				value = ws_xml_get_node_text(resource);
			debug("prop value: %s", value );
			if (value) {
				xml2property(instance, data,
						element, value);
			}
		}
	}
cleanup:
	if (classname)
		CMRelease(classname);
	if (objectpath)
		CMRelease(objectpath);
	if (element)
		u_free(element);

	return;
}



void
xml2instance(CMPIInstance * instance, WsXmlNodeH body, char *resourceUri)
{
	int i, numproperties;
	CMPIString *namespace, *classname;
	CMPIObjectPath *objectpath = NULL;
	WsXmlNodeH r;

	objectpath = instance->ft->getObjectPath(instance, NULL);
	namespace = objectpath->ft->getNameSpace(objectpath, NULL);
	classname =objectpath->ft->getClassName(objectpath, NULL);

	numproperties = instance->ft->getPropertyCount(instance, NULL);
	r = ws_xml_get_child(body, 0, resourceUri,(char *) classname->hdl);

	if (numproperties) {
		for (i = 0; i < numproperties; i++) {
			CMPIString *propertyname;
			CMPIData data = instance->ft->getPropertyAt(instance,
					i, &propertyname,
					NULL);
			WsXmlNodeH child = ws_xml_get_child(r, 0,
					resourceUri,
					(char *)
					propertyname->
					hdl);

			char *value = ws_xml_get_node_text(child);
			if (value) {
				xml2property(instance, data,
						(char *) propertyname->hdl,
						value);
			}
			CMRelease(propertyname);
		}
	}

	if (classname)
		CMRelease(classname);
	if (namespace)
		CMRelease(namespace);
	if (objectpath)
		CMRelease(objectpath);
}





void
cim_add_epr_details(CimClientInfo * client,
		WsXmlNodeH resource,
		char *resource_uri, CMPIObjectPath * objectpath)
{
	int numkeys = 0, i = 0;
	char *cv = NULL;

	ws_xml_add_child(resource, XML_NS_ADDRESSING, WSA_ADDRESS,
			WSA_TO_ANONYMOUS);
	numkeys = objectpath->ft->getKeyCount(objectpath, NULL);

	WsXmlNodeH refparam = ws_xml_add_child(resource,
			XML_NS_ADDRESSING,
			WSA_REFERENCE_PARAMETERS,
			NULL);
	ws_xml_add_child_format(refparam, XML_NS_WS_MAN, WSM_RESOURCE_URI,
			"%s", resource_uri);
	WsXmlNodeH wsman_selector_set = ws_xml_add_child(refparam,
			XML_NS_WS_MAN,
			WSM_SELECTOR_SET,
			NULL);

	for (i = 0; i < numkeys; i++) {
		CMPIString *keyname;
		CMPIData data =
			objectpath->ft->getKeyAt(objectpath, i, &keyname,
					NULL);
		WsXmlNodeH s = NULL;
		if (data.type == CMPI_ref) {
			s = ws_xml_add_child(wsman_selector_set,
					XML_NS_WS_MAN, WSM_SELECTOR,
					NULL);
			WsXmlNodeH epr = ws_xml_add_child(s,
					XML_NS_ADDRESSING,
					WSA_EPR, NULL);
			path2xml(client, epr, resource_uri, &data.value);
		} else {
			char *valuestr =
				value2Chars(data.type, &data.value);
			s = ws_xml_add_child(wsman_selector_set,
					XML_NS_WS_MAN, WSM_SELECTOR,
					valuestr);
			if (valuestr)
				free(valuestr);
		}
		ws_xml_add_node_attr(s, NULL, WSM_NAME,
				(char *) keyname->hdl);


		if (cv)
			free(cv);
		if (keyname)
			CMRelease(keyname);
	}
	return;
}

void
cim_add_epr(CimClientInfo * client,
		WsXmlNodeH resource,
		char *resource_uri, CMPIObjectPath * objectpath)
{
	WsXmlNodeH epr = ws_xml_add_child(resource,
			XML_NS_ADDRESSING, WSA_EPR,
			NULL);
	cim_add_epr_details(client, epr, resource_uri, objectpath);
	return;
}

CMCIClient *
cim_connect_to_cimom(char *cim_host,
		char *cim_port,
		char *cim_host_userid,
		char *cim_host_passwd,
		char *frontend,
		WsmanStatus * status)
{
	CMPIStatus rc;
	if (strcmp(frontend, "SfcbLocal") != 0)
		frontend = "http";

	CMCIClient *cimclient = cmciConnect(cim_host, frontend , cim_port,
			cim_host_userid,
			cim_host_passwd, &rc);

	if (cimclient == NULL) {
		debug( "Connection to CIMOM failed");
	} else {
		debug("new cimclient: 0x%8x", cimclient);
		debug("new cimclient: %d", cimclient->ft->ftVersion);
	}
	cim_to_wsman_status(rc, status);
	return cimclient;
}

void cim_release_client(CimClientInfo * cimclient)
{
	if (cimclient->cc) {
		CMRelease((CMCIClient *) cimclient->cc);
	}
}




void
cim_invoke_method(CimClientInfo * client,
		WsContextH cntx, WsXmlNodeH body, WsmanStatus * status)
{
	CMPIObjectPath *objectpath;
	CMPIStatus rc;
	WsmanStatus statusP;
	CMCIClient *cc = (CMCIClient *) client->cc;
	WsXmlNodeH method_node = NULL;

	wsman_status_init(&statusP);
	if (client->resource_uri &&
			strstr(client->resource_uri, XML_NS_CIM_CLASS) != NULL) {
		objectpath = cim_get_op_from_enum(client, &statusP);
	} else {
		debug("no base class, getting instance directly with getInstance");
		objectpath = newCMPIObjectPath(client->cim_namespace,
				client->requested_class, NULL);
		if (objectpath != NULL)
			cim_add_keys(objectpath, client->selectors);
	}

	if (objectpath != NULL) {
		CMPIArgs *argsin = NULL, *argsout = NULL;
		argsin = newCMPIArgs(NULL);

		if (client->method_args && hash_count(client->method_args) > 0) {
			debug("adding method arguments");
			cim_add_args(client, objectpath, argsin);
		}
		argsout = newCMPIArgs(NULL);
		CMPIData data = cc->ft->invokeMethod(cc, objectpath,
				client->method,
				argsin, argsout, &rc);

		debug("invokeMethod() rc=%d, msg=%s",
				rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);

		method_node = ws_xml_add_empty_child_format(body,
				client->resource_uri,
				"%s_OUTPUT",
				client->method);

		if (rc.rc == 0) {
			property2xml(client, data, "ReturnValue",
					method_node, client->resource_uri, 0, 1);
		}

		if (argsout) {
			int count = CMGetArgCount(argsout, NULL);
			int i = 0;
			for (i = 0; i < count; i++) {
				CMPIString *argname;
				CMPIData data =
					CMGetArgAt(argsout, i, &argname, NULL);
				property2xml(client, data,
						(char *) argname->hdl,
						method_node, client->resource_uri, 0, 0);
				CMRelease(argname);
			}
		}

		cim_to_wsman_status(rc, status);
		if (rc.msg)
			CMRelease(rc.msg);
		if (argsin)
			CMRelease(argsin);
		if (argsout)
			CMRelease(argsout);
	} else {
		status->fault_code = statusP.fault_code;
		status->fault_detail_code = statusP.fault_detail_code;
	}

	if (objectpath)
		CMRelease(objectpath);

	return;
}





void
cim_delete_instance_from_enum(CimClientInfo * client, WsmanStatus * status)
{
	CMPIObjectPath *objectpath = NULL;
	CMPIStatus rc;
	WsmanStatus statusP;
	CMCIClient *cc = (CMCIClient *) client->cc;

	if (!cc) {
		goto cleanup;
	}
	wsman_status_init(&statusP);

	if ((objectpath = cim_get_op_from_enum(client, &statusP)) != NULL) {
		rc = cc->ft->deleteInstance(cc, objectpath);
		if (rc.rc != 0) {
			cim_to_wsman_status(rc, status);
		}
		debug("deleteInstance rc=%d, msg=%s", rc.rc,
				(rc.msg) ? (char *) rc.msg->hdl : NULL);
	} else {
		status->fault_code = statusP.fault_code;
		status->fault_detail_code = statusP.fault_detail_code;
	}

	debug("fault: %d %d", status->fault_code,
			status->fault_detail_code);

cleanup:
	if (objectpath)
		CMRelease(objectpath);
	return;
}



void
cim_get_instance_from_enum(CimClientInfo * client,
		WsContextH cntx,
		WsXmlNodeH body, char *fragstr, WsmanStatus * status)
{
	CMPIInstance *instance;
	CMPIObjectPath *objectpath;
	CMPIStatus rc;
	WsmanStatus statusP;
	CMCIClient *cc = (CMCIClient *) client->cc;

	if (!cc) {
		goto cleanup;
	}
	wsman_status_init(&statusP);

	if ((objectpath = cim_get_op_from_enum(client, &statusP)) != NULL) {
		instance = cc->ft->getInstance(cc, objectpath,
				CMPI_FLAG_IncludeClassOrigin,
				NULL, &rc);
		if (rc.rc == 0) {
			if (instance) {
				instance2xml(client, instance, fragstr, body, NULL);
			}
		} else {
			cim_to_wsman_status(rc, status);
		}
		debug("getInstance rc=%d, msg=%s", rc.rc,
				(rc.msg) ? (char *) rc.msg->hdl : NULL);
		if (instance)
			CMRelease(instance);
	} else {
		status->fault_code = statusP.fault_code;
		status->fault_detail_code = statusP.fault_detail_code;
	}

	debug("fault: %d %d", status->fault_code,
			status->fault_detail_code);

	if (objectpath)
		CMRelease(objectpath);
cleanup:
	return;
}


void
cim_put_instance(CimClientInfo * client,
		WsContextH cntx,
		WsXmlNodeH in_body, WsXmlNodeH body, char *fragstr, WsmanStatus * status)
{
	CMPIInstance *instance = NULL;
	CMPIObjectPath *objectpath;
	CMPIStatus rc;
	WsmanStatus statusP;
	WsXmlNodeH resource = NULL;
	CMCIClient *cc = (CMCIClient *) client->cc;
	CMPIConstClass *class = NULL;

	wsman_status_init(&statusP);
	objectpath = newCMPIObjectPath(client->cim_namespace,
			client->requested_class, NULL);
	if(fragstr == NULL) {
		resource = ws_xml_get_child(in_body, 0, client->resource_uri,
				client->requested_class);
	}
	else {
		resource = ws_xml_get_child(in_body, 0, XML_NS_WS_MAN, WSM_XML_FRAGMENT);
	}
	if (!resource) {
		status->fault_code = WXF_INVALID_REPRESENTATION;
		status->fault_detail_code = WSMAN_DETAIL_INVALID_NAMESPACE;
		goto cleanup;
	}

	if (objectpath != NULL) {
		cim_add_keys(objectpath, client->selectors);
		if (!objectpath) {
			goto cleanup;
		}
	}

	instance = newCMPIInstance(objectpath, NULL);
	if (!instance)
		goto cleanup;

	class = cim_get_class(client, client->requested_class,
			CMPI_FLAG_IncludeQualifiers,
			status);
	if (class ) {
		create_instance_from_xml(instance, class, resource,
				fragstr, client->resource_uri, status);
		CMRelease(class);
	}
	if (status->fault_code == 0 && instance ) {
		CMPIString *opstr = CMObjectPathToString(objectpath, NULL);
		debug("objectpath: %s", (char *)opstr->hdl );
		rc = cc->ft->setInstance(cc, objectpath, instance, 0, NULL);
		debug("modifyInstance() rc=%d, msg=%s", rc.rc,
				(rc.msg) ? (char *) rc.msg-> hdl : NULL);
		if (rc.rc == CMPI_RC_ERR_FAILED) {
			status->fault_code =
				WSA_ACTION_NOT_SUPPORTED;
		} else {
			cim_to_wsman_status(rc, status);
		}
		if (rc.rc == 0) {
			if (instance)
				instance2xml(client, instance, fragstr, body, NULL);
		}
		if (rc.msg)
			CMRelease(rc.msg);
	}
cleanup:
	if (objectpath)
		CMRelease(objectpath);
	if (instance)
		CMRelease(instance);
	return;
}


void
cim_create_instance(CimClientInfo * client,
		WsContextH cntx,
		WsXmlNodeH in_body,
		WsXmlNodeH body, char *fragstr, WsmanStatus * status)
{

	CMPIInstance *instance = NULL;
	CMPIObjectPath *objectpath, *objectpath_r;
	CMPIStatus rc;
	WsmanStatus statusP;
	CMPIConstClass *class;
	int fragment_flag;
	char *element;
	int index;
	WsXmlNodeH resource;
	WsXmlNodeH child = NULL;
	wsman_status_init(&statusP);
	int numproperties = 0, i;

	CMCIClient *cc = (CMCIClient *) client->cc;
	objectpath = newCMPIObjectPath(client->cim_namespace,
			client->requested_class, NULL);
	class = cim_get_class(client, client->requested_class,
			CMPI_FLAG_IncludeQualifiers, status);
	if (class) {
		numproperties = class->ft->getPropertyCount(class, NULL);
	}

	if(fragstr == NULL) {
		resource = ws_xml_get_child(in_body, 0, client->resource_uri,
				client->requested_class);
	}
	else {
		resource = ws_xml_get_child(in_body, 0, XML_NS_WS_MAN, WSM_XML_FRAGMENT);
	}
	if (!resource) {
		status->fault_code = WXF_INVALID_REPRESENTATION;
		status->fault_detail_code = WSMAN_DETAIL_INVALID_NAMESPACE;
		goto cleanup;
	}
	wsman_get_fragment_type(fragstr, &fragment_flag, &element, &index);
	/*
	 * Add keys
	 */
	for (i = 0; i < numproperties; i++) {
		CMPIString *propertyname;
		class->ft->getPropertyAt(class, i, &propertyname, NULL);
		class->ft->getPropertyQualifier(class,
				(char *) propertyname->hdl,
				"KEY", &rc);
		debug("working on property: %s", (char *) propertyname->hdl );
		if (rc.rc == 0 && fragstr == NULL &&
				!ws_xml_get_child(resource, 0,
					client->resource_uri,
					(char *) propertyname->
					hdl)) {
			status->fault_code = WXF_INVALID_REPRESENTATION;
			status->fault_detail_code =
				WSMAN_DETAIL_MISSING_VALUES;
			break;
		} else if (rc.rc == 0) {
			if(fragstr == NULL) {
				child = ws_xml_get_child(resource, 0,
						client->resource_uri, (char *)propertyname->hdl);
			}
			else {
				if(strcmp(element, (char *)propertyname->hdl)) {
					CMRelease(propertyname);
					continue;
				}
				if(fragment_flag == 1 || fragment_flag == 3)
					child = ws_xml_get_child(resource, 0, NULL, element);
			}

			char *value = ws_xml_get_node_text(child);
			CMAddKey(objectpath, (char *) propertyname->hdl,
					value, CMPI_chars);
			if(fragstr && strcmp(element, (char *)propertyname->hdl) == 0) {
				CMRelease(propertyname);
				break;
			}
		}
		CMRelease(propertyname);
	}

	if(fragstr && i == numproperties) {
		status->fault_code = WSMAN_INVALID_SELECTORS;
		status->fault_detail_code = WSMAN_DETAIL_UNEXPECTED_SELECTORS;
		goto cleanup;
	}

	instance = newCMPIInstance(objectpath, NULL);
	if (status->fault_code != 0)
		goto cleanup;


	create_instance_from_xml(instance, class,
			resource, fragstr, client->resource_uri, status);
	if (status->fault_code == 0) {
		objectpath_r = cc->ft->createInstance(cc, objectpath, instance, &rc);
		debug("createInstance() rc=%d, msg=%s", rc.rc,
				(rc.msg) ? (char *) rc.msg->hdl : NULL);
		if (objectpath_r) {
			WsXmlNodeH epr = ws_xml_add_child(body, XML_NS_TRANSFER,
					WXF_RESOURCE_CREATED,
					NULL);
			cim_add_epr_details(client, epr, client->resource_uri,
					objectpath_r);
		}
		if (rc.rc == CMPI_RC_ERR_FAILED) {
			status->fault_code = WSA_ACTION_NOT_SUPPORTED;
		} else {
			cim_to_wsman_status(rc, status);
		}
		if (rc.msg)
			CMRelease(rc.msg);
	}
cleanup:
	if (class)
		CMRelease(class);
	if (instance)
		CMRelease(instance);
	if (objectpath)
		CMRelease(objectpath);
	if (element)
		u_free(element);
	return;
}

void cim_delete_instance(CimClientInfo * client, WsmanStatus * status)
{
	CMPIObjectPath *objectpath;
	CMPIStatus rc;

	CMCIClient *cc = (CMCIClient *) client->cc;
	CMPIConstClass *class =
		cim_get_class(client, client->requested_class,
				CMPI_FLAG_IncludeQualifiers,
				status);
	if (!class)
		goto cleanup;

	cim_verify_class_keys(class, client->selectors, status);
	if (status->fault_code != 0)
		goto cleanup;
	objectpath = newCMPIObjectPath(client->cim_namespace,
			client->requested_class, NULL);

	cim_add_keys(objectpath, client->selectors);
	rc = cc->ft->deleteInstance(cc, objectpath);
	/* Print the results */
	debug("deleteInstance() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	cim_to_wsman_status(rc, status);
	if (rc.msg)
		CMRelease(rc.msg);
	if (objectpath)
		CMRelease(objectpath);
cleanup:
	return;

}


CMPIInstance *
cim_get_instance_from_selectors(CimClientInfo * client,
		WsContextH cntx, WsmanStatus * status)
{
	CMPIInstance *instance = NULL;
	CMPIObjectPath *objectpath = NULL;
	CMPIStatus rc;

	CMCIClient *cc = (CMCIClient *) client->cc;

	CMPIConstClass *class = cim_get_class(client,
			client->requested_class,
			CMPI_FLAG_IncludeQualifiers,
			status);
	if (!class)
		goto cleanup;

	cim_verify_class_keys(class, client->selectors, status);
	if (status->fault_code != 0)
		goto cleanup;

	objectpath = newCMPIObjectPath(client->cim_namespace,
			client->requested_class, NULL);

	cim_add_keys(objectpath, client->selectors);
	instance = cc->ft->getInstance(cc, objectpath,
			CMPI_FLAG_DeepInheritance, NULL,
			&rc);
	/* Print the results */
	debug("getInstance() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	cim_to_wsman_status(rc, status);
	if (rc.msg)
		CMRelease(rc.msg);
cleanup:
	if (objectpath)
		CMRelease(objectpath);
	if (class)
		CMRelease(class);
	return instance;

}

CMPIObjectPath *
cim_get_objectpath_from_selectors(CimClientInfo * client,
		WsContextH cntx, WsmanStatus * status)
{
	CMPIObjectPath *objectpath = NULL;
	CMPIConstClass *class = cim_get_class(client,
			client->requested_class,
			CMPI_FLAG_IncludeQualifiers,
			status);
	if (!class)
		goto cleanup;

	cim_verify_class_keys(class, client->selectors, status);
	if (status->fault_code != 0)
		goto cleanup;

	objectpath = newCMPIObjectPath(client->cim_namespace,
			client->requested_class, NULL);

	cim_add_keys(objectpath, client->selectors);
cleanup:
	if (class)
		CMRelease(class);
	return objectpath;

}

void
cim_get_instance(CimClientInfo * client,
		WsContextH cntx, WsXmlNodeH body, char *fragstr, WsmanStatus * status)
{
	CMPIInstance *instance = cim_get_instance_from_selectors(client, cntx, status);
	if(instance) {
		instance2xml(client, instance, fragstr, body, NULL);
		CMRelease(instance);
	}
}

static CMPIObjectPath *cim_indication_filter_objectpath(CimClientInfo *client,
		WsSubscribeInfo *subsInfo, CMPIStatus *rc)
{
	CMPIObjectPath *objectpath_filter = newCMPIObjectPath(subsInfo->cim_namespace,
			"CIM_IndicationFilter", rc);
	CMAddKey(objectpath_filter, "SystemCreationClassName",
			SYSTEMCREATIONCLASSNAME, CMPI_chars);
	CMAddKey(objectpath_filter, "SystemName",
			SYSTEMNAME, CMPI_chars);
	CMAddKey(objectpath_filter, "CreationClassName",
			"CIM_IndicationFilter", CMPI_chars);
	CMAddKey(objectpath_filter, "Name",
			subsInfo->subsId, CMPI_chars);
	return objectpath_filter;
}

static CMPIObjectPath
*cim_indication_handler_objectpath(CimClientInfo *client, WsSubscribeInfo *subsInfo, CMPIStatus *rc)
{
	CMPIObjectPath *objectpath_handler = newCMPIObjectPath(subsInfo->cim_namespace,
			"CIM_IndicationHandlerCIMXML", rc);
	CMAddKey(objectpath_handler, "SystemCreationClassName",
			SYSTEMCREATIONCLASSNAME, CMPI_chars);
	CMAddKey(objectpath_handler, "SystemName",
			SYSTEMNAME, CMPI_chars);
	CMAddKey(objectpath_handler, "CreationClassName",
			"CIM_IndicationHandlerCIMXML", CMPI_chars);
	CMAddKey(objectpath_handler, "Name",
			subsInfo->subsId, CMPI_chars);
	return objectpath_handler;
}

CMPIObjectPath *cim_create_indication_filter(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status)
{
	CMPIInstance *instance = NULL;
	CMPIObjectPath *objectpath = NULL;
	CMPIObjectPath *objectpath_r = NULL;
	CMPIObjectPath *filter_op = NULL;
	CMPIStatus rc;

	CMCIClient *cc = (CMCIClient *) client->cc;

	objectpath = cim_indication_filter_objectpath(client, subsInfo, &rc);
	if(rc.rc)
		goto cleanup;
	filter_op = CMClone(objectpath, &rc);
	CMAddKey(objectpath, "Query",
			subsInfo->filter->query, CMPI_chars);
	if(subsInfo->flags & WSMAN_SUBSCRIPTION_WQL)
		CMAddKey(objectpath, "QueryLanguage",
				"WQL",CMPI_chars);
	else if(subsInfo->flags & WSMAN_SUBSCRIPTION_CQL)
		CMAddKey(objectpath, "QueryLanguage",
				"CQL",CMPI_chars);
	char *indicationns = get_cim_indication_SourceNamespace();
	if(indicationns)
		CMAddKey(objectpath, "SourceNamespace",
				indicationns, CMPI_chars);
	instance = newCMPIInstance(objectpath, NULL);
	objectpath_r = cc->ft->createInstance(cc, objectpath, instance, &rc);
cleanup:
	/* Print the results */
	debug("create CIM_IndicationFilter() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	if (rc.rc == CMPI_RC_ERR_FAILED) {
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
	} else if(rc.rc != 11){ // an object already exists. We take this erros as success
		cim_to_wsman_status(rc, status);
	}
	if (rc.msg)
		CMRelease(rc.msg);
	if (objectpath_r)
		CMRelease(objectpath_r);
	if (objectpath)
		CMRelease(objectpath);
	if (instance)
		CMRelease(instance);
	return filter_op;
}
static char * create_cimxml_listener_path(char *uuid)
{
	char path[128];
	snprintf(path, 128, "/cimindicationlistener/%s", uuid);
	return u_strdup(path);
}


CMPIObjectPath *cim_create_indication_handler(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status)
{
	CMPIInstance *instance = NULL;
	CMPIObjectPath *objectpath = NULL;
	CMPIObjectPath *objectpath_r = NULL;
	CMPIObjectPath *handler_op = NULL;
	CMPIStatus rc;

	CMCIClient *cc = (CMCIClient *) client->cc;

	objectpath = cim_indication_handler_objectpath(client, subsInfo, &rc);
	if(rc.rc)
		goto cleanup;
	handler_op = CMClone(objectpath, &rc);

	instance = newCMPIInstance(objectpath, NULL);

	char *servicepath = create_cimxml_listener_path(subsInfo->subsId);
	char serverpath[128];
	snprintf(serverpath, 128, "http://%s:%s@localhost:%s%s", client->username, client->password,
			get_server_port(), servicepath);
	u_free(servicepath);
	CMPIValue value;
	value.uint16 = 2;
	CMSetProperty(instance, "Destination",
			serverpath, CMPI_chars);
	CMSetProperty(instance, "PersistenceType",
			&value, CMPI_uint16);


	objectpath_r = cc->ft->createInstance(cc, objectpath, instance, &rc);
cleanup:
	/* Print the results */
	debug("create CIM_IndicationHandlerCIMXML() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	if (rc.rc == CMPI_RC_ERR_FAILED) {
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
	} else if(rc.rc != 11){ // an object already exists. We take this erros as success
		cim_to_wsman_status(rc, status);
	}
	if (rc.msg)
		CMRelease(rc.msg);
	if (objectpath_r)
		CMRelease(objectpath_r);
	if (objectpath)
		CMRelease(objectpath);
	if (instance)
		CMRelease(instance);
	return handler_op;
}

void cim_create_indication_subscription(CimClientInfo * client, WsSubscribeInfo *subsInfo, CMPIObjectPath *filter, CMPIObjectPath *handler, WsmanStatus *status)
{
	CMPIInstance *instance = NULL;
	CMPIObjectPath *instance_r = NULL;
	CMPIObjectPath *objectpath = NULL;
	CMPIStatus rc;

	CMCIClient *cc = (CMCIClient *) client->cc;
	//	CMPIObjectPath *objectpath_filter = cim_indication_filter_objectpath(client, subsInfo, &rc);
	//	if(rc.rc) goto cleanup;
	//	objectpath_handler = cim_indication_handler_objectpath(client, subsInfo, &rc);
	//	if(rc.rc) goto cleanup;

	objectpath = newCMPIObjectPath(client->cim_namespace,
			"CIM_IndicationSubscription", NULL);
	CMPIValue value;
	value.ref = filter;
	CMAddKey(objectpath, "Filter",
			&value, CMPI_ref);
	value.ref = handler;
	CMAddKey(objectpath, "Handler",
			&value, CMPI_ref);
	//set OnFatalErrorPolicy to "Ignore"
	value.uint16 = 2;
	CMAddKey(objectpath, "OnFatalErrorPolicy",
			&value, CMPI_uint16);
	//enable subscription
	value.uint16 = 2;
	CMAddKey(objectpath, "SubscriptionState",
			&value, CMPI_uint16);
	if(subsInfo->expires) {
		struct timeval  tv;
		gettimeofday(&tv, NULL);
		value.uint64 = subsInfo->expires - tv.tv_sec;
		CMAddKey(objectpath, "subscriptionDuration",
				&value, CMPI_uint64);
	}
	//set RepeatNotificationPolicy to None
	value.uint16 = 2;
	CMAddKey(objectpath, "RepeatNotificationPolicy",
			&value, CMPI_uint16);
	instance = newCMPIInstance(objectpath, NULL);
	instance_r = cc->ft->createInstance(cc, objectpath, instance, &rc);

	/* Print the results */
	debug("create CIM_IndicationSubscription() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	if (rc.rc == CMPI_RC_ERR_FAILED) {
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
	} else if (rc.rc != 11){ // an object already exists. We take this erros as success
		cim_to_wsman_status(rc, status);
	}
	if (rc.msg)
		CMRelease(rc.msg);
	if (objectpath)
		CMRelease(objectpath);
	if (instance)
		CMRelease(instance);
	if (instance_r)
		CMRelease(instance_r);
	return;
}

void cim_update_indication_subscription(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status)
{
	CMPIObjectPath *objectpath = NULL;
	CMPIInstance *instance = NULL;
	CMPIStatus rc;
	CMPIObjectPath *objectpath_handler = NULL;
	CMCIClient *cc = (CMCIClient *) client->cc;

	CMPIObjectPath *objectpath_filter = NULL;
	if(subsInfo->flags & WSMAN_SUBSCRIPTION_SELECTORSET)
		objectpath_filter = subsInfo->existingfilterOP;
	else {
		objectpath_filter = cim_indication_filter_objectpath(client, subsInfo, &rc);
		if(rc.rc)
			goto cleanup;
	}
	objectpath_handler = cim_indication_handler_objectpath(client, subsInfo, &rc);
	if(rc.rc)
		goto cleanup;
	objectpath = newCMPIObjectPath(client->cim_namespace,
			"CIM_IndicationSubscription", NULL);
	CMPIValue value;
	value.ref = objectpath_filter;
	CMAddKey(objectpath, "Filter",
			&value, CMPI_ref);
	value.ref = objectpath_handler;
	CMAddKey(objectpath, "Handler",
			&value, CMPI_ref);
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	value.uint64 = subsInfo->expires - tv.tv_sec;
	instance = newCMPIInstance(objectpath, NULL);
	CMSetProperty(instance, "subscriptionDuration", &value, CMPI_uint64);
	char *properties[] = {"subscriptionDuration",NULL};
	cc->ft->setInstance(cc, objectpath, instance, 0, properties);
cleanup:
	if (rc.rc == CMPI_RC_ERR_FAILED) {
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
	} else {
		cim_to_wsman_status(rc, status);
	}
	debug("cim_update_indication_subscription() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	if (rc.msg)
		CMRelease(rc.msg);
	if (!(subsInfo->flags & WSMAN_SUBSCRIPTION_SELECTORSET) && objectpath_filter)
		CMRelease(objectpath_filter);
	if (objectpath_handler)
		CMRelease(objectpath_handler);
	if (instance)
		CMRelease(instance);
	if (objectpath)
		CMRelease(objectpath);
	return;
}

void cim_delete_indication_subscription(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status)
{
	CMPIStatus rc;
	CMPIObjectPath *objectpath_subscription = NULL;
	CMCIClient *cc = (CMCIClient *) client->cc;
	CMPIObjectPath *objectpath_handler = NULL;
	CMPIObjectPath *objectpath_filter = NULL;
	if(subsInfo->flags & WSMAN_SUBSCRIPTION_SELECTORSET) {
		objectpath_filter = subsInfo->existingfilterOP;
	}
	else{
		objectpath_filter = cim_indication_filter_objectpath(client, subsInfo, &rc);
		if(rc.rc)
			goto cleanup;
	}
	objectpath_handler = cim_indication_handler_objectpath(client, subsInfo, &rc);
	if(rc.rc)
		goto cleanup;
	objectpath_subscription = newCMPIObjectPath(subsInfo->cim_namespace,
			"CIM_IndicationSubscription", &rc);
	if(rc.rc)
		goto cleanup;
	CMPIValue value;
	value.ref = objectpath_filter;
	CMAddKey(objectpath_subscription, "Filter",
			&value, CMPI_ref);
	value.ref = objectpath_handler;
	CMAddKey(objectpath_subscription, "Handler",
			&value, CMPI_ref);
	rc = cc->ft->deleteInstance(cc, objectpath_subscription);
	if(rc.rc)
		goto cleanup;
	if(!(subsInfo->flags & WSMAN_SUBSCRIPTION_SELECTORSET)) {
		rc = cc->ft->deleteInstance(cc, objectpath_filter);
		if(rc.rc)
			goto cleanup;
	}
	rc = cc->ft->deleteInstance(cc, objectpath_handler);

cleanup:
	if (rc.rc == CMPI_RC_ERR_FAILED) {
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
	} else {
		cim_to_wsman_status(rc, status);
	}
	debug("cim_delete_indication_subscription() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);
	if (rc.msg)
		CMRelease(rc.msg);
	if (objectpath_filter)
		CMRelease(objectpath_filter);
	if (objectpath_handler)
		CMRelease(objectpath_handler);
	if (objectpath_subscription)
		CMRelease(objectpath_subscription);
	return;
}

void cim_to_wsman_status(CMPIStatus rc, WsmanStatus * status)
{
	if (!status) {
		return;
	}
	switch (rc.rc) {
	case CMPI_RC_OK:
		status->fault_code = WSMAN_RC_OK;
		break;
	case CMPI_RC_ERR_INVALID_CLASS:
		status->fault_code = WSA_DESTINATION_UNREACHABLE;
		status->fault_detail_code =
			WSMAN_DETAIL_INVALID_RESOURCEURI;
		break;
	case CMPI_RC_ERR_FAILED:
		if (rc.msg && strcmp((char *) rc.msg->hdl, "CURL error: 7") == 0)
			status->fault_code = WSA_DESTINATION_UNREACHABLE;
		else
			status->fault_code = WSMAN_INTERNAL_ERROR;
		break;
	case CMPI_RC_ERR_METHOD_NOT_AVAILABLE:
	case CMPI_RC_ERR_METHOD_NOT_FOUND:
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
		break;
	case CMPI_RC_ERR_INVALID_NAMESPACE:
	case CMPI_RC_ERR_NOT_FOUND:
		status->fault_code = WSA_DESTINATION_UNREACHABLE;
		break;
	case CMPI_RC_ERR_ACCESS_DENIED:
		status->fault_code = WSMAN_ACCESS_DENIED;
		break;
	case CMPI_RC_ERR_INVALID_PARAMETER:
		status->fault_code = WSMAN_INVALID_PARAMETER;
		status->fault_detail_code = WSMAN_DETAIL_MISSING_VALUES;
		break;
	case CMPI_RC_ERR_NOT_SUPPORTED:
		status->fault_code = WSA_ACTION_NOT_SUPPORTED;
		break;
	case CMPI_RC_ERR_CLASS_HAS_CHILDREN:
	case CMPI_RC_ERR_CLASS_HAS_INSTANCES:
	case CMPI_RC_ERR_INVALID_SUPERCLASS:
	case CMPI_RC_ERR_ALREADY_EXISTS:
		status->fault_code = WSMAN_ALREADY_EXISTS;
		break;
	case CMPI_RC_ERR_INVALID_QUERY:
		status->fault_code = WSEN_CANNOT_PROCESS_FILTER;
		break;
	case CMPI_RC_ERR_NO_SUCH_PROPERTY:
	case CMPI_RC_ERR_TYPE_MISMATCH:
	case CMPI_RC_ERR_QUERY_LANGUAGE_NOT_SUPPORTED:
	case CMPI_RC_DO_NOT_UNLOAD:
	case CMPI_RC_NEVER_UNLOAD:
	case CMPI_RC_ERROR_SYSTEM:
	case CMPI_RC_ERROR:
	default:
		status->fault_code = WSMAN_UNKNOWN;
	}
	/*
	   if (rc.msg) {
	   status->msg = (char *)soap_alloc(strlen(rc.msg->hdl ), 0 );
	   status->msg = strndup(rc.msg->hdl, strlen(rc.msg->hdl ));
	   }
	   */
}


void cim_release_enum_context(WsEnumerateInfo * enumInfo)
{
	if (!enumInfo->appEnumContext)
		return;

	debug("releasing enumInfo->appEnumContext");
	sfcc_enumcontext *enumcontext = enumInfo->appEnumContext;
	CMPIEnumeration *enumeration;
	CimClientInfo *client;

	enumeration = enumcontext->ecEnumeration;
	client = enumcontext->ecClient;

	if (enumeration) {
		debug("released enumeration");
		CMRelease(enumeration);
	}
	u_free(enumcontext);
}

CimClientInfo *cim_getclient_from_enum_context(WsEnumerateInfo * enumInfo)
{
	CimClientInfo *cimclient = NULL;
	if (enumInfo && enumInfo->appEnumContext) {
		sfcc_enumcontext *enumcontext =
			(sfcc_enumcontext *) enumInfo->appEnumContext;
		cimclient = enumcontext->ecClient;
	}
	return cimclient;
}

CMPIArray *cim_enum_instancenames(CimClientInfo * client,
		char *class_name, WsmanStatus * status)
{
	CMPIStatus rc;
	CMPIObjectPath *objectpath;
	CMPIEnumeration *enumeration;

	CMCIClient *cc = (CMCIClient *) client->cc;

	objectpath =
		newCMPIObjectPath(client->cim_namespace, class_name, NULL);

	enumeration = cc->ft->enumInstanceNames(cc, objectpath, &rc);
	debug("enumInstanceNames() rc=%d, msg=%s",
			rc.rc, (rc.msg) ? (char *) rc.msg->hdl : NULL);

	if (rc.rc) {
		debug("CMCIClient enumInstanceNames() failed");
		cim_to_wsman_status(rc, status);
		return NULL;
	}
	CMPIArray *enumArr = enumeration->ft->toArray(enumeration, NULL);
	debug("Total enumeration items: %d",
			enumArr->ft->getSize(enumArr, NULL));
	cim_to_wsman_status(rc, status);
	return enumArr;
}


CMPICount cim_enum_totalItems(CMPIArray * enumArr)
{
	return enumArr->ft->getSize(enumArr, NULL);
}


char *cim_get_property(CMPIInstance * instance, char *property)
{
	CMPIStatus rc;
	CMPIData data = instance->ft->getProperty(instance, property, &rc);
	char *valuestr = NULL;
	if (CMIsArray(data)) {
		return NULL;
	} else {
		if (data.type != CMPI_null && data.state != CMPI_nullValue) {

			if (data.type != CMPI_ref) {
				valuestr =
					value2Chars(data.type, &data.value);
			}
		}
	}
	return valuestr;
}

char *cim_get_keyvalue(CMPIObjectPath * objpath, char *keyname)
{
	CMPIStatus status;
	char *valuestr;
	debug("Get key property from objpath");

	if (!objpath) {
		debug("objpath is NULL");
		return "";
	}

	CMPIData data = objpath->ft->getKey(objpath, keyname, &status);
	if (status.rc || CMIsArray(data)) {
		return "";
	} else {
		valuestr = value2Chars(data.type, &data.value);
		return valuestr;
	}
}

void
cim_get_enum_items(CimClientInfo * client,
		WsContextH cntx,
		WsXmlNodeH node,
		WsEnumerateInfo * enumInfo,
		char *namespace,
		int maxelements,
		unsigned long maxsize)
{
	WsXmlNodeH itemsNode;
	WsXmlDocH outdoc = NULL;
	WsXmlDocH clonedoc = NULL;
	int c = 0;
	if (node == NULL)
		return;

	itemsNode = ws_xml_add_child(node, namespace, WSENUM_ITEMS, NULL);
	debug("Total items: %d", enumInfo->totalItems);
	debug("enum flags: %lu", enumInfo->flags );

	if(enumInfo->totalItems == 0) {
		outdoc = ws_xml_get_node_doc(node);
		enumInfo->pullResultPtr = outdoc;
	} else if (maxelements> 0) {
		while (maxelements> 0 && enumInfo->index >= 0 &&
				enumInfo->index < enumInfo->totalItems) {
			outdoc = ws_xml_get_node_doc(node);
			ws_xml_destroy_doc(clonedoc);
			clonedoc = ws_xml_duplicate_doc(outdoc);
			if (enumInfo->flags & WSMAN_ENUMINFO_EPR ) {
				c = cim_getEprAt(client, enumInfo,
						itemsNode);
			} else if (enumInfo->flags & WSMAN_ENUMINFO_OBJEPR) {
				c = cim_getEprObjAt(client, enumInfo,
						itemsNode);
			} else {
				c = cim_getElementAt(client, enumInfo,
						itemsNode);
			}
			if(check_envelope_size(outdoc, maxsize, enumInfo->encoding)) {
				enumInfo->pullResultPtr = clonedoc;
				clonedoc = NULL;
				ws_xml_destroy_doc(outdoc);
				break;
			}
			enumInfo->index++;
			maxelements--;
		}
		enumInfo->index--;
		if(clonedoc) {
			enumInfo->pullResultPtr = outdoc;
		}
		ws_xml_destroy_doc(clonedoc);
	} else {
		while (enumInfo->index >= 0
				&& enumInfo->index < enumInfo->totalItems) {
			outdoc = ws_xml_get_node_doc(node);
			ws_xml_destroy_doc(clonedoc);
			clonedoc = ws_xml_duplicate_doc(outdoc);
			if (enumInfo->flags & WSMAN_ENUMINFO_EPR ) {
				c = cim_getEprAt(client, enumInfo,
						itemsNode);
			} else if (enumInfo->flags & WSMAN_ENUMINFO_OBJEPR) {
				c = cim_getEprObjAt(client, enumInfo,
						itemsNode);
			} else {
				c = cim_getElementAt(client, enumInfo, itemsNode);
			}
			if(check_envelope_size(outdoc, enumInfo->maxsize, enumInfo->encoding)) {
				enumInfo->pullResultPtr = clonedoc;
				clonedoc = NULL;
				ws_xml_destroy_doc(outdoc);
				break;
			}
			enumInfo->index++;
		}
		enumInfo->index--;

		if(clonedoc) {
			enumInfo->pullResultPtr = outdoc;
		}
		ws_xml_destroy_doc(clonedoc);
	}
}
