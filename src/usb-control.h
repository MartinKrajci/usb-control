/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        3.6.2020
* Description of file:
*
* This file is header file for usb-control.cpp
*
*/

#ifndef USB_CONTROL
#define USB_CONTROL

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <regex>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>

#if __GNUC__ >= 8
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include "exceptions.h"

#define FAILED 1

using namespace std;

class InterfaceRule
{
    public:
        string interfaceClass;
        string interfaceSubclass;
        bool wasUsed = false;
};

class RulesGroup
{
    public:
        string deviceClass;
        string deviceSubclass;
        string vendor;
        string product;
        string port;
        string groupID;
        int interfacesTotal = 0;
        vector<InterfaceRule *> interfaceRules;

};

class Rule
{
    public:
        string ID;
        string deviceClass;
        string deviceSubclass;
        string vendor;
        string product;
        string interfaceClass;
        string interfaceSubclass;
        string port;
        int interfacesTotal = 0;

};

class Interface
{
    public:
        string path;
        string interfaceClass;
        string interfaceSubclass;

        void load_attributes();
};

class Device
{
        int find_last_folder(const char *path);

    public:

        string path;
        string deviceClass;
        string deviceSubclass;
        string vendor;
        string product;
        string port;
        int interfacesTotal = 0;
        int interfacesFound = 0;
        vector<Interface> interfaces;

        void load_attributes();
        void disconnect();
        void authorize();

};

class Control
{
        static Control *control;
        vector<Rule *> rules;
        vector<RulesGroup *> groups;
        bool groupFoundFlag = false;
        bool indivRuleFoundFlag = false;

        static void save_rules(char **argv);
        static int callback(void *data, int argc, char **argv, char **column);
        static int check_if_no_rules_callback(void *data, int argc, char **argv, char **column);
        static int check_if_no_groups_callback(void *data, int argc, char **argv, char **column);
        static int check_if_no_indiv_rules_callback(void *data, int argc, char **argv, char **column);
        static int parse_groups(void *data, int argc, char **argv, char **column);
        static void save_groups(char **argv);
        static void save_interface_rule(char **argv);
        bool check_for_rule(Device device, string *ID);
        bool check_for_group(Device device, string *ID);
        void clean_groups();

    public:

        void read_rules();
        void check_device(Device device);
        static Control *get_control();
};

class Netlink
{
        int sock;
        char *buffer;
        vector<Device> devices;

        int find_last_folder(char *path);
        void save_device(char *buffer);
        void save_interface(char *buffer);

    public:

        Netlink();
        void init_enviroment();
        void listen_events();
};

void at_exit();

#endif