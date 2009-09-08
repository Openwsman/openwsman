#ifndef SFCC_INTERFACE_H_
#define SFCC_INTERFACE_H_



#include <CimClientLib/cmci.h>
#include <cim-interface.h>


char *cim_get_property(CMPIInstance * instance, char *property);

char *cim_get_keyvalue(CMPIObjectPath * objpath, char *keyname);

CMPICount cim_enum_totalItems(CMPIArray * enumArr);

void cim_release_enum_context(WsEnumerateInfo * enumInfo);

CimClientInfo *cim_getclient_from_enum_context(WsEnumerateInfo * enumInfo);

CMCIClient *cim_connect_to_cimom(char *cim_host, char *cim_port,
				 char *cim_host_userid,
				 char *cim_host_passwd,
				 char * frontend,
				 WsmanStatus * status);

void cim_release_client(CimClientInfo * cimclient);

void release_cmpi_data(CMPIData data);

void
create_instance_from_xml(CMPIInstance * instance,
			 CMPIConstClass * class,
			 WsXmlNodeH body, char *fragstr,
			 char *resource_uri, WsmanStatus * status);

void
xml2instance(CMPIInstance * instance, WsXmlNodeH body, char *resourceUri);

void xml2objectpath(CMPIObjectPath * objectpath, CMPIData *data, char *name,
		  char *value);

void xml2property(CMPIInstance * instance, CMPIData *data, char *name,
		  char *value);

void property2xml(CimClientInfo * client, CMPIData *data, const char *name,
		  WsXmlNodeH node, char *resourceUri, int frag_type, int is_key);

extern char *value2Chars(CMPIType type, CMPIValue * value);

void class2xml(CMPIConstClass * class, WsXmlNodeH node, char *resourceUri);

void path2xml(CimClientInfo * client, WsXmlNodeH node, char *resourceUri,
	      CMPIValue * val);

void type2xml(CimClientInfo * client, WsXmlNodeH node, char *resource_uri,
	      CMPIType type);

WsXmlNodeH datatype2xml(CimClientInfo * client, WsXmlNodeH parent,
	      char *resource_uri, const char *nodename, const char *name,
	      CMPIData *data);

void qualifiers2xml(CimClientInfo *client, WsXmlNodeH node,
	      CMPIConstClass *_class, const char *property_name);

void add_cim_location(WsXmlNodeH resource, char *resourceUri,
		      CMPIObjectPath * objectpath);

void cim_invoke_method(CimClientInfo * client, WsContextH cntx,
		       WsXmlNodeH node, WsmanStatus * status);

void cim_get_instance(CimClientInfo * client, WsContextH cntx,
		      WsXmlNodeH body, char *fragstr, WsmanStatus * status);

void cim_delete_instance(CimClientInfo * client, WsmanStatus * status);

void cim_enum_instances(CimClientInfo * client, WsEnumerateInfo * enumInfo,
		WsmanStatus * status);

#if 0
void cim_enum_reference_instances(CimClientInfo * client,
				  WsEnumerateInfo * enumInfo,
				  hash_t *selectors,
				  WsmanStatus * status);

void cim_enum_associator_instances(CimClientInfo * client,
          WsEnumerateInfo * enumInfo,
          hash_t *selectors,
          WsmanStatus * status);

#endif
void cim_put_instance(CimClientInfo * client, WsContextH cntx,
		      WsXmlNodeH in_body, WsXmlNodeH body, char *fragstr,
		      WsmanStatus * status);

void cim_create_instance(CimClientInfo * client, WsContextH cntx,
			 WsXmlNodeH in_body, WsXmlNodeH body, char *fragstr,
			 WsmanStatus * status);

CMPIArray *cim_enum_instancenames(CimClientInfo * client, char *class_name,
				  WsmanStatus * status);

void cim_to_wsman_status(CMPIStatus sfcc_status, WsmanStatus * status);

void cim_get_enum_items(CimClientInfo * client, WsContextH cntx,
			WsXmlNodeH node, WsEnumerateInfo * enumInfo,
			char *namespace, int maxelements, unsigned long maxsize);

void cim_add_epr(CimClientInfo * client, WsXmlNodeH resource,
		 char *resourceUri, CMPIObjectPath * objectpath);

void cim_add_epr_details(CimClientInfo * client, WsXmlNodeH resource,
			 char *resourceUri, CMPIObjectPath * objectpath);

int cim_getEprObjAt(CimClientInfo * client, WsEnumerateInfo * enumInfo,
		    WsXmlNodeH itemsNode);

int cim_getEprAt(CimClientInfo * client, WsEnumerateInfo * enumInfo,
		 WsXmlNodeH itemsNode);

int cim_getElementAt(CimClientInfo * client, WsEnumerateInfo * enumInfo,
		     WsXmlNodeH itemsNode);

CMPIInstance *
cim_get_instance_from_selectors(CimClientInfo * client,
		 WsContextH cntx, WsmanStatus * status);

CMPIObjectPath *
cim_get_objectpath_from_selectors(CimClientInfo * client,
		 WsContextH cntx, WsmanStatus * status);

void cim_get_instance_from_enum(CimClientInfo * cc, WsContextH cntx,
				WsXmlNodeH body, char * fragstr, WsmanStatus * status);

char *cim_get_namespace_selector(hash_t * keys);

void cim_delete_instance_from_enum(CimClientInfo * client,
				   WsmanStatus * status);
CMPIObjectPath *cim_create_indication_filter(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status);
CMPIObjectPath *cim_create_indication_handler(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status);
void cim_create_indication_subscription(CimClientInfo * client, WsSubscribeInfo *subsInfo, CMPIObjectPath *filter, 
	CMPIObjectPath *handler, WsmanStatus *status);
void cim_update_indication_subscription(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status);
void cim_delete_indication_subscription(CimClientInfo *client, WsSubscribeInfo *subsInfo, WsmanStatus *status);

void invoke_enumerate_class_names(CimClientInfo *client, WsXmlNodeH body, CMPIStatus *rc);
void invoke_get_class(CimClientInfo *client, WsXmlNodeH body, CMPIStatus *rc);

#endif				/*SFCC_INTERFACE_H_ */
