/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        25.4.2020
*/

#include "usb-control.h"

#define FAILED 1

using namespace std;
namespace fs = std::filesystem;

Control *Control::control;

/*
* Loads all attributes related with interface from files in given path.
*/
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

/*
* Loads all attributes related with device from files in given path.
*/
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

/*
* Disconnect device by writing "0" in file named "authorized" in given path on device level.
*/
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
/*
* Authorize every interface of device by writing "1" in file named "authorized" in given path on
* interface level and by writing folder name belonging to this interface into
* /sys/bus/usb/drivers_probe file, which cause drivers binding.
*/
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

/*
* Save result returned from SQL command into Control class variables.
*/
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

/*
* Callback needed by sqlite3
*/
int Control::callback(void *data, int argc, char **argv, char **column)
{
    Control::save_rules(argv);
    return 0;
}

/*
* Callback checking if there are no rules in database. If so, exception is thrown because application
* cannot continue without rules.
*/
int Control::checkIfNoRulesCallback(void *data, int argc, char **argv, char **column)
{
    if(**argv == '0')
    {
        cerr << "There are no rules in database!\n";
        throw FAILED;
    }
    return 0;
}

/*
* Callback which sets flag if any rules group was found, so we can read them later. 
*/
int Control::checkIfNoGroupsCallback(void *data, int argc, char **argv, char **column)
{
    Control *con = Control::getControl();

    if(**argv != '0')
    {
        con->GroupFoundFlag = true;
    }
    return 0;
}

/*
* Save result returned from SQL command in class variables.
*/
void Control::save_groups(char **argv)
{
    int i = 0;
    static Control *con = Control::getControl();
    RulesGroup *group = new RulesGroup;

    group->deviceClass = (argv[1] ? argv[1] : "NULL");
    group->deviceSubclass = (argv[2] ? argv[2] : "NULL");
    group->vendor = (argv[3] ? argv[3] : "NULL");
    group->product = (argv[4] ? argv[4] : "NULL");
    group->interfacesTotal = (argv[7] ? stoi(argv[7]) : 999);
    group->port = (argv[8] ? argv[8] : "NULL");
    group->groupID = (argv[9] ? argv[9] : "NULL");
    con->groups.push_back(group);
}

/*
* Save result returned from SQL command in class variables and find rules group where it belongs to.
*/
void Control::save_interface_rule(char **argv)
{
    InterfaceRule *rule = new InterfaceRule;
    Control *con = Control::getControl();
    vector<RulesGroup *>::iterator group;
    rule->interfaceClass = (argv[5] ? argv[5] : "NULL");
    rule->interfaceSubclass = (argv[6] ? argv[6] : "NULL");
    for (group = con->groups.begin(); group < con->groups.end(); group++)
    {
        if (string(argv[9]) == (* group)->groupID)
        {
            (* group)->interfaceRules.push_back(rule);
        }
    }
}

/*
* Method decides if result returned from SQL command is group for rules or interface rule.
*/
int Control::parse_groups(void *data, int argc, char **argv, char **column)
{
    if(argv[10] != NULL)
    {
        save_groups(argv);
    }
    else
    {
        save_interface_rule(argv);
    }

    return 0;
}

/*
* If there are any rules or group of rules, they are read from database and processed.
*/
void Control::read_rules()
{
    int rc;
    sqlite3 *db;
    string SQLSelectRules = "SELECT * FROM RULE WHERE GROUP_ID IS NULL";
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

    rc = sqlite3_exec(db, SQLCheckIfTableExists.c_str(), checkIfNoRulesCallback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }

    rc = sqlite3_exec(db, SQLCeckIfRulesExists.c_str(), checkIfNoRulesCallback, NULL, &errmsg);
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

    rc = sqlite3_exec(db, SQLCheckIfGroupsExist.c_str(), checkIfNoGroupsCallback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }

    if (GroupFoundFlag)
    {
        rc = sqlite3_exec(db, SQLSelectGroups.c_str(), parse_groups, NULL, &errmsg);
        if (rc)
        {
            cerr << "Could not execute command, " << errmsg << "\n";
            throw FAILED;
        }
    }
}

/*
* Every attribute from rules is compared with attributes from device and its interfaces. If no
* rules which could satisfy device and its interafaces were found, method returns false, otherwise
* true.
*/
bool Control::check_for_rule(Device device)
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
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

