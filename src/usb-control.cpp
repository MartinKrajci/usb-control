#include "usb-control.h"

#define FAILED 1

using namespace std;
namespace fs = std::filesystem;

Control *Control::control;

void Interface::loadAttributes()
{
    ifstream fInterfaceClass;
    ifstream fInterfaceSubclass;
    fInterfaceClass.exceptions(ifstream::failbit);
    fInterfaceSubclass.exceptions(ifstream::failbit);
    fInterfaceClass.open(path + "/bInterfaceClass");
    fInterfaceSubclass.open(path + "/bInterfaceSubClass");
    getline(fInterfaceClass, interfaceClass);
    getline(fInterfaceSubclass, interfaceSubclass);
}

void Device::loadAttributes()
{
    ifstream fDeviceClass;
    ifstream fDeviceSubclass;
    ifstream fVendor;
    ifstream fProduct;
    ifstream fPort;
    fDeviceClass.exceptions(ifstream::failbit);
    fDeviceSubclass.exceptions(ifstream::failbit);
    fVendor.exceptions(ifstream::failbit);
    fProduct.exceptions(ifstream::failbit);
    fPort.exceptions(ifstream::failbit);
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

void Device::disconnect()
{
    ofstream fAuthorized;
    fAuthorized.exceptions(ofstream::failbit);
    fAuthorized.open(path + "/authorized");
    fAuthorized << "0";
    fAuthorized.close();
}

        int Device::find_last_folder(const char *path)
        {
            int i = strlen(path);
            for(; path[i] != '/'; i--);
            return i + 1;
        }

void Device::authorize()
{
    vector<Interface>::iterator interface;
    int fdDriversProbe = 0;
    FILE *fDriversProbe ;

    bool test = false;
    for (interface = interfaces.begin(); interface < (interfaces.end()); interface++)
    {
            ofstream fAuthorized;
            fAuthorized.exceptions(ifstream::failbit);
            fAuthorized.open(interface->path + "/authorized");
            fAuthorized << "1";
            fDriversProbe = popen("sudo tee /sys/bus/usb/drivers_probe > /dev/null", "w");
            fdDriversProbe = fileno(fDriversProbe);
            
            write(fdDriversProbe, find_last_folder(interface->path.c_str()) + interface->path.c_str(), strlen(find_last_folder(interface->path.c_str()) + interface->path.c_str()));
            close(fdDriversProbe);
    }
}

void Control::save_rules(char **argv)
{
    int i = 0;
    static Control *con = Control::getControl();
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

int Control::callback(void *data, int argc, char **argv, char **column)
{
    Control::save_rules(argv);
    return 0;
}

int Control::checkIfNoRules(void *data, int argc, char **argv, char **column)
{
    if(**argv == '0')
    {
        cerr << "There are no rules in database!\n";
        throw FAILED;
    }
    return 0;
}

int Control::checkIfNoGroups(void *data, int argc, char **argv, char **column)
{
    Control *con = Control::getControl();

    if(**argv == '0')
    {
        con->GroupFoundFlag = true;
    }
    return 0;
}

void Control::save_groups(char **argv)
{
    int i = 0;
    static Control *con = Control::getControl();
    RulesGroup *group = new RulesGroup;

    group->deviceClass = (argv[1] ? argv[1] : "NULL");
    group->deviceSubclass = (argv[2] ? argv[2] : "NULL");
    group->vendor = (argv[3] ? argv[3] : "NULL");
    group->product = (argv[4] ? argv[4] : "NULL");
    group->interfaceClass = (argv[5] ? argv[5] : "NULL");
    group->port = (argv[8] ? argv[8] : "NULL");
    group->groupID = (argv[9] ? argv[9] : "NULL");
    con->groups.push_back(group);
}

void Control::save_interface_rule(char **argv)
{
    InterfaceRule *rule = new InterfaceRule;
    Control *con = Control::getControl();
    vector<RulesGroup *>::iterator group;

    rule->interfaceClass = (argv[6] ? argv[6] : "NULL");
    rule->interfaceSubclass = (argv[7] ? argv[7] : "NULL");

    for (group = con->groups.begin(); group < con->groups.end(); group++)
    {
        if (string(argv[9]) == (* group)->groupID)
        {
            (* group)->interfaceRules.push_back(rule);
        }
        
    }
    
}

int Control::parse_groups(void *data, int argc, char **argv, char **column)
{
    for(int i = 0; i<argc; i++)
    {
        if(string(argv[10]) == "1")
        {
            save_groups(argv);
        }
        else
        {
            save_interface_rule(argv);
        }
    }
    return 0;
}

void Control::read_rules()
{
    int rc;
    sqlite3 *db;
    //string SQLSelectRules = "SELECT * FROM RULE WHERE GROUP_ID IS NULL";
    string SQLSelectRules = "SELECT * FROM RULE";
    string SQLCheckIfTableExists = "SELECT COUNT(type) FROM sqlite_master WHERE TYPE='table' AND NAME='RULE'";
    string SQLCeckIfRulesExists =  "SELECT COUNT() FROM RULE";
    string SQLCheckIfGroupsExist = "SELECT COUNT() FROM RULE WHERE GROUP_ID IS NOT NULL";
    string SQLSelectGroups = "SELECT * FROM RULE WHERE GROUP_ID IS NOT NULL";
    char *errmsg = NULL;

    rc = sqlite3_open("database/db", &db);
    if (rc) 
    {
        cerr << "Could not open database\n";
        throw FAILED;
    }

    rc = sqlite3_exec(db, SQLCheckIfTableExists.c_str(), checkIfNoRules, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }

    rc = sqlite3_exec(db, SQLCeckIfRulesExists.c_str(), checkIfNoRules, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }

    rc = sqlite3_exec(db, SQLSelectRules.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }

    /*rc = sqlite3_exec(db, SQLCheckIfGroupsExist.c_str(), checkIfNoGroups, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }

    if (GroupFoundFlag)
    {
        GroupFoundFlag = false;

        rc = sqlite3_exec(db, SQLSelectGroups.c_str(), parse_groups, NULL, &errmsg);
        if (rc)
        {
            cerr << "Could not execute command, " << errmsg << "\n";
            throw FAILED;
        }
    }*/
}

void Control::check_device(Device device)
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

Control *Control::getControl()
{
    if(control == 0)
    {
        control = new Control;
    }
    return control;
}

Netlink::Netlink()
{
    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(struct sockaddr_nl));

    sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sock < 0)
    {
        cerr << "Unable to create netlink socket\n";
        throw FAILED;
    }

    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = -1;

    if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_nl)))
    {
        cerr << "Unable to bind socket\n";
        throw FAILED;
    }

}

