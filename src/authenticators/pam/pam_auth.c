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
 * @author Vadim Revyakin
 */
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define NO_DLLOAD

#include <string.h>
#include <stdlib.h>

#if defined(HAVE_PAM_PAM_APPL_H)
#include <pam/pam_appl.h>
#elif defined(HAVE_SECURITY_PAM_APPL_H)
#include <security/pam_appl.h>
#endif

#if defined(HAVE_PAM_PAM_MISC_H)
#include <pam/pam_misc.h>
#elif defined(HAVE_SECURITY_PAM_MISC_H)
#include <security/pam_misc.h>
#endif


#ifndef STANDALONE
#include "u/libu.h"
int initialize(void *arg);
int authorize(char *username, const char *password);
#endif


static char *service = "openwsman";


#define debug_dlsym(sym) \
	debug("Could not dlsym %s", sym)

#ifdef STANDALONE

#include <stdio.h>

#define debug(frmt, ...) \
	printf(frmt, __VA_ARGS__); printf("\n")

#define PAM_start pam_start
#define PAM_authenticate pam_authenticate
#define PAM_acct_mgmt pam_acct_mgmt
#define PAM_end pam_end
#define PAM_strerror pam_strerror

#else
#ifdef NO_DLLOAD

#define PAM_start pam_start
#define PAM_authenticate pam_authenticate
#define PAM_acct_mgmt pam_acct_mgmt
#define PAM_end pam_end
#define PAM_strerror pam_strerror

int
initialize(void *arg)
{
	return 0;
}

#else // !STANDALONE && !NO_DLLOAD

#include <dlfcn.h>

#define LIBPAM   "libpam.so"


static int (*PAM_start)(const char *service_name,
		const char *user,
		const struct pam_conv *pam_conversation,
		pam_handle_t **pamh);
static int (*PAM_authenticate)(pam_handle_t *pamh, int flags);
static int (*PAM_acct_mgmt)(pam_handle_t *pamh, int flags);
static int (*PAM_end)(pam_handle_t *pamh, int pam_status);
static int (*PAM_strerror)(pam_handle_t *pamh, int errnum);



int
initialize(void *arg)
{
	void *hnd;

	hnd = dlopen(LIBPAM, RTLD_LAZY | RTLD_GLOBAL);
	if (hnd == NULL) {
		debug("Could not dlopen %s", LIBPAM);
		return 1;
	}
	PAM_start = dlsym(hnd, "pam_start");
	if (PAM_start == NULL) {
		debug_dlsym("pam_start");
		dlclose(hnd);
		return 1;
	}
	PAM_authenticate = dlsym(hnd, "pam_authenticate");
	if (PAM_authenticate == NULL) {
		debug_dlsym("pam_authenticate");
		dlclose(hnd);
		return 1;
	}
	PAM_acct_mgmt = dlsym(hnd, "pam_acct_mgmt");
	if (PAM_acct_mgmt == NULL) {
		debug_dlsym("pam_acct_mgmt");
		dlclose(hnd);
		return 1;
	}
	PAM_end = dlsym(hnd, "pam_end");
	if (PAM_end == NULL) {
		debug_dlsym("pam_end");
		dlclose(hnd);
		return 1;
	}
	PAM_strerror = dlsym(hnd, "pam_strerror");
	if (PAM_strerror == NULL) {
		debug_dlsym("pam_strerror");
		dlclose(hnd);
		return 1;
	}


	if (arg != NULL) {
		service = (char *)arg;
	}
	//    debug_level = dbg_lvl;

	return 0;
}


#endif // NO_DLLOAD
#endif // STANDALONE


static int
#if defined (__SVR4) && defined (__sun)
pwd_conv(int num_msg, struct pam_message **msgm,
#else
pwd_conv(int num_msg, const struct pam_message **msgm,
#endif
		struct pam_response **response, void *appdata_ptr)
{
	char *pwd = (char *)appdata_ptr;
	int n;
	struct pam_response *reply;
	reply = (struct pam_response *) calloc(num_msg,
			sizeof(struct pam_response));
	if (reply == NULL) {
		debug("No %s", "memory");
		return PAM_CONV_ERR;
	}

	for (n = 0; n < num_msg; n++) {
		switch (msgm[n]->msg_style) {
		case PAM_PROMPT_ECHO_OFF:
		case PAM_PROMPT_ECHO_ON:
			reply[n].resp = strdup(pwd);
			break;
		}
	}

	*response = reply;
	return PAM_SUCCESS;
}

#ifdef STANDALONE
static
#endif
int
authorize(char *username, const char *password)
{
	struct pam_conv conv = {
		pwd_conv,
		(void *)password
	};
	pam_handle_t   *pamh = NULL;
	int             r;
	int             res = 0;
	// printf("service = %s\n", service);
	r = PAM_start(service, username, &conv, &pamh);
	if (r != PAM_SUCCESS) {
		debug("pam_start failed = %d(%s)", r, PAM_strerror(pamh, r));
		return 0;
	}

	r = PAM_authenticate(pamh, PAM_SILENT | PAM_DISALLOW_NULL_AUTHTOK);
	if (r != PAM_SUCCESS) {
		debug("pam_authenticate failed = %d(%s)", r, PAM_strerror(pamh, r));
		goto DONE;
	}
	r = PAM_acct_mgmt(pamh, PAM_SILENT | PAM_DISALLOW_NULL_AUTHTOK);
	if (r != PAM_SUCCESS) {
		debug("pam_ acct_mgmt failed = %d(%s)", r, PAM_strerror(pamh, r));
		goto DONE;
	}
	res = 1;
DONE:
	r = PAM_end(pamh, r);
	if (r != PAM_SUCCESS) {
		debug("pam_end failed = %d(%s)", r, PAM_strerror(pamh, r));
	}
	return res;
}

#ifdef STANDALONE

/*
 *     run
 *   gcc -o wspam -DSTANDALONE wsmand_pam.c -lpam
 *     to build a program
 */

int main(int argc, char **argv)
{
	char *user;
	char *pwd;
	int res;

	if ((argc != 3) && (argc != 4)) {
		printf("Usage: wsmand_pam <user> <password> [<service>]\n");
		return 1;
	}

	if (argc == 4) {
		service = argv[3];
	}

	if (authorize(argv[1], argv[2])) {
		printf("Authenticated\n");
	} else {
		printf("Not Authenticated\n");
	}
	if (authorize(argv[1], argv[2])) {
		printf("Authenticated\n");
	} else {
		printf("Not Authenticated\n");
	}
	return 0;
}

#endif