/*
* At first compare device attributes and attributes from group. After that, every interface of
* device and its attributes are compared with interface rules, belonging to particular group.
* Every interface rule can be used only once. If no match is found, method returns false, otherwise
* true. 
*/
bool Control::check_for_group(Device device)
{
    vector<RulesGroup *>::iterator group;

    for (group = groups.begin(); group < (groups.end()); group++)
    {
        if (device.interfacesTotal < (*group)->interfaceRules.size())
        {
            continue;
        }

        if(device.deviceClass != (*group)->deviceClass && (*group)->deviceClass != "NULL")
        {
            continue;
        }
        else if(device.deviceSubclass != (*group)->deviceSubclass && (*group)->deviceSubclass != "NULL")
        {
            continue;
        }
        else if(device.vendor != (*group)->vendor && (*group)->vendor != "NULL")
        {
            continue;
        }
        else if(device.product != (*group)->product && (*group)->product != "NULL")
        {
            continue;
        }
        else if(device.port != (*group)->port && (*group)->port != "NULL")
        {
            continue;
        }
        else if(device.interfacesTotal != (*group)->interfacesTotal && (*group)->interfacesTotal != 999)
        {
            continue;
        }
        else
        {
            vector<InterfaceRule *>::iterator interface;

            for(int currentInterface = 0; currentInterface < device.interfacesTotal; currentInterface++)
            {
                for (interface = (*group)->interfaceRules.begin();interface < (*group)->interfaceRules.end(); interface++)
                {
                    if((*interface)->wasUsed)
                    {
                        continue;
                    }

                    if(device.interfaces[currentInterface].interfaceClass != (*interface)->interfaceClass && (*interface)->interfaceClass != "NULL")
                    {
                        continue;
                    }
                    else if(device.interfaces[currentInterface].interfaceSubclass != (*interface)->interfaceSubclass && (*interface)->interfaceSubclass != "NULL")
                    {
                        continue;
                    }
                    else
                    {
                        if((currentInterface + 1) == device.interfacesTotal)
                        {
                            return true;
                        }
                        (*interface)->wasUsed = true;
                        break;
                    }
                }
            }
        }
    }

    return false;
}

/*
* Set flag if interface rules was used to false, so they can be used again when new device will
* be checked.
*/
void Control::clean_groups()
{
    vector<RulesGroup *>::iterator group;
    vector<InterfaceRule *>::iterator interface;

    for (group = groups.begin(); group < groups.end(); group++)
    {   
        for(interface = (*group)->interfaceRules.begin(); interface < (*group)->interfaceRules.end(); interface++)
        {
            (*interface)->wasUsed = false;
        }
    }
    
}

/*
* Check new connected device with rules and groups of rules. If at least one match was found,
* device is authorized, otherwise disconnected.
*/
void Control::check_device(Device device)
{
    bool authorized = false;

    authorized = authorized | check_for_rule(device);
    if(GroupFoundFlag)
    {
        authorized = authorized | check_for_group(device);
        clean_groups();
    }

    if (authorized)
    {
        device.authorize();
        cout << "Device authorized..\n";
    }
    else
    {
        device.disconnect();
        cout << "! Disconnecting potenctialy dangerous device !\n";
    }
}

/*
* Method to get pointer for the only one object of Control class, because singleton is used.
*/
Control *Control::getControl()
{
    if(control == 0)
    {
        control = new Control;
    }
    return control;
}

/*
* Constructor of Netlink class. Netlink socket is created and binded.
*/
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

/*
* Writes "0" to files named "interface_authorized_default" in every usb bus folder, so newly
* connected deviced can't make any harm before they are checked. 
*/
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

/*
* Return position of string when last folder starts.
*/
int Netlink::find_last_folder(char *path)
{
    int i = strlen(path);
    for(; path[i] != '/'; i--);
    return i + 1;
}

/*
* Object for newly connected device is created and path of device and number of interfaces are saved.
*/
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

/*
* Object for interface of newly connected device is created and its path is saved. If all interfaces
* were found, device is compared against rules. 
*/
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

/*
* Main loop of whole application. If there are any new messages in buffer from kernel, they are
* compared with regexes and divided into paths of devices and interfaces. 
*/
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

/*
* Function called in the end of application, returning enviroment into original state so newly
* connected devices won't be affected.
*/
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