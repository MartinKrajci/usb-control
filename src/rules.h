#ifndef RULES
#define RULES

#include <iostream>
#include <string>
#include <cstring>
#include <regex>

#include <sqlite3.h>
#include <getopt.h>

using namespace std;

class Database
{
    static Database *database;
    sqlite3 *db;
    bool hasAttribute = false;
    string deviceClass;
    string interfaceClass;
    string deviceSubclass;
    string interfaceSubclass;
    string deviceSublclass;
    string vendor;
    string product;
    string interfacesTotal;
    string port;
    string groupID;

    Database();
    static int callback(void *data, int argc, char **argv, char **column);
    static int checkIfGroupExistsCallback(void *data, int argc, char **argv, char **column);
    static int checkIfGroupNotExistsCallback(void *data, int argc, char **argv, char **column);
    void checkIfTwoHex(string arg);
    void checkIfFourHex(string arg);
    void checkIfNum(string arg);

    public:
        bool createRule = false;
        bool showRules = false;
        bool newGroup = false;
        string ruleID;

        static Database *getDatabase();
        void checkIfGroupExists();
        void checkIfGroupNotExists();
        void attributesCheck();
        void parseArguments(int argc, char **argv);
        void insert();
        void show();
        void remove();

};

#endif