void Netlink::init_enviroment()
{
    regex usbFolder("usb\\d+");

    for(auto& file: fs::directory_iterator("/sys/bus/usb/devices"))
    {
        if(regex_match(string(fs::path(file).filename()), usbFolder))
        {
            ofstream interfaceAuthDefFile;
            interfaceAuthDefFile.exceptions(ofstream::failbit);
            interfaceAuthDefFile.open(string(fs::path(file)) + "/interface_authorized_default");
            interfaceAuthDefFile << "0";
            interfaceAuthDefFile.close();
        }
    }
}

int Netlink::find_last_folder(char *path)
{
    int i = strlen(path);
    for(; path[i] != '/'; i--);
    return i + 1;
}

void Netlink::save_device(char *buffer)
{
    Device device;
    ifstream numInterfacesFile;
    string numInterfaces;
    device.path = "/sys" + string(buffer + 4);
    numInterfacesFile.exceptions(ifstream::failbit);
    numInterfacesFile.open(device.path + "/bNumInterfaces");
    getline(numInterfacesFile, numInterfaces);
    device.interfacesTotal = stoi(numInterfaces);
    device.loadAttributes();
    devices.push_back(device);
}

void Netlink::save_interface(char *buffer)
{
    static Control *con;
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

            if((*device).interfacesFound == (*device).interfacesTotal)
            {
                con = Control::getControl();
                con->check_device(*device);
                devices.erase(device);
            }
        }
    }
}

void Netlink::listen_events()
{
    buffer = (char *) malloc(120);
    if (buffer == NULL)
    {
        cerr << "Could not allocate memory\n";
        throw FAILED;
    }
    
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
                    save_interface(buffer);
                }
                
            }
        }
    }
}

void at_exit()
{
    regex usbFolder("usb\\d+");

    for(auto& file: fs::directory_iterator("/sys/bus/usb/devices"))
    {
        if(regex_match(string(fs::path(file).filename()), usbFolder))
        {
            ofstream interfaceAuthDefFile;
            interfaceAuthDefFile.exceptions(ifstream::failbit);
            interfaceAuthDefFile.open(string(fs::path(file)) + "/interface_authorized_default");
            interfaceAuthDefFile << "1";
            interfaceAuthDefFile.close();
        }
    }
    
    exit(0);
}