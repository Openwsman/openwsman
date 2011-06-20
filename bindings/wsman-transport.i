/*
 * wsman-transport.i
 * client transport declarations for openwsman swig bindings
 *
 */

%rename(Transport) _WsManTransport;
%nodefault _WsManTransport;
typedef struct _WsManTransport {} WsManTransport;

/*
 * Transport is no separate struct in openwsman but part of WsManClient
 *
 * However, accessing transport in e.g. Ruby as
 *  client.transport_<foo>
 * is cumbersome
 *
 * Instead we provide
 *  t = client.transport
 *  t.<foo>
 * as a convenience
 *
 */
#if defined(SWIGJAVA)
%rename(NO_AUTH_STR) _WS_NO_AUTH;
%rename(BASIC_AUTH_STR) _WS_BASIC_AUTH;
%rename(DIGEST_AUTH_STR) _WS_DIGEST_AUTH;
%rename(PASS_AUTH_STR) _WS_PASS_AUTH;
%rename(NTLM_AUTH_STR) _WS_NTLM_AUTH;
%rename(GSSNEGOTIATE_AUTH_STR) _WS_GSSNEGOTIATE_AUTH;
#endif

%ignore _WS_NO_AUTH;
%ignore _WS_BASIC_AUTH;
%ignore _WS_DIGEST_AUTH;
%ignore _WS_PASS_AUTH;
%ignore _WS_NTLM_AUTH;
%ignore _WS_GSSNEGOTIATE_AUTH;

%include "u/libu.h"
%include "wsman-client-transport.h"

