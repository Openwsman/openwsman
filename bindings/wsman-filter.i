/*
 * wsman-filter.i
 *
 * filter declarations for openwsman swig bindings
 *
 */

%extend filter_t {
    filter_t() {
        return filter_initialize();
    }
  ~filter_t() {
    filter_destroy( $self );
  }

  int associators( epr_t *epr, const char *assocClass, const char *resultClass,
        const char *role, const char *resultRole, char **resultProp, const int propNum) {
    return filter_set_assoc($self, epr, 0, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }
  int references( epr_t *epr, const char *assocClass,
    const char *resultClass, const char *role, const char *resultRole, char **resultProp, const int propNum) {
    return filter_set_assoc($self, epr, 1, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }

  int simple(const char *dialect, const char *query) {
    return filter_set_simple($self, dialect, query );
  }
  int xpath(const char *query) {
    return filter_set_simple($self, WSM_XPATH_FILTER_DIALECT, query );
  }
  int cql(const char *query) {
    return filter_set_simple($self, WSM_CQL_FILTER_DIALECT, query );
  }
  int wql(const char *query) {
    return filter_set_simple($self, WSM_WQL_FILTER_DIALECT, query );
  }

}

