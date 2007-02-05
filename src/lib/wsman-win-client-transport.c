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
#include "wsman-errors.h"
#include "wsman-xml.h"
#include "wsman-debug.h"
#include "wsman-client-transport.h"

#define BUFLEN 8096
#define NUM_OF_OIDS 2
#define CLIENT_CERT_STORE "MY"
#define CERT_MAX_STR_LEN 256
#define OID_CLIENT "1.3.6.1.5.5.7.3.2"
#define  OID_REMOTE  "2.16.840.1.113741.1.2.1"
/* kerberos auth */
#define WINHTTP_OPTION_SPN                           96
// values for WINHTTP_OPTION_SPN
#define WINHTTP_DISABLE_SPN_SERVER_PORT           0x00000000
#define WINHTTP_ENABLE_SPN_SERVER_PORT            0x00000001
/* ensure that the winhttp library is linked */
#pragma comment( lib, "winhttp.lib" )
static HINTERNET session;
extern ws_auth_request_func_t request_func;
static BOOL find_cert(const _TCHAR * certName,
          PCCERT_CONTEXT  *pCertContext, int* errorLast);
void wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, void* user_data);



static wchar_t * 
convert_to_unicode(char * str)
{
	wchar_t *unicode_str =NULL;
    if (str == NULL) {
        return NULL;
    }
    unicode_str = (wchar_t *)malloc((strlen(str)+1)* sizeof( wchar_t ));
    if (unicode_str) {
        if (mbstowcs(unicode_str, str, strlen(str)+1) <= 0) {
            error("No -> Unicode: %s", str);
            u_free(unicode_str);
            return NULL;
        }
    } else {
        error("Out of memory");
    }
    return unicode_str;
}


int wsman_client_transport_init(void *arg)
{
    wchar_t *agent;
	static long lock;

	if (session != NULL) {
		return 0;
	}
    agent = convert_to_unicode(
                        wsman_transport_get_agent());
    if (agent == NULL) {
        return 1;
    }
	while (InterlockedExchange(&lock, 1L));
	if (session != NULL) {
		lock = 0L;
		return 0;
	}
	session = WinHttpOpen(agent,
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, 
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );
	lock = 0L;
	u_free(agent);
	if (session == NULL) {
		error("Could not open session");
	}

	return session ? 0 : 1;
}


void wsman_client_transport_fini()
{
	if (session) 
		WinHttpCloseHandle(session);
}

void wsman_transport_close_transport(WsManClient *cl)
{
    if (cl->transport) {
        WinHttpCloseHandle((HINTERNET)cl->transport);
        cl->transport = NULL;
    }
}


static void *
init_win_transport(WsManClient *cl)
{
    HINTERNET connect;
    wchar_t *host = convert_to_unicode(cl->data.hostName);

    if (host == NULL) {
		error("No host");
        return NULL;
    }
	if (session == NULL) {
		error("could not initialize session");
		return NULL;
	}

    connect = WinHttpConnect(session,
                             host,
                             cl->data.port,
                             0);
    u_free(host);
    if (connect == NULL) {
        error("could not establish connection");
        return NULL;
    }
    cl->transport = (void *)connect;
	return (void *)connect;
}


static DWORD
ChooseAuthScheme(DWORD dwSupportedSchemes, int ws_auth)
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

