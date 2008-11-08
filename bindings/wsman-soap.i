/*
 * wsman-soap.i
 *
 * soap structure accessors for openwsman swig bindings
 *
 */

%extend __WsEnumerateInfo {
  int maxItems() { return $self->maxItems; }
#ifdef (SWIGRUBY)
  %rename("maxItems=") set_maxItems(int mi);
#endif
  void set_maxItems(int mi) { $self->maxItems = mi; }
}

%extend __SoapOp {
}

%extend _WS_CONTEXT {
  WsXmlDocH create_fault(WsXmlDocH rqstDoc,
    const char *code, const char *subCodeNs, const char *subCode,
    const char *lang, const char *reason,
    void (*addDetailProc) (WsXmlNodeH, void *), void *addDetailProcData)
  {
    return wsman_create_fault($self, rqstDoc, code, subCodeNs, subCode, lang, reason, addDetailProc, addDetailProcData);
  }
  int parse_enum_request(WsEnumerateInfo *enumInfo) {
    return wsman_parse_enum_request( $self, enumInfo);
  }
  hash_t *selector_list(WsXmlDocH doc) {
    return wsman_get_selector_list( $self, doc );
  }
}

%extend _WsmanStatus {
  WsXmlDocH generate_fault(WsXmlDocH doc) {
    return wsman_generate_fault( doc, $self->fault_code, $self->fault_detail_code, $self->fault_msg);
  }
}
