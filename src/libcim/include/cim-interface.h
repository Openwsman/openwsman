#ifndef CIM_INTERFACE_H_
#define CIM_INTERFACE_H_



#define DEFAULT_HTTP_CIMOM_PORT "5988"
#define CIM_NAMESPACE "root/cimv2"


struct __CimClientInfo 
{
	void *cc;
        hash_t          *namespaces;
        hash_t          *selectors;
        char*           cim_namespace;
        char*           resource_uri;
        char*           method;
        char*           requested_class;
            
};
typedef struct __CimClientInfo CimClientInfo;

#endif 

