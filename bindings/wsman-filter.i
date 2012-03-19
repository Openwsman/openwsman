/*
 * wsman-filter.i
 *
 * filter declarations for openwsman swig bindings
 *
 */

 
%rename(Filter) filter_t;
%nodefault filter_t;
typedef struct {
    char *resultClass;
    char *assocClass;
} filter_t;

/*
 * Filter are evaluated on the server side and help to reduce the amount
 * of processing and information transport.
 *
 * There are five basic ways to filter
 * * associations
 * * references
 * * XPath
 * * CQL (CIM query language)
 * * WQL (WS-Management query language)
 *
 * Openwsman does not do any filter processing by itself but passes it
 * to the backend CIMOM. Support for filters and query languages thus
 * depends on the used CIMOM.
 *
 */
%extend filter_t {
  /*
   * Create empty filter
   *
   */
  filter_t() {
    return filter_initialize();
  }
  ~filter_t() {
    filter_destroy( $self );
  }
#if defined(SWIGJAVA)
  %typemap(in) (char **resultProp, const int propNum) {
	  int i = 0;
	  $2 = (*jenv)->GetArrayLength(jenv, $input);
	  $1 = (char **) malloc(($2+1)*sizeof(char *));
	  /* make a copy of each string */
	  for (i = 0; i<$2; i++) {
		  jstring j_string = (jstring)(*jenv)->GetObjectArrayElement(jenv, $input, i);
		  const char * c_string = (*jenv)->GetStringUTFChars(jenv, j_string, 0);
		  $1[i] = malloc((strlen(c_string)+1)*sizeof(char));
		  strcpy($1[i], c_string);
		  (*jenv)->ReleaseStringUTFChars(jenv, j_string, c_string);
		  (*jenv)->DeleteLocalRef(jenv, j_string);
	  }
	  $1[i] = 0;
  }
/* This cleans up the memory we malloc'd before the function call */
  %typemap(freearg) (char **resultProp, const int propNum) {
	  int i;
	  for (i=0; i<$2-1; i++)
		  free($1[i]);
	  free($1);
  }
  %typemap(jni) (char **resultProp, const int propNum) "jobjectArray"
  %typemap(jtype) (char **resultProp, const int propNum) "String[]"
  %typemap(jstype) (char **resultProp, const int propNum) "String[]"
  %typemap(javain) (char **resultProp, const int propNum) "$javainput"
#endif
  /*
   * Set associators filter
   * call-seq:
   *   filter.associators(end_point_reference, assoc_class_name, result_class_name, role, result_role, result_prop[], prop_num)
   *
   */
  int associators( epr_t *epr, const char *assocClass, const char *resultClass,
        const char *role, const char *resultRole, char **resultProp, const int propNum)
  {
    return filter_set_assoc($self, epr, 0, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }
  /*
   * Set references filter
   * call-seq:
   *   filter.references(end_point_reference, assoc_class_name, result_class_name, role, result_role, result_prop[], prop_num)
   *
   */
  int references( epr_t *epr, const char *assocClass,
    const char *resultClass, const char *role, const char *resultRole, char **resultProp, const int propNum)
  {
    return filter_set_assoc($self, epr, 1, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }

  /*
   * Set simple dialect/query filter
   * call-seq:
   *   filter.simple(dialect, query)
   *
   */
  int simple(const char *dialect, const char *query) {
    return filter_set_simple($self, dialect, query );
  }
  /*
   * Set XPath filter
   * call-seq:
   *   filter.xpath(query)
   *
   */
  int xpath(const char *query) {
    return filter_set_simple($self, WSM_XPATH_FILTER_DIALECT, query );
  }
  /*
   * Set CQL (CIM query language) filter
   * call-seq:
   *   filter.cql(query)
   *
   */
  int cql(const char *query) {
    return filter_set_simple($self, WSM_CQL_FILTER_DIALECT, query );
  }
  /*
   * Set WQL (WS-Management query language) filter
   * call-seq:
   *   filter.wql(query)
   *
   */
  int wql(const char *query) {
    return filter_set_simple($self, WSM_WQL_FILTER_DIALECT, query );
  }

}
