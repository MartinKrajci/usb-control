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

class Contol;

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

    void loadAttributes()
    {
        ifstream fInterfaceClass;
        ifstream fInterfaceSubclass;
        fInterfaceClass.open(path + "/bInterfaceClass");
        fInterfaceSubclass.open(path + "/bInterfaceSubClass");
        getline(fInterfaceClass, interfaceClass);
        getline(fInterfaceSubclass, interfaceSubclass);
    }
};

class Device
{
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

        void loadAttributes()
        {
            ifstream fDeviceClass;
            ifstream fDeviceSubclass;
            ifstream fVendor;
            ifstream fProduct;
            ifstream fPort;
            fDeviceClass.open(path + "/bDeviceClass");
            fDeviceSubclass.open(path + "/bDeviceSubClass");
            fVendor.open(path + "/idVendor");
            fProduct.open(path + "/idProduct");
            fPort.open(path + "/devpath");
            getline(fDeviceClass, deviceClass);
            getline(fDeviceSubclass, deviceSubclass);
            getline(fVendor, vendor);
            getline(fProduct, product);
            getline(fPort, port);
            fDeviceClass.close();
            fDeviceSubclass.close();
            fVendor.close();
            fProduct.close();
            fPort.close();
        }

        void disconnect()
        {
            ofstream fAuthorized;
            fAuthorized.open(path + "/authorized");
            fAuthorized << "0";
            fAuthorized.close();
        }

        int find_last_folder(const char *path)
        {
            int i = strlen(path);
            for(; path[i] != '/'; i--);
            return i + 1;
        }

        void authorize()
        {
            vector<Interface>::iterator interface;
            int fdDriversProbe = 0;
            FILE *fDriversProbe ;

            bool test = false;
            for (interface = interfaces.begin(); interface < (interfaces.end()); interface++)
            {
                    ofstream fAuthorized;
                    fAuthorized.open(interface->path + "/authorized");
                    fAuthorized << "1";

                    if(fDriversProbe < 0)
                    {
                        cerr << "Could not open file drivers_probe.";
                    }

                    fDriversProbe = popen("sudo tee /sys/bus/usb/drivers_probe", "w");
                    fdDriversProbe = fileno(fDriversProbe);
                    
                    write(fdDriversProbe, find_last_folder(interface->path.c_str()) + interface->path.c_str(), strlen(find_last_folder(interface->path.c_str()) + interface->path.c_str()));
                    close(fdDriversProbe);
            }
        }

};

class Control
{
    public:
        vector<Rule *> rules;

        void save_rules(int argc, char **argv, Control *con)
        {
            int i = 0;
            
            Rule *rule = new Rule;

            rule->deviceClass = (argv[1] ? argv[1] : "NULL");
            rule->deviceSubclass = (argv[2] ? argv[2] : "NULL");
            rule->vendor = (argv[3] ? argv[3] : "NULL");
            rule->product = (argv[4] ? argv[4] : "NULL");
            rule->interfaceClass = (argv[5] ? argv[5] : "NULL");
            rule->interfaceSubclass = (argv[6] ? argv[6] : "NULL");
            rule->interfacesTotal = (argv[7] ? stoi(argv[7]) : 999);
            rule->port = (argv[8] ? argv[8] : "NULL");
            con->rules.push_back(rule);

        }

        static int callback(void *con, int argc, char **argv, char **azColName)
        {
            /*for(int i = 0; i<argc; i++)
            {
                printf("%s = %s | ", azColName[i], argv[i] ? argv[i] : "NULL");

            }
            printf("\n");*/

            ((Control *)con)->save_rules(argc, argv, (Control *) con);
            return 0;
        }

        void read_rules(Control *con)
        {
            int rc;
            sqlite3 *db;
            string SQLSelect = "SELECT * FROM RULE";
            char *errmsg = NULL;

            rc = sqlite3_open("db", &db);
            if (rc) 
            {
                cerr << "Could not open database\n";
                exit(1);
            }

            rc = sqlite3_exec(db, SQLSelect.c_str(), callback, (void *) con, &errmsg);
            if (rc)
            {
                cerr << "Could not execute command, " << errmsg << "\n";
                exit(1);
            }

        }

