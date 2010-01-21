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
 
/* convert char* to string VALUE */
static VALUE
makestring( const char *s )
{
    if (s) return rb_str_new2( s );
    return Qnil;
}


/* convert symbol or string VALUE to char* */
static const char *
as_string( VALUE v )
{
    const char *s;
    if ( SYMBOL_P( v ) ) {
	ID id = SYM2ID( v );
	s = rb_id2name( id );
    }
    else {
	s = StringValuePtr( v );
    }
    return s;
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


/* add key,value VALUE pair to hash_t*
 *  (used as callback for value2hash)
 */
static int
add_i( VALUE key, VALUE value, hash_t *h )
{
    if (key != Qundef) {
	const char *k = as_string( key );
        const char *v = as_string( value );

	if (!hash_lookup( h, k ) ) {
	    if ( !hash_alloc_insert( h, k, v ) ) {
		rb_raise( rb_eException, "hash_alloc_insert failed" );
            }
	}
    }
    return 0;
}


/* create hash (h == NULL) or add to hash (h != NULL) from hash VALUE */
static hash_t *
value2hash( hash_t *h, VALUE v )
{
    if (NIL_P(v)) return NULL;
  
    Check_Type( v, T_HASH );

    if (!h) h = hash_create(HASHCOUNT_T_MAX, 0, 0);

    rb_hash_foreach( v, add_i, (unsigned long)h );

    return h;
}


/*
 * callback function if client authentication fails
 *
 */
static void
auth_request_callback( WsManClient *client, wsman_auth_type_t t, char **username, char **password )
{

/*
 * Uhm, swig 1.3.40 (or earlier) renamed its internal class variables from
 * cFoo to SwigClassFoo
 * 1.3.36 certainly used cFoo
 *
 */

#if SWIG_VERSION < 0x010340
#define TRANSPORT_CLASS cTransport
#else
#define TRANSPORT_CLASS SwigClassTransport
#endif
  
    extern swig_class TRANSPORT_CLASS;
    VALUE c = SWIG_NewPointerObj((void*) client, SWIGTYPE_p__WsManClient, 0);

    /* ruby callback */
    VALUE result = rb_funcall( TRANSPORT_CLASS.klass, rb_intern( "auth_request_callback" ), 2, c, INT2NUM( t ) );

    if (CLASS_OF( result ) == rb_cArray) {
	if (RARRAY(result)->len == 2 ) {
	    VALUE first = rb_ary_entry( result, 0 );
	    VALUE second = rb_ary_entry( result, 1 );
	    if ((TYPE( first ) == T_STRING)
		&& (TYPE( second ) == T_STRING) )
	    {
		*username = StringValuePtr( first );
		*password= StringValuePtr( second );
		return;
	    }
	}
    }

    *username = NULL;		/* abort authentication */
    return;
}
