//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2003 - 2006.
//
//  File:       OpenWsmanClient.h 
//
//  Contents:   A wrapper class for openweman
//
//----------------------------------------------------------------------------

#ifndef __OPEN_WSMAN_EXCEPTION_H
#define __OPEN_WSMAN_EXCEPTION_H

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

using namespace std;

struct _WsManClient;
typedef struct _WsManClient WsManClient; // FW declaration of struct
struct WsManClientData;	

namespace WsmanClientNamespace
{


    class Exception : public std::exception
    {
        private:
            string _what;

        public:
            Exception(const string& what_arg) throw();
            virtual ~Exception() throw();
            virtual const char *getString() const;
            virtual const char *what() const throw() { return _what.c_str(); };
    };

    // Exception thrown when transport error occurs
    // Exception thrown when transport error occurs
    class TransportException : public Exception
    {
        private:
            int transferErrorCode;
        public:
            TransportException(int error, string what = "")  throw();
            virtual ~TransportException() throw();
            virtual int GetTransportReturnCode() const throw()  {return transferErrorCode;}
    };


    // Exception thrown when http error occurs
    class HttpException : public Exception
    {
        private:
            long errorCode;
        public:
            HttpException(long error, string what = "") throw();
            virtual ~HttpException() throw(){}
            virtual long GetErrorCode() {return errorCode;}
    };

    // Exception thrown when server error occurs
    class WsmanException : public Exception
    {
        private:
            long httpCode;
            string codeValue;
            string subcodeValue;
            string reason;
            string detail;
        public:
            WsmanException(long errorC, string what = "") throw();
            ~WsmanException() throw(){}
            long GetHttpReturnCode() {return httpCode;}
            void SetWsmanFaultFields(string& faultCode, string& faultSubcode,
                    string& faultReason, string& faultDetail)
            {
                codeValue = faultCode;
                subcodeValue = faultSubcode;
                reason = faultReason;
                detail = faultDetail;
            }
            string GetFaultCode(){return codeValue;}
            string GetFaultSubcode(){return subcodeValue;}
            string GetFaultReason(){return reason;}
            string GetFaultDetail(){return detail;}
    };
}; // namespace WsmanClient
#endif
