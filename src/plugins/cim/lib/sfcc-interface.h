#ifndef SFCCINTERFACE_H_
#define SFCCINTERFACE_H_



#include <glib.h>
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

CMPIInstance * cim_get_instance (CMCIClient *cc, char *class_name, GList *keys, CMPIStatus *status);
CMPIConstClass * cim_get_class (CMCIClient *cc, char *class_name, CMPIStatus *status); 

CMPICount cim_enum_totalItems (CMPIArray * enumArr);
CMPIArray * cim_enum_instances (CMCIClient *cc, char *class_name , CMPIStatus *status);
CMPIArray *cim_enum_instancenames(CMCIClient *cc, char *class_name, CMPIStatus *status );

void cim_connect_to_cimom(
	CimClientInfo *cimclient,
	char *cim_host, 
	char *cim_host_userid, 
	char *cim_host_passwd, CMPIStatus *status);




#endif /*SFCCINTERFACE_H_*/

