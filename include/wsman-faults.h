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

#ifndef WSMAN_FAULTS_H_
#define WSMAN_FAULTS_H_

#define FAULT_SENDER_CODE "Sender"
#define FAULT_MUSTUNDERSTAND_CODE "MustUnderstand"
#define FAULT_RECEIVER_CODE "Receiver"
#define FAULT_SENDER_CODE_NS "s:Sender"
#define FAULT_RECEIVER_CODE_NS "s:Receiver"
#define FAULT_MUSTUNDERSTAND_CODE_NS "s:MustUnderstand"

enum __WsmanFaultCodeType
{
    WSMAN_RC_OK = 0,
    WSMAN_ACCESS_DENIED,

    /** wsa:ActionNotSupported */
    WSA_ACTION_NOT_SUPPORTED,

    WSMAN_ALREADY_EXISTS,
    /** wsen:CannotProcessFilter */
    WSEN_CANNOT_PROCESS_FILTER,

    WSMAN_CANNOT_PROCESS_FILTER,
    WSMAN_CONCURRENCY,
    /**wse:DeliveryModeRequestedUnavailable */
    WSE_DELIVERY_MODE_REQUESTED_UNAVAILABLE,

    WSMAN_DELIVERY_REFUSED,
    /** wsa:DestinationUnreachable */
    WSA_DESTINATION_UNREACHABLE,

    WSMAN_ENCODING_LIMIT,
    /** wsa:EndpointUnavailable */
    WSA_ENDPOINT_UNAVAILABLE,

    WSMAN_EVENT_DELIVER_TO_UNUSABLE,

    /** wse:EventSourceUnableToProcess*/
    WSE_EVENT_SOURCE_UNABLE_TO_PROCESS,

    /** wsen:FilterDialectRequestedUnavailable */
    WSEN_FILTER_DIALECT_REQUESTED_UNAVAILABLE,

    /** wse:FilteringNotSupported */
    WSE_FILTERING_NOT_SUPPORTED,
    /** wsen:FilteringNotSupported */
    WSEN_FILTERING_NOT_SUPPORTED,
    /** wse:FilteringRequestedUnavailable */
    WSE_FILTERING_REQUESTED_UNAVAILABLE,

    WSMAN_FRAGMENT_DIALECT_NOT_SUPPORTED,
    WSMAN_INTERNAL_ERROR,
    WSMAN_INVALID_BOOKMARK,

    // WSEN
    WSEN_INVALID_ENUMERATION_CONTEXT,

    /** wse:InvalidExpirationTime */
    WSE_INVALID_EXPIRATION_TIME,

    /** wsen:InvalidExpirationTime */
    WSEN_INVALID_EXPIRATION_TIME,
    /** wse:InvalidMessage */
    WSE_INVALID_MESSAGE,

    /** wsa:InvalidMessageInformationHeader */
    WSA_INVALID_MESSAGE_INFORMATION_HEADER,


    WSMAN_INVALID_OPTIONS,
    WSMAN_INVALID_PARAMETER,
    // WS-Transfer
    WXF_INVALID_REPRESENTATION,

    WSMAN_INVALID_SELECTORS,
    /** wsa:MessageInformationHeaderRequired */
    WSA_MESSAGE_INFORMATION_HEADER_REQUIRED,

    WSMAN_NO_ACK,
    WSMAN_QUOTA_LIMIT,
    WSMAN_SCHEMA_VALIDATION_ERROR,

    /** wsen:TimedOut */
    WSEN_TIMED_OUT,
    WSMAN_TIMED_OUT,

    /** wse:UnableToRenew */
    WSE_UNABLE_TO_RENEW,

    /** wse:UnsupportedExpirationType */
    WSE_UNSUPPORTED_EXPIRATION_TYPE,

    /** wsen:UnsupportedExpirationType */
    WSEN_UNSUPPORTED_EXPIRATION_TYPE,

    WSMAN_UNSUPPORTED_FEATURE,

    // SOAP
    SOAP_FAULT_VERSION_MISMATCH,
    // MustUnderstand
    SOAP_FAULT_MUSTUNDERSTAND,

    // wsmb:PolymorphismModeNotSupported
    WSMB_POLYMORPHISM_MODE_NOT_SUPPORTED,

    WSMAN_UNKNOWN
};
typedef enum  __WsmanFaultCodeType WsmanFaultCodeType;

