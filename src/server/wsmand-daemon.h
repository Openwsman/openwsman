/*******************************************************************************
* Copyright (C) 2004-2006 Intel Corp. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Intel Corp. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Anas Nashif
 */


#ifndef __WSMAND_DAEMON_H__
#define __WSMAND_DAEMON_H__


#define DEFAULT_SERVICE_PATH "/wsman"
#define ANON_IDENTIFY_PATH "/wsman-anon/identify"
#define DEFAULT_CIMINDICATION_PATH "/cimindicationlistener"
#define DEFAULT_PID_PATH "/var/run/wsmand.pid"

typedef void (*WsmandShutdownFn) (void *);

/* At shutdown time, handlers are executed in the
   reverse of the order in which they are added. */
void wsmand_shutdown_add_handler(WsmandShutdownFn fn, void *user_data);

void wsmand_shutdown_block(void);

void wsmand_shutdown_allow(void);

/* wsmand_shutdown does return.  The actual shutdown happens
   in an idle function. */

void wsmand_shutdown(void);
void wsmand_restart(void);


int wsmand_parse_options(int argc, char **argv);

int wsmand_options_get_daemon_flag(void);
int wsmand_options_get_no_plugins_flag(void);
int wsmand_options_get_use_ssl(void);
int wsmand_options_get_use_ipv4(void);
#ifdef ENABLE_IPV6
int wsmand_options_get_use_ipv6(void);
/* will be called to denote ipv4 fallback */
void wsmand_options_disable_use_ipv6(void);
#endif
int wsmand_options_get_debug_level(void);
int wsmand_options_get_syslog_level(void);
int wsmand_options_get_server_port(void);
int wsmand_options_get_server_ssl_port(void);
char *wsmand_options_get_ssl_key_file(void);
char *wsmand_options_get_ssl_cert_file(void);
int wsmand_options_get_digest(void);
char *wsmand_options_get_digest_password_file(void);
char *wsmand_options_get_basic_password_file(void);
char *wsmand_options_get_service_path(void);
int wsmand_options_get_min_threads(void);
int wsmand_options_get_max_threads(void);
char *wsmand_default_basic_authenticator(void);
char *wsmand_option_get_basic_authenticator(void);
char *wsmand_option_get_basic_authenticator_arg(void);
char *wsmand_options_get_pid_file(void);
unsigned long wsmand_options_get_enumIdleTimeout(void);
const char *wsmand_options_get_config_file(void);
int wsmand_options_get_foreground_debug(void);
char * wsmand_options_get_subscription_repository_uri(void);
char *wsmand_options_get_identify_file(void);
char *wsmand_options_get_anon_identify_file(void);
unsigned int wsmand_options_get_thread_stack_size(void);
int wsmand_options_get_max_connections_per_thread(void);

const char **wsmand_options_get_argv(void);
int wsmand_read_config(dictionary * ini);




#endif				/* __WSMAND_DAEMON_H__ */
