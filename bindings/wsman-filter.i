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
  filter_t() {
    return filter_initialize();
  }
  ~filter_t() {
    filter_destroy( $self );
  }
  /*
   * Set associators filter
   */
  int associators( epr_t *epr, const char *assocClass, const char *resultClass,
        const char *role, const char *resultRole, char **resultProp, const int propNum)
  {
    return filter_set_assoc($self, epr, 0, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }
  /*
   * Set references filter
   */
  int references( epr_t *epr, const char *assocClass,
    const char *resultClass, const char *role, const char *resultRole, char **resultProp, const int propNum)
  {
    return filter_set_assoc($self, epr, 1, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }

  /*
   * Set simple dialect/query filter
   */
  int simple(const char *dialect, const char *query) {
    return filter_set_simple($self, dialect, query );
  }
  /*
   * Set XPath filter
   */
  int xpath(const char *query) {
    return filter_set_simple($self, WSM_XPATH_FILTER_DIALECT, query );
  }
  /*
   * Set CQL (CIM query language) filter
   */
  int cql(const char *query) {
    return filter_set_simple($self, WSM_CQL_FILTER_DIALECT, query );
  }
  /*
   * Set WQL (WS-Management query language) filter
   */
  int wql(const char *query) {
    return filter_set_simple($self, WSM_WQL_FILTER_DIALECT, query );
  }

}