        void check_device(Device device)
        {
            vector<Rule *>::iterator rule;

            for(int currentInterface = 0; currentInterface < device.interfacesTotal; currentInterface++)
            {
                for (rule = rules.begin(); rule < (rules.end()); rule++)
                {
                    if(device.deviceClass != (*rule)->deviceClass && (*rule)->deviceClass != "NULL")
                    {
                        continue;
                    }
                    else if(device.deviceSubclass != (*rule)->deviceSubclass && (*rule)->deviceSubclass != "NULL")
                    {
                        continue;
                    }
                    else if(device.vendor != (*rule)->vendor && (*rule)->vendor != "NULL")
                    {
                        continue;
                    }
                    else if(device.product != (*rule)->product && (*rule)->product != "NULL")
                    {
                        continue;
                    }
                    else if(device.interfaces[currentInterface].interfaceClass != (*rule)->interfaceClass && (*rule)->interfaceClass != "NULL")
                    {
                        continue;
                    }
                    else if(device.interfaces[currentInterface].interfaceSubclass != (*rule)->interfaceSubclass && (*rule)->interfaceSubclass != "NULL")
                    {
                        continue;
                    }
                    else if(device.port != (*rule)->port && (*rule)->port != "NULL")
                    {
                        continue;
                    }
                    else if(device.interfacesTotal != (*rule)->interfacesTotal && (*rule)->interfacesTotal != 999)
                    {
                        continue;
                    }
                    else
                    {
                        if((currentInterface + 1) == device.interfacesTotal)
                        {
                            device.authorize();
                            cout << "Device authorized..\n";
                            return;
                        }
                        break;
                    }
                }
            }

            device.disconnect();
            cout << "! Disconnecting potenctialy dangerous device !\n";
            return;
        }
};

class Netlink
{
    public:
        int sock;
        int counter = 0;
        vector<Device> devices;

        Netlink()
        {
            struct sockaddr_nl addr;
            memset(&addr, 0, sizeof(struct sockaddr_nl));

            sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
            if (sock < 0)
            {
                cerr << "Unable to create netlink socket\n";
                exit(FAILED);
            }

            addr.nl_family = AF_NETLINK;
            addr.nl_pid = getpid();
            addr.nl_groups = -1;

            if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_nl)))
            {
                cerr << "Unable to bind socket\n";
                exit(FAILED);
            }

        }

        void init_enviroment()
        {
            regex usbFolder("usb\\d+");

            for(auto& file: fs::directory_iterator("/sys/bus/usb/devices"))
            {
                if(regex_match(string(fs::path(file).filename()), usbFolder))
                {
                    //ofstream authDefFile;
                    ofstream interfaceAuthDefFile;
                    //authDefFile.open(string(fs::path(file)) + "/authorized_default");
                    interfaceAuthDefFile.open(string(fs::path(file)) + "/interface_authorized_default");
                    //authDefFile << "0";
                    interfaceAuthDefFile << "0";
                    //authDefFile.close();
                    interfaceAuthDefFile.close();
                }
            }
        }

        int find_last_folder(char *path)
        {
            int i = strlen(path);
            for(; path[i] != '/'; i--);
            return i + 1;
        }

        void save_device(char *buffer)
        {
            Device device;
            ifstream numInterfacesFile;
            string numInterfaces;
            device.path = "/sys" + string(buffer + 4);
            //cout << counter << ":DEVICE: " + device.path  + "\n";
            numInterfacesFile.open(device.path + "/bNumInterfaces");
            getline(numInterfacesFile, numInterfaces);
            device.interfacesTotal = stoi(numInterfaces);
            device.loadAttributes();
            devices.push_back(device);
            counter++;
        }

        void save_interface(char *buffer, Control con)
        {
            Interface interface;
            vector<Device>::iterator device;
            for (device = devices.begin(); device < devices.end(); device++)
            {
                if(("/sys" + string(buffer + 4)).find((*device).path) != string::npos)
                {

                    interface.path = "/sys" + string(buffer + 4);
                    interface.loadAttributes();
                    (*device).interfaces.push_back(interface);
                    (*device).interfacesFound++;
                    //cout << counter << ":INTERFACE: " << (*device).interfaces.back().path << "\n";
                    counter++;

                    if((*device).interfacesFound == (*device).interfacesTotal)
                    {
                        con.check_device(*device);
                        devices.erase(device);
                    }
                }
            }
        }

        void listen_events(Control con)
        {
            char *buffer = (char *) malloc(120);
            memset(buffer, 0, 120);

            regex device("\\d+-\\d+");
            regex interface("\\d+-\\d+:\\d+.\\d+");

            while(1)
            {
                if (read(sock, buffer, 120) > 0)
                {
                    if (!(strncmp("add@", buffer, 4)))
                    {
                        if (regex_match(buffer + find_last_folder(buffer), device))
                        {
                            save_device(buffer);
                        }
                        
                        if (regex_match(buffer + find_last_folder(buffer), interface))
                        {
                            save_interface(buffer, con);
                        }
                        
                    }
                }
            }
        }
};

int main()
{
    Control con;
    Netlink mon;

    con.read_rules(&con);

    mon.init_enviroment();
    mon.listen_events(con);

    cout << con.rules.back()->interfaceSubclass << "\n";

    return 0;
}