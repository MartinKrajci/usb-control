/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        12.5.2020
*/

#ifndef RULES
#define RULES

#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include <fstream>
#include <regex>

#if __GNUC__ >= 8
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <sqlite3.h>
#include <getopt.h>

#include "exceptions.h"

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
    string vendor;
    string product;
    string interfacesTotal;
    string port;
    string groupID;
    string nextFreeGroupID = "1";

    Database();
    static int callback(void *data, int argc, char **argv, char **column);
    static int checkIfGroupExistsCallback(void *data, int argc, char **argv, char **column);
    static int checkIfGroupNotExistsCallback(void *data, int argc, char **argv, char **column);
    static int findGroupID(void *data, int argc, char **argv, char **column);
    void checkIfTwoHex(string arg);
    void checkIfFourHex(string arg);
    void checkIfNum(string arg);
    void checkIfPort(string arg);
    void loadInterfaceAttributes(string path);
    void loadDeviceAttributes(string path);
    void clearDeviceAttributes();
    void clearInterfaceAttributes();
    int find_last_folder(const char *path);

    public:
        bool createRule = false;
        bool showRules = false;
        bool newGroup = false;
        bool defaultRules = false;
        vector<string> ruleIDs;

        static Database *getDatabase();
        void checkIfGroupExists();
        void checkIfGroupNotExists();
        void attributesCheck();
        void parseArguments(int argc, char **argv);
        void insert();
        void show();
        void remove();
        void setDefaultRules();
        void init();

};

#endif