enum __WsmanFaultDetailType
{
    WSMAN_DETAIL_OK = 0,
    WSMAN_DETAIL_ACK,
    WSMAN_DETAIL_ACTION_MISMATCH,
    WSMAN_DETAIL_ALREADY_EXISTS,
    WSMAN_DETAIL_AMBIGUOUS_SELECTORS,
    WSMAN_DETAIL_ASYNCHRONOUS_REQUEST,
    WSMAN_DETAIL_ADDRESSING_MODE,
    WSMAN_DETAIL_AUTHERIZATION_MODE,
    WSMAN_DETAIL_BOOKMARKS,
    WSMAN_DETAIL_CHARECHTER_SET,
    WSMAN_DETAIL_DELIVERY_RETRIES,
    WSMAN_DETAIL_DUPLICATE_SELECTORS,
    WSMAN_DETAIL_ENCODING_TYPE,
    WSMAN_DETAIL_ENUMERATION_MODE,
    WSMAN_DETAIL_EXPIRATION_TIME,
    WSMAN_DETAIL_EXPIRED,
    WSMAN_DETAIL_FILTERING_REQUIRED,
    WSMAN_DETAIL_FORMAT_MISMATCH,
    WSMAN_DETAIL_FORMAT_SECURITY_TOKEN,
    WSMAN_DETAIL_FRAGMENT_LEVEL_ACCESS,
    WSMAN_DETAIL_HEARTBEATS,
    WSMAN_DETAIL_INSECURE_ADDRESS,
    WSMAN_DETAIL_INSUFFICIENT_SELECTORS,
    WSMAN_DETAIL_INVALID,
    WSMAN_DETAIL_INVALID_ADDRESS,
    WSMAN_DETAIL_INVALID_FORMAT,
    WSMAN_DETAIL_INVALID_FRAGMENT,
    WSMAN_DETAIL_INVALID_NAME,
    WSMAN_DETAIL_INVALID_NAMESPACE,
    WSMAN_DETAIL_INVALID_RESOURCEURI,
    WSMAN_DETAIL_INVALID_SELECTOR_ASSIGNMENT,
    WSMAN_DETAIL_INVALID_SYSTEM,
    WSMAN_DETAIL_INVALID_TIMEOUT,
    WSMAN_DETAIL_INVALID_VALUE,
    WSMAN_DETAIL_INVALID_VALUES,
    WSMAN_DETAIL_LOCALE,
    WSMAN_DETAIL_MAX_ELEMENTS,
    WSMAN_DETAIL_MAX_ENVELOPE_POLICY,
    WSMAN_DETAIL_MAX_ENVELOPE_SIZE,
    WSMAN_DETAIL_MAX_TIME,
    WSMAN_DETAIL_MINIMUM_ENVELOPE_LIMIT,
    WSMAN_DETAIL_MISSING_VALUES,
    WSMAN_DETAIL_NOT_SUPPORTED,
    WSMAN_DETAIL_OPERATION_TIMEOUT,
    WSMAN_DETAIL_OPTION_LIMIT,
    WSMAN_DETAIL_OPTION_SET,
    WSMAN_DETAIL_READ_ONLY,
    WSMAN_DETAIL_RESOURCE_OFFLINE,
    WSMAN_DETAIL_RENAME,
    WSMAN_DETAIL_SELECTOR_LIMIT,
    WSMAN_DETAIL_SERVICE_ENVELOPE_LIMIT,
    WSMAN_DETAIL_TARGET_ALREADY_EXISTS,
    WSMAN_DETAIL_TYPE_MISMATCH,
    WSMAN_DETAIL_UNEXPECTED_SELECTORS,
    WSMAN_DETAIL_UNREPORTABLE_SUCCESS,
    WSMAN_DETAIL_UNUSABLE_ADDRESS,
    WSMAN_DETAIL_URI_LIMIT_EXCEEDED,
    WSMAN_DETAIL_WHITESPACE,

    // WS-Addressing
    WSA_DETAIL_DUPLICATE_MESSAGE_ID,

    // SOAP
    SOAP_DETAIL_HEADER_NOT_UNDERSTOOD,

   // WS-Trust
   WST_DETAIL_UNSUPPORTED_TOKENTYPE,

   // WS_Policy
   WSP_DETAIL_INVALID_EPR,

    // OpenWSMAN
    OWSMAN_DETAIL_ENDPOINT_ERROR,
    OWSMAN_NO_DETAILS,
    OWSMAN_SYSTEM_ERROR
};
typedef enum __WsmanFaultDetailType WsmanFaultDetailType;


