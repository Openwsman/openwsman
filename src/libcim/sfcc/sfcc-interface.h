#ifndef SFCCINTERFACE_H_
#define SFCCINTERFACE_H_



#include <CimClientLib/cmci.h>


#define DEFAULT_HTTP_CIMOM_PORT "5988"
#define CIM_NAMESPACE "root/cimv2"


struct __CimClientInfo 
{
	CMCIClient *cc;
};
typedef struct __CimClientInfo CimClientInfo;


char* cim_get_property(CMPIInstance *instance, char *property);
char *cim_get_keyvalue(CMPIObjectPath *objpath, char *keyname);

void cim_get_instance (CMCIClient *cc, char *resourceUri, hash_t *keys, WsXmlNodeH body, WsmanStatus *status);
CMPIConstClass * cim_get_class (CMCIClient *cc, char *class_name, WsmanStatus *status); 

CMPICount cim_enum_totalItems (CMPIArray * enumArr);
void cim_enum_instances (CMCIClient *cc, char *class_name , WsEnumerateInfo* enumInfo, WsmanStatus *status);
CMPIArray *cim_enum_instancenames(CMCIClient *cc, char *class_name, WsmanStatus *status );
void cim_release_enum_context( WsEnumerateInfo* enumInfo );

void cim_connect_to_cimom(
	CimClientInfo *cimclient,
	char *cim_host, 
	char *cim_host_userid, 
	char *cim_host_passwd, WsmanStatus *status);



void cim_put_instance_from_enum (CMCIClient *cc, char *resourceUri, hash_t *keys, WsXmlNodeH in_body, WsXmlNodeH body,
        WsmanStatus *status); 
void xml2instance( CMPIInstance *instance, WsXmlNodeH body, char *resourceUri);
void xml2property( CMPIInstance *instance, CMPIData data , char *name , char *value);
void property2xml( CMPIData data, char *name , WsXmlNodeH node, char *resourceUri);
void instance2xml( CMPIInstance *instance , WsXmlNodeH body, char  *resourceUri);
extern char *value2Chars(CMPIType type, CMPIValue * value);
void class2xml( CMPIConstClass * class, WsXmlNodeH node, char *resourceUri );
void path2xml(  WsXmlNodeH node, char *resourceUri ,  CMPIValue *val);
void add_cim_location ( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);
void cim_getElementAt(WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode, char *resourceUri);
void cim_invoke_method (CMCIClient *cc, char *class_name, 
        hash_t *keys, char *method, WsXmlNodeH node,  WsmanStatus *status); 

void cim_to_wsman_status(CMPIStatus sfcc_status, WsmanStatus *status);
CMPIInstance * cim_get_instance_raw (CMCIClient *cc, char *class_name, hash_t *keys );
CMPIArray * cim_enum_instances_raw (CMCIClient *cc, char *class_name );
void cim_get_enum_items(WsContextH cntx, WsXmlNodeH node, WsEnumerateInfo* enumInfo, char *namespace, int max);

void cim_add_epr( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);
void cim_add_epr_details(WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);

void cim_getEprObjAt(WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode, char *resourceUri);
void cim_getEprAt(WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode, char *resourceUri);

void cim_get_instance_from_enum (CMCIClient *cc, char *resourceUri, hash_t *keys, WsXmlNodeH body, WsmanStatus *status);

#endif /*SFCCINTERFACE_H_*/