static int
cleanup_request_data(HINTERNET request)
{

	LPSTR buffer[BUFLEN];
	DWORD dwDownloaded = 0;
	DWORD dwSize = 0;
	while (1) {
		// Verify available data.
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(request, &dwSize)) {
			error( "Error %u in WinHttpQueryDataAvailable.",
				GetLastError());
			return 1;
		}
		dwSize = (dwSize > BUFLEN) ? BUFLEN : dwSize;
		if (!WinHttpReadData(request, (LPVOID)buffer,
			dwSize, &dwDownloaded))
		{
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
wsman_client_handler( WsManClient *cl,
                      WsXmlDocH rqstDoc,
                      void* user_data)
{
    HINTERNET connect;
    HINTERNET request = NULL;
    unsigned long flags = 0;
    char *buf = NULL;
    int errLen;
    int ws_auth;
    DWORD dwStatusCode = 0;
    DWORD dwSupportedSchemes;
    DWORD dwFirstScheme;
    DWORD dwSelectedScheme;
    DWORD dwTarget;
    DWORD dwLastStatus = 0;
    DWORD dwSize = sizeof(DWORD);
    BOOL  bResult = FALSE;
	BOOL  bResults = FALSE;
    BOOL  bDone = FALSE;
	DWORD dwDownloaded = 0;
	BOOL updated;

    wchar_t *pwd;
    wchar_t *usr;
    int lastErr =0;
    char *p;
    size_t len;
	u_buf_t *ubuf;
	PCCERT_CONTEXT      certificate;

	if (session == NULL && wsman_client_transport_init(NULL)) {
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

    connect = (HINTERNET)cl->transport;
	if(strnicmp(cl->data.endpoint, "https", 5) == 0)
	//if (wsman_transport_get_cafile() != NULL) 
	{
        flags |= WINHTTP_FLAG_SECURE;
    }

	
   /* request = WinHttpOpenRequest(connect, L"POST",
                       cl->data.path, NULL, 
                       WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 
                       flags);*/
    

	request = WinHttpOpenRequest(
		connect, L"POST", L"wsman", L"HTTP/1.1", 
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 
		flags );

    if (request == NULL) {
        dwStatusCode = 400;
        goto DONE;
    }
    bResult = WinHttpAddRequestHeaders(request, 
            L"Content-Type: application/soap+xml;charset=UTF-8\r\n",
            -1, WINHTTP_ADDREQ_FLAG_ADD_IF_NEW );
    if (!bResult) {
        error("can't add Content-Type header");
        dwStatusCode = 400;
        goto DONE;
    }

	ws_xml_dump_memory_enc(rqstDoc, &buf, &errLen, "UTF-8");
	updated = 0;
    ws_auth = ws_client_transport_get_auth_value();
	if(ws_auth == WS_NTLM_AUTH)
	{
		DWORD d = WINHTTP_ENABLE_SPN_SERVER_PORT;
		if(!WinHttpSetOption(
			request,
			WINHTTP_OPTION_SPN, 
			(LPVOID) (&d),
			sizeof(DWORD)))
		{
			lastErr = GetLastError();
			goto DONE;
		}

	}
    while (!bDone) {
        bResult = WinHttpSendRequest(request,
                        WINHTTP_NO_ADDITIONAL_HEADERS, (DWORD)0,
                        (LPVOID)buf, (DWORD)errLen, (DWORD)errLen,
						(DWORD_PTR)NULL);
        if(bResult) {
             bResults = WinHttpReceiveResponse(request, NULL);
        }

        // Resend the request in case of 
        // ERROR_WINHTTP_RESEND_REQUEST error.
        if (!bResults)
		{
			lastErr = GetLastError();
			if(ERROR_WINHTTP_RESEND_REQUEST ==lastErr) {
				lastErr = 0;
				continue;
			}
        }

        // Check the status code.
        if (bResults) {
             bResults = WinHttpQueryHeaders(request, 
                                      WINHTTP_QUERY_STATUS_CODE |
                                      WINHTTP_QUERY_FLAG_NUMBER,
                                      NULL, 
                                      &dwStatusCode, 
                                      &dwSize, 
                                      NULL );
        }
        if(!bResults )
		{
			if(updated)
			{
				bDone = TRUE;
				break;
			}
			if( ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED == lastErr)
			{
				lastErr = 0;
				if (!find_cert((const _TCHAR *) wsman_transport_get_cafile(), &certificate, &lastErr)) {
                    debug("No certificate");

                    bDone = TRUE;
                    break;
                }
				
                bResults = WinHttpSetOption(request,
                                WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
                                (LPVOID)certificate,
								(DWORD)(sizeof (CERT_CONTEXT)));
                if (!bResults) { 
					lastErr = GetLastError();
                     bDone = TRUE;
					 break;
                }
				else
				{	
					bResults = 0;
					updated = 1;
					continue;
				}			
			}
			else
			{
				break;
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
			    debug("Error. Status code %d returned.", dwStatusCode);
                bDone = TRUE;
               break;
			 case 500: 
				debug("Error. Status code %d returned.", dwStatusCode);
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
               bResults = WinHttpQueryAuthSchemes(request, 
                                              &dwSupportedSchemes, 
                                              &dwFirstScheme, 
                                              &dwTarget );

               // Set the credentials before resending the request.
               if (bResults) {
                  if (!ws_auth || !cl->data.user || !cl->data.pwd) {
                      // we don't have credentials
                      bDone = TRUE;
					  bResults = 0;
                      break;
                  }
                  dwSelectedScheme = ChooseAuthScheme(dwSupportedSchemes,
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
                                        usr, pwd, NULL);
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
                debug("Error. Status code %d returned.", dwStatusCode);
				bResults = 0;
                bDone = TRUE;
       }

       // Keep track of the last status code.
       dwLastStatus = dwStatusCode;

       // If there are any errors, break out of the loop.
       if (!bResults) { 
           bDone = TRUE;
       }
    }     // while 

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
            error("Error %u in WinHttpQueryDataAvailable.", lastErr);
            break;
        }
        if (dwSize > 0) {
            u_buf_append(ubuf, NULL, (size_t)dwSize);
        }
        p = (char*)u_buf_ptr(ubuf);
        len = u_buf_len(ubuf);
        bResults = WinHttpReadData(request,
                        (LPVOID)(p + len), dwSize, &dwDownloaded);
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


static BOOL find_cert(const _TCHAR * certName,
          PCCERT_CONTEXT  *pCertContext, int* errorLast)
{

    _TCHAR pszNameString[CERT_MAX_STR_LEN];
    HANDLE hStoreHandle = NULL;
	LPSTR oids[NUM_OF_OIDS] = {OID_CLIENT,OID_REMOTE};
	BOOL certSuccess = FALSE;
	CERT_ENHKEY_USAGE amtOID ={ NUM_OF_OIDS, oids };
	int lastErr =0;
	////oids[0] = OID_CLIENT;
   // oids[1] = wsman_transport_get_cafile();
   // amtOID = { NUM_OF_OIDS, oids };
   

    if (!(hStoreHandle = CertOpenSystemStore((HCRYPTPROV)NULL, "MY"))) {
         /* Cannot open the certificates store - exit immediately */
		lastErr = GetLastError();
         error("error %d in CertOpenSystemStore", lastErr);
         return TRUE;
    }
    // Search for the first client certificate

    *pCertContext = CertFindCertificateInStore(
        hStoreHandle,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
        CERT_FIND_ENHKEY_USAGE,
        &amtOID,
        NULL);

    /*
    If certificate was found - determinate its name. Keep search 
    while the certificate's Common Name doesn't match the name
    defined by the user
    */
    while (*pCertContext != NULL) {
        if(!CertGetNameString(*pCertContext,
                              CERT_NAME_SIMPLE_DISPLAY_TYPE,
                              0,
                              NULL,
                              pszNameString,
                              CERT_MAX_STR_LEN-1)) {
            /* obtaining certificate name failed */
			lastErr = GetLastError();
            error("error %d in CertGetNameString.", lastErr);
            break;
        }

        if (certName == NULL || _tcscmp(pszNameString,certName) == 0) {
            certSuccess = TRUE;
            break;
        }

        *pCertContext = 
            CertFindCertificateInStore(hStoreHandle,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
            CERT_FIND_ENHKEY_USAGE,
            &amtOID,
            *pCertContext);

    }

    if (*pCertContext == NULL) {
		lastErr = GetLastError();
        error("error %d (%s) in CertFindCertificateInStore.", lastErr);
    }
	if(errorLast)
	{
		*errorLast = lastErr;
	}
    return certSuccess;
}



