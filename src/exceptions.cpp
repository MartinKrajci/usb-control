#include "exceptions.h"

/*
* Exception constructor. Thrown when any socket error occured.
*/
SocketExc::SocketExc(string errMsg)
{
    this->errMsg = errMsg;
}

/*
* Derived function from Exception class, returns message describing exception.
*/
const char * SocketExc::what() const noexcept
{
    return errMsg.c_str();
}

/*
* Exception constructor. Thrown when user entered bad argument for parameter.
*/
BadArgExc::BadArgExc(string type, string errMsg)
{
    this->type = type;
    this->errMsg = errMsg;
}

/*
* Derived function from Exception class, returns message describing exception.
*/
const char * BadArgExc::what() const noexcept
{
    return errMsg.c_str();
}

/*
* Exception constructor. Thrown when user entered bad parameter.
*/
BadParamExc::BadParamExc(string type, string errMsg)
{
    this->type = type;
    this->errMsg = errMsg;
}

/*
* Derived function from Exception class, returns message describing exception.
*/
const char * BadParamExc::what() const noexcept
{
    return errMsg.c_str();
}

/*
* Exception constructor. Thrown when function from .
*/
DatabaseExc::DatabaseExc(string type, string originalError)
{
    this->type = type;
    this->originalError = originalError;
}

/*
* Derived function from Exception class, returns message describing exception.
*/
const char * DatabaseExc::what() const noexcept
{
    return originalError.c_str();
}

/*
* Exception constructor. Thrown when user do not have root privileges.
*/
BadPrivilegesExc::BadPrivilegesExc(string errMsg)
{
    this->errMsg = errMsg;
}

/*
* Derived function from Exception class, returns message describing exception.
*/
const char * BadPrivilegesExc::what() const noexcept
{
    return errMsg.c_str();
}

/*
* Exception constructor. Thrown when exception not belonging into any classes above occured.
*/
GeneralExc::GeneralExc(string errMsg)
{
    this->errMsg = errMsg;
}

/*
* Derived function from Exception class, returns message describing exception.
*/
const char * GeneralExc::what() const noexcept
{
    return errMsg.c_str();
}