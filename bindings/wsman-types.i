/*
 * wsman-types.i
 * type definitions for openwsman swig bindings
 */

/* local definitions
 *
 * Openwsman handles some structures as 'anonymous', just declaring
 * them without exposing their definition.
 * However, SWIG need the definition in order to create bindings.
 */

/*
 * hash_t typemaps
 */
%typemap(out) hash_t * {
 $result = hash2value($1);
}

%typemap(in) hash_t * {
 $input = value2hash(NULL, $1);
}


%include "wsman-types.h"
