/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        3.6.2020
* Description of file:
*
* This file is header file for exceptions.cpp
*
*/

#ifndef EXCEPTIONS
#define EXCEPTIONS

#include <string>

using namespace std;

class SocketExc : public exception 
{
    string errMsg;

    public:
        SocketExc(string errMsg);
        const char *what() const noexcept;
};

class BadArgExc : public exception 
{
    string type;
    string errMsg;

    public:
        BadArgExc(string typ, string errMs);
        const char *what() const noexcept;
};

class BadParamExc : public exception 
{
    string type;
    string errMsg;

    public:
        BadParamExc(string typ, string errMs);
        const char *what() const noexcept;
};

class DatabaseExc : public exception 
{
    string type;
    string originalError;

    public:
        DatabaseExc(string type, string originalError);
        const char *what() const noexcept;
};

class BadPrivilegesExc : public exception 
{
    string errMsg;

    public:
        BadPrivilegesExc(string errMsg);
        const char *what() const noexcept;
};

class GeneralExc : public exception 
{
    string errMsg;

    public:
        GeneralExc(string errMsg);
        const char *what() const noexcept;
};

#endif