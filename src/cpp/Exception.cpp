

#include  <Exception.h>

namespace WsmanClientNamespace {


	Exception::Exception(const string& what_arg) throw():
		_what(what_arg)
	{ }
	Exception::~Exception() throw()
	{}

	const char *Exception::getString() const
	{
		return _what.c_str();
	}

	TransportException::TransportException(int error, string what) throw():
		Exception(what), transferErrorCode(error) 
	{}
	TransportException::~TransportException() throw() {}


	HttpException::HttpException(long error, string what) throw():
		Exception(what), errorCode(error) 
	{}
	HttpException::~HttpException() throw();

	WsmanException::WsmanException(long error, string what) throw():
		Exception(what.c_str()), httpCode(error), codeValue(""),
		subcodeValue(""), reason(""), detail(""){}
	//WsmanException::~WsmanException() throw() {}

}
