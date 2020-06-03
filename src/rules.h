/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        3.6.2020
* Description of file:
*
* This file is header file for rules.cpp
*
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
    static int check_if_group_exists_callback(void *data, int argc, char **argv, char **column);
    static int check_if_group_not_exists_callback(void *data, int argc, char **argv, char **column);
    static int find_group_ID(void *data, int argc, char **argv, char **column);
    void check_if_two_hex(string arg);
    void check_if_four_hex(string arg);
    void check_if_num(string arg);
    void check_if_port(string arg);
    void load_interface_attributes(string path);
    void load_device_attributes(string path);
    void clear_device_attributes();
    void clear_interface_attributes();
    int find_last_folder(const char *path);
    void hex_to_lower(char* chars);

    public:
        bool createRule = false;
        bool showRules = false;
        bool deleteRule = false;
        bool newGroup = false;
        bool defaultRules = false;
        bool deleteAllRules = false;
        vector<string> ruleIDs;

        static Database *getDatabase();
        void checkIfGroupExists();
        void check_if_group_not_exists();
        void attributes_check();
        void parse_arguments(int argc, char **argv);
        void insert();
        void show();
        void remove();
        int set_default_rules();
        void init();

};

#endif