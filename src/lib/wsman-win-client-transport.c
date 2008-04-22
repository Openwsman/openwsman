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

#ifdef _WIN32
#include <winsock2.h>
#endif
/* system */
#include <windows.h>
#include <crtdbg.h>
#include <winhttp.h>
#include <tchar.h>
#include <stdio.h>
#include <intrin.h>

#pragma intrinsic(_InterlockedExchange)

/* local */
#include "u/libu.h"
#include "wsman-types.h"
#include "wsman-client.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-debug.h"
#include "wsman-client-transport.h"

#define BUFLEN 8096
#define MAX_NUM_OF_OIDS 2
#define CLIENT_CERT_STORE "MY"
#define CERT_MAX_STR_LEN 256
#define OID_CLIENT "1.3.6.1.5.5.7.3.2"
/* kerberos auth */
#define WINHTTP_OPTION_SPN                     96
// values for WINHTTP_OPTION_SPN
#define WINHTTP_DISABLE_SPN_SERVER_PORT         0x00000000
#define WINHTTP_ENABLE_SPN_SERVER_PORT          0x00000001
#define AUTH_SCHEME_NTLM			0x00000004
/* ensure that the winhttp library is linked */
#pragma comment( lib, "winhttp.lib" )
static BOOL find_cert(const _TCHAR * oid,
		const _TCHAR * certName,
		BOOL localMachine,
		PCCERT_CONTEXT  *pCertContext,
		int* errorLast);
void wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, void* user_data);



static wchar_t *convert_to_unicode(char *str)
{
	wchar_t *unicode_str = NULL;
	if (str == NULL) {
		return NULL;
	}
	unicode_str =
		(wchar_t *) malloc((strlen(str) + 1) * sizeof(wchar_t));
	if (unicode_str) {
		if (mbstowcs(unicode_str, str, strlen(str) + 1) <= 0) {
			error("No -> Unicode: %s", str);
			u_free(unicode_str);
			return NULL;
		}
	} else {
		error("Out of memory");
	}
	return unicode_str;
}


int wsmc_transport_init(WsManClient *cl, void *arg)
{
	wchar_t *agent;
	wchar_t *proxy;
	if (cl->session_handle != NULL) {
		return 0;
	}
	agent = convert_to_unicode(wsman_transport_get_agent(cl));
	if (agent == NULL) {
		return 1;
	}
	while (InterlockedExchange(&cl->lock_session_handle, 1L));
	if (cl->session_handle != NULL) {
		cl->lock_session_handle = 0L;
		return 0;
	}
	if(!cl->proxy_data.proxy){

		cl->session_handle = WinHttpOpen(agent,
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0);
	}
	else
	{
		proxy = convert_to_unicode(cl->proxy_data.proxy);
		cl->session_handle = WinHttpOpen(agent,
				WINHTTP_ACCESS_TYPE_NAMED_PROXY,
				proxy,
				WINHTTP_NO_PROXY_BYPASS, 0);
		if (proxy)
			u_free(proxy);

	}

	cl->lock_session_handle = 0L;
	u_free(agent);
	if (cl->session_handle == NULL) {
		error("Could not open session");
	}

	return cl->session_handle ? 0 : 1;
}


void wsmc_transport_fini(WsManClient *cl)
{
	if (cl->session_handle)
		WinHttpCloseHandle(cl->session_handle);
}

void wsman_transport_close_transport(WsManClient * cl)
{
	if (cl->transport) {
		WinHttpCloseHandle((HINTERNET) cl->transport);
		cl->transport = NULL;
	}
}


static void *init_win_transport(WsManClient * cl)
{
	HINTERNET connect;
	wchar_t *host = convert_to_unicode(cl->data.hostname);

	if (host == NULL) {
		error("No host");
		return NULL;
	}
	if (cl->session_handle == NULL) {
		error("could not initialize session");
		return NULL;
	}

	connect = WinHttpConnect(cl->session_handle, host, cl->data.port, 0);
	u_free(host);
	if (connect == NULL) {
		error("could not establish connection");
		return NULL;
	}
	cl->transport = (void *) connect;
	return (void *) connect;
}


