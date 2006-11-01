#ifndef SFCC_INTERFACE_H_
#define SFCC_INTERFACE_H_



#include <CimClientLib/cmci.h>
#include <cim-interface.h>


char* cim_get_property(CMPIInstance *instance, char *property);

char *cim_get_keyvalue(CMPIObjectPath *objpath, char *keyname);

CMPICount cim_enum_totalItems (CMPIArray * enumArr);

void cim_release_enum_context( WsEnumerateInfo* enumInfo );

CMCIClient* cim_connect_to_cimom( char *cim_host, 
	char *cim_host_userid, 
	char *cim_host_passwd, WsmanStatus *status);

void xml2instance( CMPIInstance *instance, WsXmlNodeH body, char *resourceUri);

void xml2property( CMPIInstance *instance, CMPIData data , char *name , char *value);

void property2xml( CMPIData data, char *name , WsXmlNodeH node, char *resourceUri);

extern char *value2Chars(CMPIType type, CMPIValue * value);

void class2xml( CMPIConstClass * class, WsXmlNodeH node, char *resourceUri );

void path2xml(  WsXmlNodeH node, char *resourceUri ,  CMPIValue *val);

void add_cim_location ( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);

void cim_invoke_method (CimClientInfo* client, WsContextH cntx, WsXmlNodeH node,  WsmanStatus *status); 

void cim_get_instance (CimClientInfo* client, WsContextH cntx, WsXmlNodeH body, WsmanStatus *status);

void cim_enum_instances (CimClientInfo* client, WsEnumerateInfo* enumInfo, WsmanStatus *status);

void cim_put_instance_from_enum (CimClientInfo* client, WsContextH cntx,  WsXmlNodeH in_body, WsXmlNodeH body, WsmanStatus *status); 

CMPIArray *cim_enum_instancenames(CimClientInfo* client, char *class_name, WsmanStatus *status );

void cim_to_wsman_status(CMPIStatus sfcc_status, WsmanStatus *status);

void cim_get_enum_items(CimClientInfo* client, WsContextH cntx, WsXmlNodeH node, WsEnumerateInfo* enumInfo, char *namespace, int max);

void cim_add_epr( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);

void cim_add_epr_details(WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);

int cim_getEprObjAt(CimClientInfo* client, WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode);

int cim_getEprAt(CimClientInfo* client, WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode);

int cim_getElementAt(CimClientInfo* client, WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode);

void cim_get_instance_from_enum (CimClientInfo* cc, WsContextH cntx, WsXmlNodeH body, WsmanStatus *status);

char* cim_get_namespace_selector(hash_t *keys);

#endif /*SFCC_INTERFACE_H_*/

