#ifndef RUBY_HELPERS_H
#define RUBY_HELPERS_H

/*
 * openwsman-ruby.c
 *
 * helper functions to convert between ruby and openwsman values
 * 
 * Author: Klaus Kaempf <kkaempf@suse.de>
 * 
 */

/*****************************************************************************
* Copyright (C) 2008 Novell Inc. All rights reserved.
* Copyright (C) 2008 SUSE Linux Products GmbH. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*   - Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
* 
*   - Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
* 
*   - Neither the name of Novell Inc. nor of SUSE Linux Products GmbH nor the
*     names of its contributors may be used to endorse or promote products
*     derived from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell Inc. OR SUSE Linux Products GmbH OR
* THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/


/*
 * Get access to Ruby klass pointers
 * 
 */

#if SWIG_VERSION > 0x020004
#define KLASS_DECL(k,t) swig_class *k = (swig_class *)(t->clientdata)
#define KLASS_OF(x) x->klass
#else
#define KLASS_DECL(k,t) extern swig_class k
#define KLASS_OF(x) x.klass
#endif

/* convert char* to string VALUE */
static VALUE
makestring( const char *s )
{
    if (s) return rb_str_new2( s );
    return Qnil;
}


/* convert VALUE to char* */
static const char *
as_string( VALUE v )
{
  const char *str;
  if (SYMBOL_P(v)) {
    str = rb_id2name(SYM2ID(v));
  }
  else if (TYPE(v) == T_STRING) {
    str = StringValuePtr(v);
  }
  else if (v == Qnil) {
    str = NULL;
  }
  else {
    VALUE v_s = rb_funcall(v, rb_intern("to_s"), 0 );
    str = StringValuePtr(v_s);
  }
  return str;
}


/* convert openwsman hash_t* to hash VALUE (string pairs) */
static VALUE
hash2value( hash_t *hash )
{
    VALUE v;
    hnode_t *node;
    hscan_t ptr;
  
    if (!hash) return Qnil;

    hash_scan_begin( &ptr, hash );

    v = rb_hash_new();
    while ((node = hash_scan_next( &ptr )) ) {
	rb_hash_aset( v, makestring( hnode_getkey( node ) ), makestring( hnode_get( node ) ) );
    }
    return v;
}


/* add key,value VALUE pair to hash_t* as char*
 *  (used as callback for value2hash)
 */
static int
_add_str( VALUE key, VALUE value, VALUE opaque )
{
    hash_t *h = (hash_t *)opaque;
    if (key != Qundef) {
	const char *k = strdup( as_string( key ) );
	if (!hash_lookup( h, k ) ) {
            const char *v = strdup( as_string( value ) );
	    if ( !hash_alloc_insert( h, k, v ) ) {
		rb_raise( rb_eException, "hash_alloc_insert failed" );
            }
	}
    }
    return 0;
}


/* add key,value VALUE pair to hash_t* as key_value_t*
 *  (used as callback for value2hash)
 */
static int
_add_kv( VALUE key, VALUE value, VALUE opaque )
{
    hash_t *h = (hash_t *)opaque;
    if (key != Qundef) {
	const char *k = as_string( key );
	if (!hash_lookup( h, k ) ) {
          epr_t *epr;
          const char *text;
          key_value_t *entry;
          KLASS_DECL(SwigClassEndPointReference,SWIGTYPE_p_epr_t);
          if (CLASS_OF(value) == KLASS_OF(SwigClassEndPointReference)) {
            SWIG_ConvertPtr(value, (void **)&epr, SWIGTYPE_p_epr_t, 0);
            text = NULL;
          }
          else if (TYPE(value) == T_ARRAY) {
            rb_raise( rb_eException, "Passing array parameter via invoke() still unsupported" );
          }
          else {
            text = as_string(value);
            epr = NULL;
          }
          entry = key_value_create(k, text, epr, NULL);
          if ( !hash_alloc_insert( h, k, entry ) ) {
            rb_raise( rb_eException, "hash_alloc_insert failed" );
          }
	}
    }
    return 0;
}


/*
 * Convert Ruby Hash to hash_t
 * 
 * create hash (h == NULL) or add to hash (h != NULL) from hash VALUE
 *
 * valuetype - type of hash values
 *   0 - values are string (char *)
 *   1 - values are key_value_t *
 * 
 */
static hash_t *
value2hash( hash_t *h, VALUE v, int valuetype )
{
    if (NIL_P(v)) return NULL;
  
    Check_Type( v, T_HASH );

    if (!h) h = hash_create3(HASHCOUNT_T_MAX, 0, 0);

    rb_hash_foreach( v, (valuetype==0)?_add_str:_add_kv, (unsigned long)h );

    return h;
}


/*
 * callback function if client authentication fails
 *
 */
static void
auth_request_callback( WsManClient *client, wsman_auth_type_t t, char **username, char **password )
{
    KLASS_DECL(SwigClassTransport,SWIGTYPE_p__WsManTransport);

    VALUE c = SWIG_NewPointerObj((void*) client, SWIGTYPE_p__WsManClient, 0);

    /* ruby callback */
    VALUE result = rb_funcall( KLASS_OF(SwigClassTransport), rb_intern( "auth_request_callback" ), 2, c, INT2NUM( t ) );

    if (CLASS_OF( result ) == rb_cArray) {
      if (RARRAY_LEN(result) == 2 ) {
	*username = strdup(as_string( rb_ary_entry( result, 0 ) ));
	*password= strdup(as_string( rb_ary_entry( result, 1 ) ));
	return;
      }
    }

    *username = NULL;		/* abort authentication */
    return;
}

static int associators_references( void *filter, int type, VALUE epr_v,
  VALUE assocClass_v, VALUE resultClass_v, VALUE role_v, VALUE resultRole_v,
  VALUE resultProp_v, VALUE propNum_v);

#endif /* RUBY_HELPERS_H */