static DWORD ChooseAuthScheme(DWORD dwSupportedSchemes, int ws_auth)
{
	//  It is the server's responsibility only to accept
	//  authentication schemes that provide a sufficient
	//  level of security to protect the servers resources.
	//
	//  The client is also obligated only to use an authentication
	//  scheme that adequately protects its username and password.
	//

	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE) {
		if ((ws_auth == 0) || (ws_auth == WS_GSSNEGOTIATE_AUTH)) {
			return WINHTTP_AUTH_SCHEME_NEGOTIATE;
		}
	}
	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM) {
		if ((ws_auth == 0) || (ws_auth == WS_NTLM_AUTH)) {
			return WINHTTP_AUTH_SCHEME_NTLM;
		}
	}
	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT) {
		if ((ws_auth == 0) || (ws_auth == WS_PASS_AUTH)) {
			return WINHTTP_AUTH_SCHEME_PASSPORT;
		}
	}
	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST) {
		if ((ws_auth == 0) || (ws_auth == WS_DIGEST_AUTH)) {
			return WINHTTP_AUTH_SCHEME_DIGEST;
		}
	}
	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_BASIC) {
		if ((ws_auth == 0) || (ws_auth == WS_BASIC_AUTH)) {
			return WINHTTP_AUTH_SCHEME_BASIC;
		}
	}
	return 0;
}

static DWORD Auth2Scheme(int ws_auth)
{
	if (ws_auth == WS_GSSNEGOTIATE_AUTH) {
		return WINHTTP_AUTH_SCHEME_NEGOTIATE;
	}
	if (ws_auth == WS_NTLM_AUTH) {
		return WINHTTP_AUTH_SCHEME_NTLM;
	}
	if (ws_auth == WS_PASS_AUTH) {
		return WINHTTP_AUTH_SCHEME_PASSPORT;
	}
	if (ws_auth == WS_DIGEST_AUTH) {
		return WINHTTP_AUTH_SCHEME_DIGEST;
	}
	if (ws_auth == WS_BASIC_AUTH) {
		return WINHTTP_AUTH_SCHEME_BASIC;
	}
	return 0;
}

static int cleanup_request_data(HINTERNET request)
{

	LPSTR buffer[BUFLEN];
	DWORD dwDownloaded = 0;
	DWORD dwSize = 0;
	while (1) {
		// Verify available data.
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(request, &dwSize)) {
			error("Error %u in WinHttpQueryDataAvailable.",
					GetLastError());
			return 1;
		}
		dwSize = (dwSize > BUFLEN) ? BUFLEN : dwSize;
		if (!WinHttpReadData(request, (LPVOID) buffer,
					dwSize, &dwDownloaded)) {
			error("Error %u in WinHttpReadData.",
					GetLastError());
			return 1;
		}
		if (dwDownloaded == 0) {
			break;
		}
	}
	return 0;
}



