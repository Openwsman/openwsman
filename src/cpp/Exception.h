//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2007.
//
//  File:       Exception.h
//
//  Contents:   General Wsman Exception definition
//
//----------------------------------------------------------------------------

#ifndef __WSMAN_EXCEPTION_H
#define __WSMAN_EXCEPTION_H

#include <string>
#include <stdexcept>

using namespace std;
// Error Codes
#define WSMAN_SUCCESS 0
// A general error occurred
#define WSMAN_GENERAL_ERROR 1
// A transport level error occurred
#define WSMAN_TRANSPORT_ERROR 2
// An HTTP error occurred
#define WSMAN_HTTP_ERROR 3
// The WS-MAN server returned a soap fault
#define WSMAN_SOAP_FAULT 4
// A string to type or type to string error occurred
#define WSMAN_TYPE_CONVERSION_ERROR 5
// Failed to parse or write an XML string
#define WSMAN_XML_ERROR 6
// Input information is missing
#define WSMAN_MISSING_INPUT 7
// An unexpected response was received
#define WSMAN_RESPONSE_UNKNOWN 8
// Error resulting from MustUnderstand attribute
#define WSMAN_MUST_UNDERSTAND 9
// The soap message is invalid or of invalid version
#define WSMAN_SOAP_MESSAGE_INVALID 10
// The Soap header is too long
#define WSMAN_SOAP_HDR_OVERFLOW 11
// a UDP error occurred
#define WSMAN_UDP_ERROR 12
// a TCP error occurred
#define WSMAN_TCP_ERROR 13
// failed to connect to server
#define WSMAN_CONNECT_ERROR 14

namespace WsmanExceptionNamespace
{
	class GeneralWsmanException : public exception
	{
	private:
		string _what;
		unsigned int error;

	public:
		GeneralWsmanException(const char* what,
			unsigned int err = WSMAN_GENERAL_ERROR)
			:_what(what), error(err){}
		virtual ~GeneralWsmanException() throw (){}
		virtual string getDetail()
		{
			return _what;
		}
		virtual const char *what() const throw()
		{
		  return _what.c_str();
		};
		virtual unsigned int getErr() const throw()
		{
			return error;
		}
		 string getStrErr()
                {
                        string res = "Unknown error";
                        switch(error)
                        {
                        	case WSMAN_SUCCESS:
					res = "Operation succeeded";
					break;
				case WSMAN_GENERAL_ERROR:
					res = "General error occurred";
					break;
				case WSMAN_TRANSPORT_ERROR:
					res = "Transport error occurred";
					break;
				case WSMAN_HTTP_ERROR:
					res = "HTTP error occurred";
					break;
				case WSMAN_SOAP_FAULT:
					res = "SOAP error occurred";
					break;
				case WSMAN_TYPE_CONVERSION_ERROR:
					res = "Conversion type error occurred";
					break;
				case WSMAN_XML_ERROR:
					res = "XML error occurred";
					break;
				case WSMAN_MISSING_INPUT:
					res = "Missing input error occurred";
					break;
				case WSMAN_RESPONSE_UNKNOWN:
					res = "Response unknown";
					break;
				case WSMAN_MUST_UNDERSTAND:
					res = "Must understand error";
					break;
				case WSMAN_SOAP_MESSAGE_INVALID:
					res = "SOAP message invalid";
					break;
				case WSMAN_SOAP_HDR_OVERFLOW:
					res = "Overflow soap header";
					break;
				case WSMAN_UDP_ERROR:
					res = "UDP error occurred";
					break;
				case WSMAN_TCP_ERROR:
					res = "TCP error occurred";
					break;
				case WSMAN_CONNECT_ERROR:
					res = "Connection error occurred";
					break;
                        }
                        return res;
                }

	};
}
#endif
