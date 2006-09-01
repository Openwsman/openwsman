

#ifndef WSMAN_SOAP_ENVELOPE_H_
#define WSMAN_SOAP_ENVELOPE_H_


#define ENFORCE_MUST_UNDERSTAND	"EnforceMustUnderstand"

void wsman_is_valid_envelope(WsmanMessage *msg, WsXmlDocH doc);
int wsman_is_duplicate_message_id (SOAP_FW* fw, WsXmlDocH doc);
char* get_soap_header_value(SOAP_FW* fw, WsXmlDocH doc, char* nsUri, char* name);
WsXmlNodeH get_soap_header_element(SOAP_FW* fw, 
        WsXmlDocH doc, char* nsUri, char* name);
WsXmlDocH build_soap_fault(SOAP_FW *fw, char *soapNsUri, char *faultNsUri, char *code, char *subCode, char *reason, char *detail);
void build_soap_version_fault(SOAP_FW *fw);

WsXmlDocH ws_create_response_envelope(WsContextH cntx, 
        WsXmlDocH rqstDoc, 
        char* action);
WsXmlDocH wsman_build_envelope(WsContextH cntx, char* action, char* replyToUri, char* systemUri, char* resourceUri,
        char* toUri, actionOptions options);
WsXmlDocH wsman_build_inbound_envelope(SOAP_FW* fw, WsmanMessage *msg);


#endif