void
wsmc_handler(WsManClient * cl, WsXmlDocH rqstDoc, void *user_data)
{
	HINTERNET connect;
	HINTERNET request = NULL;
	unsigned long flags = 0;
	char *buf = NULL;
	int errLen;
	DWORD dwStatusCode = 0;
	DWORD dwSupportedSchemes;
	DWORD dwFirstScheme;
	DWORD dwSelectedScheme;
	DWORD dwTarget;
	DWORD dwLastStatus = 0;
	DWORD dwSize = sizeof(DWORD);
	BOOL bResult = FALSE;
	BOOL bResults = FALSE;
	BOOL bDone = FALSE;
	DWORD dwDownloaded = 0;
	BOOL updated;
	int ws_auth;

	wchar_t *pwd;
	wchar_t *usr;
	int lastErr = 0;
	char *p;
	size_t len;
	u_buf_t *ubuf;
	char pszAnsi[128];
	wchar_t *pwsz;

	PCCERT_CONTEXT certificate;
	wchar_t *proxy_username;
	wchar_t *proxy_password;
	if (cl->session_handle == NULL && wsmc_transport_init(cl, NULL)) {
		error("could not initialize transport");
		lastErr = GetLastError();
		goto DONE;
	}
	if (cl->transport == NULL) {
		init_win_transport(cl);
	}
	if (cl->transport == NULL) {
		lastErr = GetLastError();
		goto DONE;
	}

	connect = (HINTERNET) cl->transport;
	if (strnicmp(cl->data.endpoint, "https", 5) == 0)
	{
		flags |= WINHTTP_FLAG_SECURE;
	}


	/* request = WinHttpOpenRequest(connect, L"POST",
	   cl->data.path, NULL,
	   WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
	   flags); */

	pwsz = convert_to_unicode(cl->data.path);
	request =
		WinHttpOpenRequest(connect, L"POST", pwsz, L"HTTP/1.1",
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
	u_free(pwsz);
	if (request == NULL) {
		dwStatusCode = 400;
		goto DONE;
	}
	snprintf(pszAnsi, 128, "Content-Type: application/soap+xml;charset=%s\r\n", cl->content_encoding);
	pwsz = convert_to_unicode(pszAnsi);
	bResult = WinHttpAddRequestHeaders(request,
			pwsz,
			-1,
			WINHTTP_ADDREQ_FLAG_ADD_IF_NEW);
	u_free(pwsz);
	if (!bResult) {
		error("can't add Content-Type header");
		dwStatusCode = 400;
		goto DONE;
	}

	ws_xml_dump_memory_enc(rqstDoc, &buf, &errLen, cl->content_encoding);
	updated = 0;
	ws_auth = wsmc_transport_get_auth_value(cl);
	if(ws_auth  == AUTH_SCHEME_NTLM)
	{
		DWORD d = WINHTTP_ENABLE_SPN_SERVER_PORT;
		bResults =WinHttpSetOption(request,
				WINHTTP_OPTION_SPN,
				(LPVOID) (&d),
				sizeof(DWORD));
		if (!bResults)
		{
			lastErr = GetLastError();
			bDone = TRUE;

		}
		bResults = FALSE;
	}
	if(cl->proxy_data.proxy_username)
	{
		proxy_username = convert_to_unicode(cl->proxy_data.proxy_username);
		bResults = WinHttpSetOption(request, WINHTTP_OPTION_PROXY_USERNAME,
				proxy_username, wcslen(proxy_username));
		u_free(proxy_username);
		if (!bResults)
		{
			lastErr = GetLastError();
			bDone = TRUE;

		}
		bResults = FALSE;
	}
	if(cl->proxy_data.proxy_password)
	{
		proxy_password = convert_to_unicode(cl->proxy_data.proxy_password);
		bResults = WinHttpSetOption(request, WINHTTP_OPTION_PROXY_PASSWORD,
				proxy_password, wcslen(proxy_password));
		u_free(proxy_password);
		if (!bResults)
		{
			lastErr = GetLastError();
			bDone = TRUE;

		}
		bResults = FALSE;
	}
	if(0==cl->authentication.verify_host || 0==cl->authentication.verify_peer)
	{
		if(0==cl->authentication.verify_host)
			flags = flags | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
		if(0==cl->authentication.verify_peer)
			flags = flags | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		bResult = WinHttpSetOption(request,WINHTTP_OPTION_SECURITY_FLAGS,(LPVOID) (&flags),sizeof(DWORD));
		if (!bResult) {
			//log the error and proceed
			error("cannot ignore server certificate");
		}
	}
	while (!bDone) {
		bResult = WinHttpSendRequest(request,
				WINHTTP_NO_ADDITIONAL_HEADERS,
				(DWORD) 0, (LPVOID) buf,
				(DWORD) errLen,
				(DWORD) errLen,
				(DWORD_PTR) NULL);
		if (bResult) {
			bResults = WinHttpReceiveResponse(request, NULL);
		}
		// Resend the request in case of
		// ERROR_WINHTTP_RESEND_REQUEST error.
		if (!bResults) {
			lastErr = GetLastError();
			if (ERROR_WINHTTP_RESEND_REQUEST == lastErr) {
				lastErr = 0;
				continue;
			}
		}
		// Check the status code.
		if (bResults) {
			bResults = WinHttpQueryHeaders(request,
					WINHTTP_QUERY_STATUS_CODE
					|
					WINHTTP_QUERY_FLAG_NUMBER,
					WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode,
					&dwSize, WINHTTP_NO_HEADER_INDEX);
		}
		if (!bResults) {
			if (updated) {
				bDone = TRUE;
				break;
			}
			if (ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED ==
					lastErr) {
				lastErr = 0;

				if (!find_cert(	(const _TCHAR *)  cl->authentication.caoid,
							(const _TCHAR *)  cl->authentication.cainfo,
							cl->authentication.calocal, &certificate, &lastErr)) {
					debug("No certificate");

					bDone = TRUE;
					break;
				}

				bResults = WinHttpSetOption(request,
						WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
						(LPVOID)
						certificate,
						(DWORD) (sizeof
							(CERT_CONTEXT)));
				if (!bResults) {
					lastErr = GetLastError();
					bDone = TRUE;
					break;
				} else {
					bResults = 0;
					updated = 1;
					continue;
				}
			} else {

				bResults = WinHttpQueryAuthSchemes(request,
						&dwSupportedSchemes,
						&dwFirstScheme,
						&dwTarget);

				// Set the credentials before resending the request.
				if (bResults) {
					if (WS_MAX_AUTH == ws_auth || !cl->data.user ||
							!cl->data.pwd) {
						// we don't have credentials
						bDone = TRUE;
						bResults = 0;
						break;
					}
					dwSelectedScheme =
						ChooseAuthScheme(dwSupportedSchemes,
								ws_auth);

					if (dwSelectedScheme == 0) {
						bDone = TRUE;
						bResults = 0;
						break;
					}
					pwd = convert_to_unicode(cl->data.pwd);
					usr = convert_to_unicode(cl->data.user);
					if ((pwd == NULL) || (usr == NULL)) {
						bDone = TRUE;
						bResults = 0;
						break;
					}
					bResults = WinHttpSetCredentials(request,
							dwTarget,
							dwSelectedScheme,
							usr, pwd,
							NULL);
					u_free(pwd);
					u_free(usr);
				}
				if (cleanup_request_data(request)) {
					// the problems to read data
					bDone = TRUE;
					break;
				}
				lastErr = 0;
				bResults = 0;
				updated = 1;
				continue;
			}
		}
		switch (dwStatusCode) {
		case 200:
			// The resource was successfully retrieved.
			// You can use WinHttpReadData to read the
			// contents of the server's response.
			bDone = TRUE;
			break;
		case 400:
			debug("Error. Status code %d returned.",
					dwStatusCode);
			bDone = TRUE;
			break;
		case 500:
			debug("Error. Status code %d returned.",
					dwStatusCode);
			bDone = TRUE;
			break;
		case 401:
			// The server requires authentication.
			// Obtain the supported and preferred schemes.

			// If the same credentials are requested twice, abort the
			// request.
			if (dwLastStatus == 401) {
				bResults = 0;
				bDone = TRUE;
				break;
			}
			if(ws_auth  == AUTH_SCHEME_NTLM)
			{
				DWORD data = WINHTTP_AUTOLOGON_SECURITY_LEVEL_MEDIUM;
				DWORD dataSize = sizeof(DWORD);
				bResults = WinHttpSetOption(
						request,
						WINHTTP_OPTION_AUTOLOGON_POLICY,
						&data,
						dataSize);
				if (!bResults)
				{
					lastErr = GetLastError();
					bDone = TRUE;

				}
				if (cleanup_request_data(request)) {
					// the problems to read data
					bDone = TRUE;
				}
				break;
			}

			bResults = WinHttpQueryAuthSchemes(request,
					&dwSupportedSchemes,
					&dwFirstScheme,
					&dwTarget);

			// Set the credentials before resending the request.
			if (bResults) {
				if (WS_MAX_AUTH == ws_auth || !cl->data.user ||
						!cl->data.pwd) {
					// we don't have credentials
					bDone = TRUE;
					bResults = 0;
					break;
				}
				dwSelectedScheme =
					ChooseAuthScheme(dwSupportedSchemes,
							ws_auth);

				if (dwSelectedScheme == 0) {
					bDone = TRUE;
					bResults = 0;
					break;
				}
				pwd = convert_to_unicode(cl->data.pwd);
				usr = convert_to_unicode(cl->data.user);
				if ((pwd == NULL) || (usr == NULL)) {
					bDone = TRUE;
					bResults = 0;
					break;
				}
				bResults = WinHttpSetCredentials(request,
						dwTarget,
						dwSelectedScheme,
						usr, pwd,
						NULL);
				u_free(pwd);
				u_free(usr);
			}
			if (cleanup_request_data(request)) {
				// the problems to read data
				bDone = TRUE;
			}
			break;
			/*
			   case 407:
			// The proxy requires authentication.

			// Obtain the supported and preferred schemes.
			bResults = WinHttpQueryAuthSchemes( hRequest,
			&dwSupportedSchemes,
			&dwFirstScheme,
			&dwTarget );

			// Set the credentials before resending the request.
			if( bResults )
			dwProxyAuthScheme = ChooseAuthScheme(dwSupportedSchemes);

			// If the same credentials are requested twice, abort the
			// request.  For simplicity, this sample does not check
			// for a repeated sequence of status codes.
			if( dwLastStatus == 407 )
			bDone = TRUE;
			break;
			*/
		default:
			// The status code does not indicate success.
			debug("Error. Status code %d returned.",
					dwStatusCode);
			bResults = 0;
			bDone = TRUE;
		}

		// Keep track of the last status code.
		dwLastStatus = dwStatusCode;

		// If there are any errors, break out of the loop.
		if (!bResults) {
			bDone = TRUE;
		}
	}			// while

	// Read data
	if (!bResults) {
		goto DONE;
	}

	ubuf = cl->connection->response;
	while (1) {
		// Verify available data.
		dwSize = 0;
		bResults = WinHttpQueryDataAvailable(request, &dwSize);
		if (!bResults) {
			lastErr = GetLastError();
			error("Error %u in WinHttpQueryDataAvailable.",
					lastErr);
			break;
		}
		if (dwSize > 0) {
			u_buf_append(ubuf, NULL, (size_t) dwSize);
		}
		p = (char *) u_buf_ptr(ubuf);
		len = u_buf_len(ubuf);
		bResults = WinHttpReadData(request,
				(LPVOID) (p + len), dwSize,
				&dwDownloaded);
		if (!bResults) {
			lastErr = GetLastError();
			error("Error %u in WinHttpReadData.", lastErr);
			break;
		}
		if (dwDownloaded == 0) {
			// end of data
			break;
		}
		u_buf_set_len(ubuf, len + dwDownloaded);
	}
DONE:
	cl->response_code = dwStatusCode;
	cl->last_error = lastErr;
	ws_xml_free_memory(buf);
	if (request) {
		WinHttpCloseHandle(request);
	}
}

// in future change this to return a list of certs...
BOOL find_cert(const _TCHAR * oid,
		const _TCHAR * certName,
		BOOL localMachine,
		PCCERT_CONTEXT  *pCertContext,
		int* errorLast)
{

	_TCHAR pszNameString[CERT_MAX_STR_LEN];
	HANDLE hStoreHandle = NULL;
	LPSTR oids[MAX_NUM_OF_OIDS] = {OID_CLIENT,oid};
	BOOL certSuccess = FALSE;
	CERT_ENHKEY_USAGE od ={ oid ? 2 : 1, oids };
	int lastErr =0;

	/* Choose which personal store to use : Current User or Local Machine*/
	DWORD flags;
	if (localMachine)
	{
		flags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
	}
	else
	{
		flags = CERT_SYSTEM_STORE_CURRENT_USER;
	}

	/* Open the personal store */
	if ( !(hStoreHandle = CertOpenStore(
					CERT_STORE_PROV_SYSTEM,          // The store provider type
					0,                               // The encoding type is not needed
					NULL,                            // Use the default HCRYPTPROV
					flags,  // Set the store location in a registry location
					L"MY"                            // The store name as a Unicode string
					)))
	{
		/* Cannot open the certificates store - exit immediately */
		lastErr = GetLastError();
		error("error %d in CertOpenStore", lastErr);
		return TRUE;
	}

	// Search for the first client certificate

	*pCertContext = CertFindCertificateInStore(
			hStoreHandle,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
			CERT_FIND_ENHKEY_USAGE,
			&od,
			NULL);

	/*
	   If certificate was found - determinate its name. Keep search
	   while the certificate's Common Name doesn't match the name
	   defined by the user
	   */
	while (*pCertContext != NULL) {
		if (!CertGetNameString(*pCertContext,
					CERT_NAME_SIMPLE_DISPLAY_TYPE,
					0,
					NULL,
					pszNameString,
					CERT_MAX_STR_LEN - 1)) {
			/* obtaining certificate name failed */
			lastErr = GetLastError();
			error("error %d in CertGetNameString.", lastErr);
			break;
		}

		if (certName == NULL ||
				_tcscmp(pszNameString, certName) == 0) {
			certSuccess = TRUE;
			break;
		}

		*pCertContext =
			CertFindCertificateInStore(hStoreHandle,
					X509_ASN_ENCODING |
					PKCS_7_ASN_ENCODING,
					CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
					CERT_FIND_ENHKEY_USAGE,
					&od, *pCertContext);

	}

	if (*pCertContext == NULL) {
		lastErr = GetLastError();
		error("error %d (%s) in CertFindCertificateInStore.",
				lastErr);
	}
	if (errorLast) {
		*errorLast = lastErr;
	}
	return certSuccess;
}
