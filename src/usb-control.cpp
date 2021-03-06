/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        3.6.2020
* Description of file:
*
* This file consists of the program implementation, which detects newly connected USB devices
* and inspects their attributes, trying to match them with rules defined by user. In case of a
* match, all interfaces under inspected device are authorized and loading of their driver is
* triggered.
*
*/

#include "usb-control.h"

#define FAILED 1

using namespace std;
#if __GNUC__ >= 8
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif

Control *Control::control;

/*
* Loads all attributes related with interface from files in given path.
*/
void Interface::load_attributes()
{
    ifstream fInterfaceClass;
    ifstream fInterfaceSubclass;
    fInterfaceClass.open(path + "/bInterfaceClass");
    fInterfaceSubclass.open(path + "/bInterfaceSubClass");
    getline(fInterfaceClass, interfaceClass);
    getline(fInterfaceSubclass, interfaceSubclass);
}

/*
* Loads all attributes related with device from files in given path.
*/
void Device::load_attributes()
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

/*
* Disconnect device by writing "0" in file named "authorized" in given path on device level.
*/
void Device::disconnect()
{
    ofstream fAuthorized;
    fAuthorized.open(path + "/authorized");
    fAuthorized << "0";
    fAuthorized.close();
}

/*
* Return position of string when last folder starts.
*/
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
    static Control *con = Control::get_control();
    Rule *rule = new Rule;

    rule->ID = (argv[0] ? argv[0] : "NULL");
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
int Control::check_if_no_rules_callback(void *data, int argc, char **argv, char **column)
{
    if(**argv == '0')
    {
        throw GeneralExc("There are no rules in database.");
    }
    return 0;
}

/*
* Callback which sets flag if any rules group was found, so we can read them later. 
*/
int Control::check_if_no_groups_callback(void *data, int argc, char **argv, char **column)
{
    Control *con = Control::get_control();

    if(**argv != '0')
    {
        con->groupFoundFlag = true;
    }
    return 0;
}

/*
* Callback which sets flag if any rules group was found, so we can read them later. 
*/
int Control::check_if_no_indiv_rules_callback(void *data, int argc, char **argv, char **column)
{
    Control *con = Control::get_control();

    if(**argv != '0')
    {
        con->indivRuleFoundFlag = true;
    }
    return 0;
}

/*
* Save result returned from SQL command in class variables.
*/
void Control::save_groups(char **argv)
{
    int i = 0;
    static Control *con = Control::get_control();
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
    Control *con = Control::get_control();
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
    string SQLCheckIfIndivRulesExist = "SELECT COUNT() FROM RULE WHERE GROUP_ID IS NULL";
    string SQLSelectGroups = "SELECT * FROM RULE WHERE GROUP_ID IS NOT NULL";
    char *errmsg = NULL;


    rc = sqlite3_open_v2("database/db", &db, SQLITE_OPEN_READWRITE, NULL);
    if (rc) 
    {
        throw GeneralExc("Could not open database. Does it exist?");
    }

    rc = sqlite3_exec(db, SQLCheckIfTableExists.c_str(), check_if_no_rules_callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }

    rc = sqlite3_exec(db, SQLCeckIfRulesExists.c_str(), check_if_no_rules_callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }

    rc = sqlite3_exec(db, SQLSelectRules.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }

    rc = sqlite3_exec(db, SQLCheckIfGroupsExist.c_str(), check_if_no_groups_callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }

    rc = sqlite3_exec(db, SQLCheckIfIndivRulesExist.c_str(), check_if_no_indiv_rules_callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }

    if (groupFoundFlag)
    {
        rc = sqlite3_exec(db, SQLSelectGroups.c_str(), parse_groups, NULL, &errmsg);
        if (rc)
        {
            throw DatabaseExc("Could not execute command", string(errmsg));
        }
    }
}

/*
* Every attribute from rules is compared with attributes from device and its interfaces. If no
* rules which could satisfy device and its interafaces were found, method returns false, otherwise
* true.
*/
bool Control::check_for_rule(Device device, string *ruleID)
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
            else if(device.port.find((*rule)->port) == string::npos && (*rule)->port != "NULL")
            {
                continue;
            }
            else if(device.interfacesTotal != (*rule)->interfacesTotal && (*rule)->interfacesTotal != 999)
            {
                continue;
            }
            else
            {
                *ruleID = (*rule)->ID;
                return true;
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
bool Control::check_for_group(Device device, string *groupID)
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
                            *groupID = (*group)->groupID;
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
    vector<Interface>::iterator interface;
    int counter = 0;
    string ruleID;
    string groupID;

    if(indivRuleFoundFlag)
    {
        authorized = authorized | check_for_rule(device, &ruleID);
    }
    if(groupFoundFlag && !authorized)
    {
        authorized = authorized | check_for_group(device, &groupID);
        clean_groups();
    }

    if (authorized)
    {
        device.authorize();
        cout << "Device with " + device.vendor + ":" + device.product + " vendor:product ID and class: " + device.deviceClass + ", subclass: " + device.deviceSubclass + "\n";
        cout << "with interface(s):\n";
        for( interface = device.interfaces.begin(); interface < device.interfaces.end(); interface++)
        {
            counter++;
            cout << "(" << counter << ")\tclass: " << interface->interfaceClass << ", sublclass: " << interface->interfaceSubclass << "\n";
        }
        cout << "authorized on port " + device.port + "\n";
        if(!ruleID.empty())
        {
            cout << "Authorization based on rule " + ruleID + "\n\n";
        }
        else if(!groupID.empty())
        {
            cout << "Authorization based on group " + groupID + "\n\n";
        }
    }
    else
    {
        device.disconnect();
        cout << "Disconnecting potentially dangerous device with " + device.vendor + ":" + device.product + " vendor:product ID and class: " + device.deviceClass + ", subclass: " + device.deviceSubclass + "\n";
        cout << "with interface(s):\n";
        for( interface = device.interfaces.begin(); interface < device.interfaces.end(); interface++)
        {
            counter++;
            cout << "(" << counter << ")\tclass: " << interface->interfaceClass << ", sublclass: " << interface->interfaceSubclass << "\n";
        }
        cout << "on port " + device.port + "\n\n";
    }
}

/*
* Method to get pointer for the only one object of Control class, because singleton is used.
*/
Control *Control::get_control()
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
        throw SocketExc("Unable to create netlink socket.");
    }

    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = -1;

    if (bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_nl)))
    {
        throw SocketExc("Unable to bind socket.");
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
    numInterfacesFile.open(device.path + "/bNumInterfaces");
    getline(numInterfacesFile, numInterfaces);
    device.interfacesTotal = stoi(numInterfaces);
    device.load_attributes();
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
            interface.load_attributes();
            (*device).interfaces.push_back(interface);
            (*device).interfacesFound++;

            if((*device).interfacesFound == (*device).interfacesTotal)
            {
                con = Control::get_control();
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
        throw GeneralExc("Could not allocate memory.");
    }
    
    memset(buffer, 0, 120);

    regex device("\\d+-\\d+(\\.\\d)*");
    regex interface("\\d+-\\d+(\\.\\d)*:\\d+\\.\\d+");

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
            interfaceAuthDefFile.open(string(fs::path(file)) + "/interface_authorized_default");
            interfaceAuthDefFile << "1";
            interfaceAuthDefFile.close();
        }
    }

    exit(0);
}