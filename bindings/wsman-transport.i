/*
 * wsman-transport.i
 * client transport declarations for openwsman swig bindings
 *
 */

%rename(Transport) _WsManTransport;
%nodefault _WsManTransport;
typedef struct _WsManTransport {} WsManTransport;

/*
 * Document-class: Transport
 *
 * Transport reflects details of the http(s) transport layer between
 * client and server.
 *
 */

/* rename those as <foo>_STR below */
%ignore _WS_NO_AUTH;
%ignore _WS_BASIC_AUTH;
%ignore _WS_DIGEST_AUTH;
%ignore _WS_PASS_AUTH;
%ignore _WS_NTLM_AUTH;
%ignore _WS_GSSNEGOTIATE_AUTH;

%include "u/libu.h"
%include "wsman-client-transport.h"

%extend _WsManTransport {

 /* No authentication */
 %constant int NO_AUTH           = WS_NO_AUTH;
 /* HTTP basic auth */
 %constant int BASIC_AUTH        = WS_BASIC_AUTH;
 /* HTTP digest auth */
 %constant int DIGEST_AUTH       = WS_DIGEST_AUTH;
 /* Windows Passport auth */
 %constant int PASS_AUTH         = WS_PASS_AUTH;
 /* Windows NT Lan manager auth */
 %constant int NTLM_AUTH         = WS_NTLM_AUTH;
 /* GSSAPI auth */
 %constant int GSSNEGOTIATE_AUTH = WS_GSSNEGOTIATE_AUTH;

 /* No authentication */
 %constant char *NO_AUTH_STR           = _WS_NO_AUTH;
 /* HTTP basic auth */
 %constant char *BASIC_AUTH_STR        = _WS_BASIC_AUTH;
 /* HTTP digest auth */
 %constant char *DIGEST_AUTH_STR       = _WS_DIGEST_AUTH;
 /* Windows Passport auth */
 %constant char *PASS_AUTH_STR         = _WS_PASS_AUTH;
 /* Windows NT Lan manager auth */
 %constant char *NTLM_AUTH_STR         = _WS_NTLM_AUTH;
 /* GSSAPI auth */
 %constant char *GSSNEGOTIATE_AUTH_STR = _WS_GSSNEGOTIATE_AUTH;

#if defined(SWIGRUBY)
  %rename("auth_method?") is_auth_method(int method);
  %typemap(out) int is_auth_method
    "$result = ($1 != 0) ? Qtrue : Qfalse;";
#endif
  /*
   * Check if the passed method id is valid for authentication
   *
   * call-seq:
   *   transport.auth_method?(Integer) -> Boolean
   *
   */
  int is_auth_method(int method) {
    return wsman_is_auth_method((WsManClient *)$self, method);
  }

  /*
   * Close the transport. No further communication possible.
   *
   */
  void close() {
    wsman_transport_close_transport((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("agent=") set_agent(const char *agent);
#endif
  /*
   * Set the HTTP agent identifier (User-agent:) string
   *
   * This is how the client will show up in the servers http log.
   * Defaults to "Openwsman"
   *
   * call-seq:
   *   transport.agent = "Client identifier"
   *
   */
  void set_agent(const char *agent) {
    wsman_transport_set_agent((WsManClient *)$self, agent);
  }
  %newobject agent;
  /*
   * Get the HTTP agent identifier string
   *
   * call-seq:
   *   transport.agent -> String
   *
   */
  char *agent() {
    return wsman_transport_get_agent ((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("username") get_username();
#endif
  %newobject get_username;
  /*
   * Server credentials
   *
   * Get the username part of the http transport credentials
   *
   * call-seq:
   *   transport.username -> String
   *
   */
  char *get_username() {
    return wsman_transport_get_userName((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("username=") set_username(char *user_name);
#endif
  /*
   * Server credentials
   *
   * Set the username part of the http transport credentials
   *
   * call-seq:
   *   transport.username = "Username"
   *
   */
  void set_username(char *user_name) {
    wsman_transport_set_userName((WsManClient *)$self, user_name);
  }

#if defined(SWIGRUBY)
  %rename("password") get_password();
#endif
  %newobject get_password;
  /*
   * Server credentials
   *
   * Get the password part of the http transport credentials
   *
   * call-seq:
   *   transport.password -> String
   *
   */
  char *get_password() {
    return wsman_transport_get_password((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("password=") set_password(char *password);
#endif
  /*
   * Server credentials
   *
   * Set the password part of the http transport credentials
   *
   * call-seq:
   *   transport.password = "Password"
   *
   */
  void set_password(char *password) {
    wsman_transport_set_password((WsManClient *)$self, password);
  }

#if defined(SWIGRUBY)
  %rename("proxy_username") get_proxy_username();
#endif
  %newobject get_proxy_username;
  /*
   * Windows clients: HTTP proxy credentials
   *
   * Get the username part of the http proxy credentials
   *
   * call-seq:
   *   transport.proxy_username -> String
   *
   */
  char *get_proxy_username() {
    return wsman_transport_get_proxy_username((WsManClient *)$self );
  }

#if defined(SWIGRUBY)
  %rename("proxy_username=") set_proxy_username(char *proxy_username);
#endif
  /*
   * Windows clients: HTTP proxy credentials
   *
   * Set the username part of the http proxy credentials
   *
   * call-seq:
   *   transport.proxy_username = "proxy_username"
   *
   */
  void set_proxy_username(char *proxy_username) {
    wsman_transport_set_proxy_username((WsManClient *)$self, proxy_username );
  }

#if defined(SWIGRUBY)
  %rename("proxy_password") get_proxy_password();
#endif
  %newobject get_proxy_password;
  /*
   * Windows clients: HTTP proxy credentials
   *
   * Get the password part of the http proxy credentials
   *
   * call-seq:
   *   transport.proxy_password -> String
   *
   */
  char *get_proxy_password() {
    return wsman_transport_get_proxy_password((WsManClient *)$self );
  }

#if defined(SWIGRUBY)
  %rename("proxy_password=") set_proxy_password(char *proxy_password);
#endif
  /*
   * Windows clients: HTTP proxy credentials
   *
   * Set the password part of the http proxy credentials
   *
   * call-seq:
   *   transport.proxy_password = "proxy_password"
   *
   */
  void set_proxy_password(char *proxy_password) {
    wsman_transport_set_proxy_password((WsManClient *)$self, proxy_password );
  }

#if defined(SWIGRUBY)
  %rename("auth_method=") set_auth_method( const char *am);
#endif
  /*
   * Set the authentication method
   *
   * Value must be one of:
   * * Openwsman::NO_AUTH_STR
   * * Openwsman::BASIC_AUTH_STR
   * * Openwsman::DIGEST_AUTH_STR
   * * Openwsman::PASS_AUTH_STR
   * * Openwsman::NTLM_AUTH_STR
   * * Openwsman::GSSNEGOTIATE_AUTH_STR
   *
   */
  void set_auth_method(const char *am) {
    wsman_transport_set_auth_method((WsManClient *)$self, am);
  }
  %newobject auth_method;
  /*
   * Set the authentication method
   *
   * call-seq:
   *   transport.auth_method -> String
   *
   */
  char *auth_method() {
    return wsman_transport_get_auth_method ((WsManClient *)$self);
  }

  /*
   * Set the authentication method string corresponding to the given
   * auth method id
   *
   * Value must be one of:
   * * Openwsman::NO_AUTH
   * * Openwsman::BASIC_AUTH
   * * Openwsman::DIGEST_AUTH
   * * Openwsman::PASS_AUTH
   * * Openwsman::NTLM_AUTH
   * * Openwsman::GSSNEGOTIATE_AUTH
   *
   * call-seq:
   *   transport.auth_name(Integer) -> String
   *
   */
  static const char *auth_name(int auth) {
    return wsmc_transport_get_auth_name(auth);
  }
  /*
   * Get the authentication method integer id
   *
   */
  int auth_value() {
    return wsmc_transport_get_auth_value((WsManClient *)$self);
  }
  /*
   * Get string corresponding to given error code
   *
   * call-seq:
   *   transport.error_string(Integer) -> String
   *
   */
  static char *error_string(int err) {
    return wsman_transport_get_last_error_string(err);
  }

#if defined(SWIGRUBY)
  %rename("timeout=") set_timeout(unsigned long timeout);
#endif
  /*
   * Set the transport timeout in seconds
   *
   * ====== Note
   * This is the http layer timeout. Not to be confused with the
   * WS-Management operation timeout set via Openwsman::ClientOptions.timeout
   *
   */
  void set_timeout(unsigned long timeout) {
    wsman_transport_set_timeout((WsManClient *)$self, timeout);
  }
  /*
   * Get the transport timeout in seconds
   *
   * call-seq:
   *   transport.timeout -> Integer
   *
   */
  unsigned long timeout() {
    return wsman_transport_get_timeout((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("verify_peer=") set_verify_peer( VALUE rvalue );
 /*
  * verify the peer in SSL communication ?
  *
  * If passed +false+, +nil+, or 0: disable peer verification
  * else: enable peer verification
  *
  */
  void set_verify_peer( VALUE rvalue ) {
    unsigned int value;
    if ((rvalue == Qfalse) || (rvalue == Qnil)) {
      value = 0;
    }
    else if ((TYPE(rvalue) == T_FIXNUM) && (FIX2INT(rvalue) == 0)) {
      value = 0;
    }
    else {
      value = 1;
    }
#else
 /*
  * verify the peer in SSL communication ?
  * no: == 0
  * yes: != 0
  *
  */
  void set_verify_peer( unsigned int value ) {
#endif
    wsman_transport_set_verify_peer((WsManClient *)$self, value);
  }
#if defined(SWIGRUBY)
  %rename("verify_peer?") verify_peer();
  %typemap(out) unsigned int verify_peer
    "$result = ($1 != 0) ? Qtrue : Qfalse;";
#endif
  /*
   * Peer to be verified ?
   *
   * call-seq:
   *   transport.verify_peer? -> Boolean
   *
   */
  unsigned int verify_peer() {
    return wsman_transport_get_verify_peer((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("verify_host=") set_verify_host(VALUE rvalue);
  /*
  * verify the host in SSL communication ?
  *
  * If passed +false+, +nil+, or 0: disable peer verification
  * else: enable peer verification
  *
  */
  void set_verify_host( VALUE rvalue ) {
    unsigned int value;
    if ((rvalue == Qfalse) || (rvalue == Qnil)) {
      value = 0;
    }
    else if ((TYPE(rvalue) == T_FIXNUM) && (FIX2INT(rvalue) == 0)) {
      value = 0;
    }
    else {
      value = 1;
    }
#else
  /*
  * verify the host in SSL communication ?
  * no: == 0
  * yes: != 0
  *
  */
  void set_verify_host(unsigned int value) {
#endif
    wsman_transport_set_verify_host((WsManClient *)$self, value);
  }
#if defined(SWIGRUBY)
  %rename("verify_host?") verify_host();
  %typemap(out) unsigned int verify_host
    "$result = ($1 != 0) ? Qtrue : Qfalse;";
#endif
  /*
   * Host to be verified ?
   *
   * call-seq:
   *   transport.verify_host? -> Boolean
   *
   */
  unsigned int verify_host() {
    return wsman_transport_get_verify_host((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("proxy=") set_proxy(const char *proxy);
#endif
  /*
   * Set http proxy URL
   *
   * Pass nil to disable proxy communication
   *
   * ====== Example
   *   transport.proxy = "http://your.proxy.com:80"
   *
   */
  void set_proxy(const char *proxy) {
    wsman_transport_set_proxy((WsManClient *)$self, proxy);
  }
  %newobject proxy;
  /*
   * Get http proxy URL
   *
   */
  char *proxy() {
    return wsman_transport_get_proxy((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("proxyauth=") set_proxyauth(const char *pauth);
#endif
  /*
   * Linux clients: HTTP proxy credentials
   *
   * Set the proxy username and password
   *
   * ====== Example
   *   transport.proxyauth = "username:password"
   *
   */
  void set_proxyauth(const char *pauth) {
    wsman_transport_set_proxyauth((WsManClient *)$self, pauth);
  }
  %newobject proxyauth;
  /*
   * Linux clients: HTTP proxy credentials
   *
   * Get the proxy username and password as "username:password"
   *
   * call-seq:
   *   transport.proxyauth -> String
   *
   */
  char *proxyauth(){
    return wsman_transport_get_proxyauth((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("cainfo=") set_cainfo(const char *cainfo);
#endif
  /*
   * Set the certification authority (CAINFO)
   *
   */
  void set_cainfo(const char *cainfo) {
    wsman_transport_set_cainfo((WsManClient *)$self, cainfo);
  }
  %newobject cainfo;
  /*
   * Get the certification authority (CAINFO)
   *
   * call-seq:
   *   transport.cainfo -> String
   *
   */
  char *cainfo() {
    return wsman_transport_get_cainfo((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("certhumbprint=") set_certhumbprint(const char *arg);
#endif
  /*
   * Set the certification thumbprint
   *
   */
  void set_certhumbprint(const char *arg) {
    wsman_transport_set_certhumbprint((WsManClient *)$self, arg);
  }
  %newobject certhumbprint;
  /*
   * Set the certification thumbprint
   *
   * call-seq:
   *   transport.certhumbprint -> String
   *
   */
  char *certhumbprint() {
    return wsman_transport_get_certhumbprint((WsManClient *)$self);
  }
  
#if defined(SWIGRUBY)
  %rename("capath=") set_capath(const char *capath);
#endif
  /*
   * Set the path to the certification authority (CAINFO) store
   *
   */
  void set_capath(const char *capath) {
    wsman_transport_set_capath((WsManClient *)$self, capath);
  }
  %newobject capath;
  /*
   * Get the path to the certification authority (CAINFO) store
   *
   */
  char *capath() {
    return wsman_transport_get_capath((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("caoid=") set_caoid(const char *oid);
#endif
  /*
   * Windows client
   *
   * Set the CA OID
   *
   * ====== Reference
   * http://support.microsoft.com/kb/287547
   *
   */
  void set_caoid(const char *oid) {
    wsman_transport_set_caoid((WsManClient *)$self, oid);
  }
  %newobject caoid;
  /*
   * Windows client
   *
   * Get the CA OID
   *
   */
  char *caoid() {
    return wsman_transport_get_caoid((WsManClient *)$self);
  }

#ifdef _WIN32
#if defined(SWIGRUBY)
  %rename("calocal=") set_calocal(BOOL local);
#endif
  /*
   * Windows client
   *
   * Use local CA ?
   *
   */
  void set_calocal(BOOL local) {
    wsman_transport_set_calocal((WsManClient *)$self, local);
  }
  /*
   * Windows client
   *
   * Use local CA ?
   *
   * call-seq:
   *   transport.calocal -> Boolean
   *
   */
  BOOL calocal() {
    return wsman_transport_get_calocal((WsManClient *)$self);
  }
#endif

#if defined(SWIGRUBY)
  %rename("cert=") set_cert(const char *cert);
#endif
  /*
   * Set the certificate
   *
   */
  void set_cert(const char *cert) {
    wsman_transport_set_cert((WsManClient *)$self, cert);
  }
  %newobject cert;
  /*
   * Get the certificate
   *
   */
  char *cert() {
    return wsman_transport_get_cert((WsManClient *)$self);
  }
  
#if defined(SWIGRUBY)
  %rename("key=") set_key(const char *key);
#endif
  /*
   * Set the key
   *
   */
  void set_key(const char *key) {
    wsman_transport_set_key((WsManClient *)$self, key);
  }
  %newobject key;
  /*
   * Get the key
   *
   */
  char *key() {
    return wsman_transport_get_key((WsManClient *)$self);
  }


}
