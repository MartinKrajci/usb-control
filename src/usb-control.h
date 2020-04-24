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
#include <filesystem>
#include <cstdlib>

#define FAILED 1

using namespace std;
namespace fs = std::filesystem;

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

        void loadAttributes();
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

        void loadAttributes();
        void disconnect();
        void authorize();

};

class Control
{
        static Control *control;
        vector<Rule *> rules;
        vector<RulesGroup *> groups;
        bool GroupFoundFlag = false;

        static void save_rules(char **argv);
        static int callback(void *data, int argc, char **argv, char **column);
        static int checkIfNoRules(void *data, int argc, char **argv, char **column);
        static int checkIfNoGroups(void *data, int argc, char **argv, char **column);
        static int parse_groups(void *data, int argc, char **argv, char **column);
        static void save_groups(char **argv);
        static void save_interface_rule(char **argv);
        bool check_for_rule(Device device);
        bool check_for_group(Device device);
        void clean_groups();

    public:

        void read_rules();
        void check_device(Device device);
        static Control *getControl();
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