%extend WsManTransport {
#ifndef SWIGJAVA
 %constant int NO_AUTH           = WS_NO_AUTH;
 %constant int BASIC_AUTH        = WS_BASIC_AUTH;
 %constant int DIGEST_AUTH       = WS_DIGEST_AUTH;
 %constant int PASS_AUTH         = WS_PASS_AUTH;
 %constant int NTLM_AUTH         = WS_NTLM_AUTH;
 %constant int GSSNEGOTIATE_AUTH = WS_GSSNEGOTIATE_AUTH;

 %constant char *NO_AUTH_STR           = _WS_NO_AUTH;
 %constant char *BASIC_AUTH_STR        = _WS_BASIC_AUTH;
 %constant char *DIGEST_AUTH_STR       = _WS_DIGEST_AUTH;
 %constant char *PASS_AUTH_STR         = _WS_PASS_AUTH;
 %constant char *NTLM_AUTH_STR         = _WS_NTLM_AUTH;
 %constant char *GSSNEGOTIATE_AUTH_STR = _WS_GSSNEGOTIATE_AUTH;
#endif

#if defined(SWIGRUBY)
  %rename("auth_method?") is_auth_method(int method);
#endif
  int is_auth_method(int method) {
    return wsman_is_auth_method((WsManClient *)$self, method);
  }

  void close() {
    wsman_transport_close_transport((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("agent=") set_agent(const char *agent);
#endif
  void set_agent(const char *agent) {
    wsman_transport_set_agent((WsManClient *)$self, agent);
  }
  char *agent() {
    return wsman_transport_get_agent ((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("username=") set_username(char *user_name);
#endif
  void set_username(char *user_name) {
    wsman_transport_set_userName((WsManClient *)$self, user_name);
  }

#if defined(SWIGRUBY)
  %rename("password=") set_password(char *password);
#endif
  void set_password(char *password) {
    wsman_transport_set_password((WsManClient *)$self, password);
  }

#if defined(SWIGRUBY)
  %rename("proxy_username=") set_proxy_username(char *proxy_username);
#endif
  void set_proxy_username(char *proxy_username) {
    wsman_transport_set_proxy_username((WsManClient *)$self, proxy_username );
  }

#if defined(SWIGRUBY)
  %rename("proxy_password=") set_proxy_password(char *proxy_password);
#endif
  void set_proxy_password(char *proxy_password) {
    wsman_transport_set_proxy_password((WsManClient *)$self, proxy_password );
  }

#if defined(SWIGRUBY)
  %rename("auth_method=") set_auth_method( const char *am);
#endif
  void set_auth_method( const char *am) {
    wsman_transport_set_auth_method((WsManClient *)$self, am);
  }
  char *auth_method() {
    return wsman_transport_get_auth_method ((WsManClient *)$self);
  }

  static char *auth_name(int auth) {
    return wsmc_transport_get_auth_name(auth);
  }
  int auth_value() {
    return wsmc_transport_get_auth_value((WsManClient *)$self);
  }
  static char *error_string(int err) {
    return wsman_transport_get_last_error_string(err);
  }

#if defined(SWIGRUBY)
  %rename("timeout=") set_timeout(unsigned long timeout);
#endif
  void set_timeout(unsigned long timeout) {
    wsman_transport_set_timeout((WsManClient *)$self, timeout);
  }
  unsigned long timeout() {
    return wsman_transport_get_timeout((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("verify_peer=") set_verify_peer( unsigned int value );
#endif
  void set_verify_peer( unsigned int value ) {
    wsman_transport_set_verify_peer((WsManClient *)$self, value);
  }
  unsigned int verify_peer() {
    return wsman_transport_get_verify_peer((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("verify_host=") set_verify_host(unsigned int value);
#endif
  void set_verify_host(unsigned int value) {
    wsman_transport_set_verify_host((WsManClient *)$self, value);
  }
  unsigned int verify_host() {
    return wsman_transport_get_verify_host((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("proxy=") set_proxy(const char *proxy);
#endif
  void set_proxy(const char *proxy) {
    wsman_transport_set_proxy((WsManClient *)$self, proxy);
  }
  char *proxy() {
    return wsman_transport_get_proxy((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("proxyauth=") set_proxyauth(const char *pauth);
#endif
  void set_proxyauth(const char *pauth) {
    wsman_transport_set_proxyauth((WsManClient *)$self, pauth);
  }
  char *proxyauth(){
    return wsman_transport_get_proxyauth((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("cainfo=") set_cainfo(const char *cainfo);
#endif
  void set_cainfo(const char *cainfo) {
    wsman_transport_set_cainfo((WsManClient *)$self, cainfo);
  }
  char *cainfo() {
    return wsman_transport_get_cainfo((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("certhumbprint=") set_certhumbprint(const char *arg);
#endif
  void set_certhumbprint(const char *arg) {
    wsman_transport_set_certhumbprint((WsManClient *)$self, arg);
  }
  char *certhumbprint() {
    return wsman_transport_get_certhumbprint((WsManClient *)$self);
  }
  
#if defined(SWIGRUBY)
  %rename("capath=") set_capath(const char *capath);
#endif
  void set_capath(const char *capath) {
    wsman_transport_set_capath((WsManClient *)$self, capath);
  }
  char *capath() {
    return wsman_transport_get_capath((WsManClient *)$self);
  }

#if defined(SWIGRUBY)
  %rename("caoid=") set_caoid(const char *oid);
#endif
  void set_caoid(const char *oid) {
    wsman_transport_set_caoid((WsManClient *)$self, oid);
  }
  char *caoid() {
    return wsman_transport_get_caoid((WsManClient *)$self);
  }

#ifdef _WIN32
#if defined(SWIGRUBY)
  %rename("calocal=") set_calocal(BOOL local);
#endif
  void set_calocal(BOOL local) {
    wsman_transport_set_calocal((WsManClient *)$self, local);
  }
  BOOL calocal() {
    return wsman_transport_get_calocal((WsManClient *)$self);
  }
#endif

#if defined(SWIGRUBY)
  %rename("cert=") set_cert(const char *cert);
#endif
  void set_cert(const char *cert) {
    wsman_transport_set_cert((WsManClient *)$self, cert);
  }
  const char *cert() {
    return wsman_transport_get_cert((WsManClient *)$self);
  }
  
#if defined(SWIGRUBY)
  %rename("key=") set_key(const char *key);
#endif
  void set_key(const char *key) {
    wsman_transport_set_key((WsManClient *)$self, key);
  }
  char *key() {
    return wsman_transport_get_key((WsManClient *)$self);
  }


}
