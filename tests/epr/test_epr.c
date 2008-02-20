#include <stdio.h>
#include "wsman-epr.h"
#include "wsman-xml-api.h"
#include "wsman-xml.h"
#include "u/libu.h"

static void test_serialize1(void)
{
	hash_t *selectors_filter = hash_create(HASHCOUNT_T_MAX, 0, 0);
	selector_entry *entry1 = NULL;
	entry1 = u_malloc(sizeof(selector_entry)*4);
	entry1[0].type = 0;
	entry1[0].entry.text = "OperatingSystemFilter0";
	entry1[1].type = 0;
        entry1[1].entry.text = "localhost.localdomain";
	entry1[2].type = 0;
        entry1[2].entry.text = "CIM_IndicationFilter";
	entry1[3].type = 0;
        entry1[3].entry.text = "CIM_ComputerSystem";	
	hash_alloc_insert(selectors_filter, "Name", &entry1[0]);
	hash_alloc_insert(selectors_filter, "SystemName", &entry1[1]);
	hash_alloc_insert(selectors_filter, "CreationClassName", &entry1[2]);
	hash_alloc_insert(selectors_filter, "SystemCreationClassName", &entry1[3]);
	epr_t *epr_filter = epr_create("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter", selectors_filter, NULL);
        if(epr_filter == NULL) {
                printf("epr_create filter failed!\n");
                return;
        }

	hash_t *selectors_handler = hash_create(HASHCOUNT_T_MAX, 0, 0);
	selector_entry *entry2 = u_malloc(sizeof(selector_entry)*4);
	entry2[0].type = 0;
        entry2[0].entry.text = "OperatingSystemHandler0";
        entry2[1].type = 0;
        entry2[1].entry.text = "localhost.localdomain";
        entry2[2].type = 0;
        entry2[2].entry.text = "CIM_IndicationHandlerCIMXML";
        entry2[3].type = 0;
        entry2[3].entry.text = "CIM_ComputerSystem";
	hash_alloc_insert(selectors_handler, "Name", &entry2[0]);
        hash_alloc_insert(selectors_handler, "SystemName", &entry2[1]);
        hash_alloc_insert(selectors_handler, "CreationClassName", &entry2[2]);
        hash_alloc_insert(selectors_handler, "SystemCreationClassName", &entry2[3]);
	epr_t *epr_handler = epr_create("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationHandlerCIMXML", selectors_handler, NULL);
	if(epr_handler == NULL) {
		printf("epr_create handler failed!\n");
		return;
	}
	
	hash_t *selectors_subscription =  hash_create(HASHCOUNT_T_MAX, 0, 0);
        selector_entry *entry3 = NULL;
        entry3 = u_malloc(sizeof(selector_entry)*2);
        entry3[0].type = 1;
        entry3[0].entry.eprp = epr_filter;
        entry3[1].type = 1;
        entry3[1].entry.eprp = epr_handler;
        hash_alloc_insert(selectors_subscription, "Filter", &entry3[1]);
	hash_alloc_insert(selectors_subscription, "Handler", &entry3[1]);
	epr_t *epr_subscription = epr_create("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationSubscription", selectors_subscription, NULL);
        if(epr_subscription == NULL) {
                printf("epr_create subscription failed!\n");
                return;
        }

	epr_t *epr_cpy = epr_copy(epr_subscription); //test epr_copy
	WsXmlDocH doc = ws_xml_create_envelope();
	WsXmlNodeH header = ws_xml_get_soap_header(doc);
	epr_serialize(header,NULL,NULL,epr_cpy,0);
	ws_xml_dump_doc(stdout, doc);

	epr_destroy(epr_filter);
	epr_destroy(epr_handler);
	epr_destroy(epr_subscription);
	epr_destroy(epr_cpy);
	hash_free(selectors_filter);
	hash_free(selectors_handler);
	hash_free(selectors_subscription);
	u_free(entry1);
	u_free(entry2);
	u_free(entry3);
	ws_xml_destroy_doc(doc);
	printf("\033[22;32mtest serialize epr successfully!\033[m\n\n");
}

static void test_serialize2(void)
{
	epr_t *epr = epr_from_string("http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem");
	if(epr == NULL) {
		printf("test serialize from string failed!\n");
		return;
	}
	WsXmlDocH doc = ws_xml_create_envelope();
        WsXmlNodeH header = ws_xml_get_soap_header(doc);
        epr_serialize(header,NULL,NULL,epr,0);
        ws_xml_dump_doc(stdout, doc);
	
	ws_xml_destroy_doc(doc);
	epr_destroy(epr);
        printf("\033[22;32mtest create epr from string successfully!\033[m\n\n");
	
}

static void test_deserialize(void)
{
	WsXmlDocH doc = ws_xml_read_file("./sample.xml", "utf-8", 0);
	if(doc == NULL) return;
	WsXmlNodeH node = ws_xml_get_soap_header(doc);
	epr_t *epr = epr_deserialize(node, NULL, NULL, 0);
	if(epr == NULL) {
		printf("epr deserialize failed!\n");
		return;
	}

	ws_xml_destroy_doc(doc);

	doc = ws_xml_create_envelope();
	node = ws_xml_get_soap_header(doc);
	epr_serialize(node, NULL, NULL,epr, 0);
	ws_xml_dump_doc(stdout, doc);
	
	epr_destroy(epr);
	ws_xml_destroy_doc(doc);
	printf("\033[22;32mtest deserialize epr successfully!\033[m\n\n");
}

int main(int argc, char *argv[])
{
	test_serialize1();
	test_serialize2();
	test_deserialize();
	return 0;
}