typedef enum {
    WSMAN_STATUS_NONE,

    /* Transport Errors */
    WSMAN_STATUS_CANCELLED                       = 1,
    WSMAN_STATUS_CANT_RESOLVE,
    WSMAN_STATUS_CANT_RESOLVE_PROXY,
    WSMAN_STATUS_CANT_CONNECT,
    WSMAN_STATUS_CANT_CONNECT_PROXY,
    WSMAN_STATUS_SSL_FAILED,
    WSMAN_STATUS_IO_ERROR,
    WSMAN_STATUS_MALFORMED,
    WSMAN_STATUS_TRY_AGAIN,

    /* HTTP Status Codes */
    WSMAN_STATUS_CONTINUE                        = 100,
    WSMAN_STATUS_SWITCHING_PROTOCOLS             = 101,
    WSMAN_STATUS_PROCESSING                      = 102, /* WebDAV */

    WSMAN_STATUS_OK                              = 200,
    WSMAN_STATUS_CREATED                         = 201,
    WSMAN_STATUS_ACCEPTED                        = 202,
    WSMAN_STATUS_NON_AUTHORITATIVE               = 203,
    WSMAN_STATUS_NO_CONTENT                      = 204,
    WSMAN_STATUS_RESET_CONTENT                   = 205,
    WSMAN_STATUS_PARTIAL_CONTENT                 = 206,
    WSMAN_STATUS_MULTI_STATUS                    = 207, /* WebDAV */

    WSMAN_STATUS_MULTIPLE_CHOICES                = 300,
    WSMAN_STATUS_MOVED_PERMANENTLY               = 301,
    WSMAN_STATUS_FOUND                           = 302,
    WSMAN_STATUS_MOVED_TEMPORARILY               = 302, /* RFC 2068 */
    WSMAN_STATUS_SEE_OTHER                       = 303,
    WSMAN_STATUS_NOT_MODIFIED                    = 304,
    WSMAN_STATUS_USE_PROXY                       = 305,
    WSMAN_STATUS_NOT_APPEARING_IN_THIS_PROTOCOL  = 306, /* (reserved) */
    WSMAN_STATUS_TEMPORARY_REDIRECT              = 307,

    WSMAN_STATUS_BAD_REQUEST                     = 400,
    WSMAN_STATUS_UNAUTHORIZED                    = 401,
    WSMAN_STATUS_PAYMENT_REQUIRED                = 402, /* (reserved) */
    WSMAN_STATUS_FORBIDDEN                       = 403,
    WSMAN_STATUS_NOT_FOUND                       = 404,
    WSMAN_STATUS_METHOD_NOT_ALLOWED              = 405,
    WSMAN_STATUS_NOT_ACCEPTABLE                  = 406,
    WSMAN_STATUS_PROXY_AUTHENTICATION_REQUIRED   = 407,
    WSMAN_STATUS_PROXY_UNAUTHORIZED              = WSMAN_STATUS_PROXY_AUTHENTICATION_REQUIRED,
    WSMAN_STATUS_REQUEST_TIMEOUT                 = 408,
    WSMAN_STATUS_CONFLICT                        = 409,
    WSMAN_STATUS_GONE                            = 410,
    WSMAN_STATUS_LENGTH_REQUIRED                 = 411,
    WSMAN_STATUS_PRECONDITION_FAILED             = 412,
    WSMAN_STATUS_REQUEST_ENTITY_TOO_LARGE        = 413,
    WSMAN_STATUS_REQUEST_URI_TOO_LONG            = 414,
    WSMAN_STATUS_UNSUPPORTED_MEDIA_TYPE          = 415,
    WSMAN_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    WSMAN_STATUS_INVALID_RANGE                   = WSMAN_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE,
    WSMAN_STATUS_EXPECTATION_FAILED              = 417,
    WSMAN_STATUS_UNPROCESSABLE_ENTITY            = 422, /* WebDAV */
    WSMAN_STATUS_LOCKED                          = 423, /* WebDAV */
    WSMAN_STATUS_FAILED_DEPENDENCY               = 424, /* WebDAV */

    WSMAN_STATUS_INTERNAL_SERVER_ERROR           = 500,
    WSMAN_STATUS_NOT_IMPLEMENTED                 = 501,
    WSMAN_STATUS_BAD_GATEWAY                     = 502,
    WSMAN_STATUS_SERVICE_UNAVAILABLE             = 503,
    WSMAN_STATUS_GATEWAY_TIMEOUT                 = 504,
    WSMAN_STATUS_HTTP_VERSION_NOT_SUPPORTED      = 505,
    WSMAN_STATUS_INSUFFICIENT_STORAGE            = 507, /* WebDAV search */
    WSMAN_STATUS_NOT_EXTENDED                    = 510  /* RFC 2774 */
} WsmanKnownStatusCode;



struct __WsmanFaultCodeTable
{
	WsmanFaultCodeType  fault_code;
        WsmanKnownStatusCode http_code;
	char*               fault_action;
	char*               subCodeNs;
	char*               code;
	char*               subCode;
	char*               reason;
};
typedef struct __WsmanFaultCodeTable WsmanFaultCodeTable;

struct __WsmanFaultDetailTable
{
	WsmanFaultDetailType  fault_code;
    char*   detail;
};
typedef struct __WsmanFaultDetailTable WsmanFaultDetailTable;


#endif /*WSMAN_FAULTS_H_*/
