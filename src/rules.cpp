/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        3.6.2020
* Description of file:
*
* This file consists of the program implementation, which creates interface for defining rules.
* Rules can be created, displayed or removed. Also support for default rules creation is
* implemented.
*
*/

#include "rules.h"

using namespace std;

#if __GNUC__ >= 8
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif

Database *Database::database;

/*
* Database constructor. Open database file and create a new table for rules, if one already don't exist.
*/
Database::Database()
{
    int rc = 0;
    char *errmsg = NULL;
    string SQLCreate = "CREATE TABLE IF NOT EXISTS RULE("  \
        "ID                 INTEGER PRIMARY KEY," \
        "DEVICE_CLASS           TEXT," \
        "DEVICE_SUBCLASS         TEXT," \
        "VENDOR                 TEXT," \
        "PRODUCT                TEXT," \
        "INTERFACE_CLASS        TEXT," \
        "INTERFACE_SUBCLASS     TEXT," \
        "INTERFACE_COUNT        INT," \
        "PORT                   TEXT," \
        "GROUP_ID                  INT," \
        "IS_GROUP                  INT);";

    rc = sqlite3_open("database/db", &db);
    if (rc) 
    {
        throw GeneralExc("Could not open database. Does it exist?");
    }

    rc = sqlite3_exec(db, SQLCreate.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }
}

/*
* Method to get pointer to the only one pointer to object of Database class because we are
* using singletone.
*/
Database *Database::getDatabase()
{
    if (database == NULL)
    {
        database = new Database;
    }

    return database;
}

/*
* Check if given string is of correct format (two hexadecimal numbers).
*/
void Database::check_if_two_hex(string arg)
{
    regex twoHex("[0-9a-fA-F]{2}");

    if (!regex_match(arg, twoHex))
    {
        throw BadArgExc("twoHex", "Wrong input! Expecting argument in form of two hex numbers.");
    }
}

/*
* Check if given string is of correct format (four hexadecimal numbers).
*/
void Database::check_if_four_hex(string arg)
{
    regex fourHex("[0-9a-fA-F]{4}");

    if (!regex_match(arg, fourHex))
    {
        throw BadArgExc( "fourHex", "Wrong input! Expecting argument in form of four hex numbers.");
    }
}

/*
* Check if given string is of correct format (decimal number).
*/
void Database::check_if_num(string arg)
{
    regex num("\\d+");

    if (!regex_match(arg, num))
    {
        throw BadArgExc("num", "Wrong input! Expecting argument of type number.");
    }
}

/*
* Check if given string is of correct format (number or numbers, separated by dot).
*/
void Database::check_if_port(string arg)
{
    regex port("\\d+(\\.\\d+)*(\\.){0,1}");

    if (!regex_match(arg, port))
    {
        throw BadArgExc("num", "Wrong input! Expecting string describing port as an argument.");
    }
}

/*
* Convert upper-case hex numbers to lower-case.
*/
void Database::hex_to_lower(char* chars)
{
    for(int i = 0; chars[i] != 0; i++)
    {
        if (chars[i] >= 'A' && chars[i] <= 'Z')
        {
            chars[i] = chars[i] - ('A' - 'a');
        }
    }

}

/*
* Method for parsing command line parameters. Arguments of parameters are checked and stored in
* class variables.
*/
void Database::parse_arguments(int argc, char **argv)
{
    int opt = 0;
    int opt_index = 0;

    const struct option longopts[] = 
    {
        {"add-rule", no_argument, 0, 'a'},
        {"show-rules", no_argument, 0, 's'},
        {"remove-rule", required_argument, 0, 'x'},
        {"device-class", required_argument, 0, 'd'},
        {"device-subclass", required_argument, 0, 'e'},
        {"vendor", required_argument, 0, 'v'},
        {"product", required_argument, 0, 'p'},
        {"interface-class", required_argument, 0, 'i'},
        {"interface-subclass", required_argument, 0, 'u'},
        {"interfaces-total", required_argument, 0, 'c'},
        {"port", required_argument, 0, 'o'},
        {"group-id", required_argument, 0, 'g'},
        {"new-group", no_argument, 0, 'n'},
        {"set-default-rules", no_argument, 0, 't'},
        {0, 0, 0, 0}
    };


    while ((opt = getopt_long(argc, argv, "-asntx:d:i:h:e:u:v:p:c:o:g:", longopts, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'a':
            createRule = true;
            break;
        case 'x':
            deleteRule = true;
            if(string(optarg) == "all")
            {
                deleteAllRules = true;
                break;
            }
            check_if_num(string(optarg));
            ruleIDs.push_back(string(optarg));
            break;
        case 's':
            showRules = true;
            break;
        case 'd':
            check_if_two_hex(string(optarg));
            hex_to_lower(optarg);
            deviceClass = string(optarg);
            hasAttribute = true;
            break;
        case 'e':
            check_if_two_hex(string(optarg));
            hex_to_lower(optarg);
            deviceSubclass = string(optarg);
            hasAttribute = true;
            break;
        case 'i':
            check_if_two_hex(string(optarg));
            hex_to_lower(optarg);
            interfaceClass = string(optarg);
            hasAttribute = true;
            break;
        case 'u':
            check_if_two_hex(string(optarg));
            hex_to_lower(optarg);
            interfaceSubclass = string(optarg);
            hasAttribute = true;
            break;
        case 'v':
            check_if_four_hex(string(optarg));
            hex_to_lower(optarg);
            vendor = string(optarg);
            hasAttribute = true;
            break;
        case 'p':
            check_if_four_hex(string(optarg));
            hex_to_lower(optarg);
            product = string(optarg);
            hasAttribute = true;
            break;
        case 'c':
            check_if_num(string(optarg));
            interfacesTotal = string(optarg);
            hasAttribute = true;
            break;
        case 'o':
            check_if_port(string(optarg));
            port = string(optarg);
            hasAttribute = true;
            break;
        case 'g':
            check_if_num(string(optarg));
            groupID = string(optarg);
            break;
        case 'n':
            newGroup = true;
            break;
        case 't':
            defaultRules = true;
            break;
        case '?':
            if(optopt == 'x')
            throw GeneralExc("Expecting number or string \"all\".");
            break;
        default:
            if(!ruleIDs.empty())
            {
                check_if_num(string(optarg));
                ruleIDs.push_back(string(optarg));
            }
            else
            {
                if(optarg == NULL)
                {
                    throw GeneralExc("Unrecognized parameter.");
                }
                else
                {
                    throw GeneralExc("Unrecognized argument \"" + string(optarg) + "\".");
                }
            }
            break;
        }
    }
}


/*
* Callback for SQL commands, its form is required by sqlite3. It's called for every row in
/ result of SQL command.
*/
int Database::callback(void *data, int argc, char **argv, char **column)
{
    string middleSeparator =    "---------------------------"\
                                "---------------------------"\
                                "---------------------------"\
                                "---------------------------"\
                                "-------------------------------";
    string separator =  "==========================="\
                        "==========================="\
                        "==========================="\
                        "==========================="\
                        "===============================";
    cout << separator << "\n|";

    for(int i = 0; i<argc; i++)
    {
        if(i == 5)
        {
            cout << "\n";
            cout << middleSeparator;
            cout << "\n|";
        }

        if(string(column[i]) != "IS_GROUP")
        {
            printf("%18s = %4s | ", column[i], argv[i] ? argv[i] : "*");
        }
    }

    cout << "\n" << separator << "\n\n";

   return 0;
}

/*
* Callback for SQL command which check if given group exists.
*/
int Database::check_if_group_exists_callback(void *data, int argc, char **argv, char **column)
{
    if(**argv == '0')
    {
        throw BadArgExc("groupID", "Group with this ID do not exists! Use '-n' parameter to create a new one.");
    }
    return 0;
}

/*
* Execute SQL command which check if given group exists.
*/
void Database::checkIfGroupExists()
{
    int rc;
    char *errmsg = NULL;
    string SQLCheck;

    SQLCheck = "SELECT COUNT() FROM RULE WHERE GROUP_ID='" + groupID + "'";

    rc = sqlite3_exec(db, SQLCheck.c_str(), check_if_group_exists_callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }
}

/*
* Method assemble SQL command from parameters saved in class variables and parts of strings. When
* adding new value, it needs to be checked if it's first entry. If so, different string parts are
* added to the final command. After assembling, SQL command is executed and report of success is
* printed in terminal.
*/
void Database::insert()
{
    string SQLInsert =  "INSERT INTO RULE (";
    string SQLInsertValues = "VALUES (";
    string partWithComma = ", \'";
    string partWithoutComma = "\'";
    string comma = ", ";
    bool firstEntry = true;
    int rc;
    char *errmsg = NULL;

    if (!(createRule & hasAttribute) & !defaultRules)
    {
        throw BadParamExc("-a", "Missing arguments for rule creation.");
    }

    if(newGroup)
    {
        check_if_group_not_exists();
        attributes_check();
    }

    if (!groupID.empty() & !newGroup)
    {
        if(!deviceClass.empty() | !deviceSubclass.empty() | !vendor.empty() |
           !product.empty() | !interfacesTotal.empty() | !port.empty())
        {
            throw BadParamExc("-g", "Do not use device attributes when group already exists.");
        }
        checkIfGroupExists();
    }

    if (!deviceClass.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "DEVICE_CLASS";
            SQLInsertValues = SQLInsertValues + partWithoutComma + deviceClass + partWithoutComma;
        }
        else
        {
            SQLInsert = SQLInsert + comma + "DEVICE_CLASS";
            SQLInsertValues = SQLInsertValues  + partWithComma + deviceClass + partWithoutComma;
        }
    }

    if (!deviceSubclass.empty())
    {
        if(firstEntry)
        {
            firstEntry = false;
            SQLInsert += "DEVICE_SUBCLASS";
            SQLInsertValues = SQLInsertValues + partWithoutComma + deviceSubclass + partWithoutComma;
        }
        else
        {
            SQLInsert = SQLInsert + comma + "DEVICE_SUBCLASS";
            SQLInsertValues = SQLInsertValues + partWithComma + deviceSubclass + "\'";   
        }
    }

    if (!interfaceClass.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "INTERFACE_CLASS";
            SQLInsertValues = SQLInsertValues + partWithoutComma + interfaceClass + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "INTERFACE_CLASS";
            SQLInsertValues = SQLInsertValues + partWithComma + interfaceClass + partWithoutComma;   
        }
    }

    if (!interfaceSubclass.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "INTERFACE_SUBCLASS";
            SQLInsertValues = SQLInsertValues + partWithoutComma + interfaceSubclass + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "INTERFACE_SUBCLASS";
            SQLInsertValues = SQLInsertValues + partWithComma + interfaceSubclass + partWithoutComma;   
        }
    }

    if (!vendor.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "VENDOR";
            SQLInsertValues = SQLInsertValues + partWithoutComma + vendor + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "VENDOR";
            SQLInsertValues = SQLInsertValues + partWithComma + vendor + partWithoutComma;   
        }
    }

    if (!product.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "PRODUCT";
            SQLInsertValues = SQLInsertValues + partWithoutComma + product + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "PRODUCT";
            SQLInsertValues = SQLInsertValues + partWithComma + product + partWithoutComma;   
        }
    }

    if (!interfacesTotal.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "INTERFACE_COUNT";
            SQLInsertValues = SQLInsertValues + partWithoutComma + interfacesTotal + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "INTERFACE_COUNT";
            SQLInsertValues = SQLInsertValues + partWithComma + interfacesTotal + partWithoutComma;   
        }
    }

    if (!port.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "PORT";
            SQLInsertValues = SQLInsertValues + partWithoutComma + port + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "PORT";
            SQLInsertValues = SQLInsertValues + partWithComma + port + partWithoutComma;   
        }
    }

    if (!groupID.empty())
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "GROUP_ID";
            SQLInsertValues = SQLInsertValues + partWithoutComma + groupID + partWithoutComma;                
        }
        else
        {
            SQLInsert = SQLInsert + comma + "GROUP_ID";
            SQLInsertValues = SQLInsertValues + partWithComma + groupID + partWithoutComma;   
        }
    }

    if (newGroup)
    {
        if (firstEntry)
        {
            firstEntry = false;
            SQLInsert += "IS_GROUP";
            SQLInsertValues = SQLInsertValues + partWithoutComma + "1" + partWithoutComma;
        }
        else
        {
            SQLInsert = SQLInsert + comma + "IS_GROUP";
            SQLInsertValues = SQLInsertValues + partWithComma + "1" + partWithoutComma;   
        }
    }

    SQLInsert += ") " + SQLInsertValues + ");";

    rc = sqlite3_exec(db, SQLInsert.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }
}

/*
* Execute SQL command which shows all rules from database and their values.
*/
void Database::show()
{
    int rc;
    char *errmsg = NULL;
    string SQLSelect = "SELECT * FROM RULE";
    string separator =  "==========================="\
                        "==========================="\
                        "==========================="\
                        "==========================="\
                        "===============================";
    
    rc = sqlite3_exec(db, SQLSelect.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }

    //cout << separator << "\n";
}

/*
* Execute SQL command which removes rule from database with given rule ID.
*/
void Database::remove()
{
    int rc;
    char *errmsg = NULL;
    vector<string>::iterator ruleID;
    string SQLDeleteAll = "DELETE FROM RULE;";

    if(deleteAllRules)
    {
        rc = sqlite3_exec(db, SQLDeleteAll.c_str(), callback, NULL, &errmsg);
        if (rc)
        {
            throw DatabaseExc("Could not execute command", string(errmsg));
        }
    }
    else
    {
        for(ruleID = ruleIDs.begin(); ruleID < ruleIDs.end();ruleID++)
        {
            string SQLDeleteGroup = "DELETE FROM RULE WHERE group_id = (SELECT group_id from RULE WHERE ID = " + *ruleID + " AND is_group = 1);";
            string SQLDelete =  "DELETE FROM RULE WHERE ID = " + *ruleID + ";";

            rc = sqlite3_exec(db, SQLDeleteGroup.c_str(), callback, NULL, &errmsg);
            if (rc)
            {
                throw DatabaseExc("Could not execute command", string(errmsg));
            }

            rc = sqlite3_exec(db, SQLDelete.c_str(), callback, NULL, &errmsg);
            if (rc)
            {
                throw DatabaseExc("Could not execute command", string(errmsg));
            }
        }
    }
}

/*
* Callback for SQL command which check if given group dosn't exist.
*/
int Database::check_if_group_not_exists_callback(void *data, int argc, char **argv, char **column)
{
    if(**argv != '0')
    {
        throw BadArgExc("groupID", "Group with this ID already exists.");
    }
    return 0;
}

/*
* Check if group ID was given and execute SQL command which check if given group dosn't exist.
*/
void Database::check_if_group_not_exists()
{
    int rc;
    char *errmsg = NULL;
    string SQLCheck;

    if(groupID.empty())
    {
        throw BadArgExc("groupID", "Group ID needs to be specified when creating new group.");
    }

    SQLCheck = "SELECT COUNT() FROM RULE WHERE GROUP_ID='" + groupID + "'";

    rc = sqlite3_exec(db, SQLCheck.c_str(), check_if_group_not_exists_callback, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }
}

/*
* Load attributes of interface on given path.
*/
void Database::load_interface_attributes(string path)
{
    ifstream fInterfaceClass;
    ifstream fInterfaceSubclass;
    fInterfaceClass.open(path + "/bInterfaceClass");
    fInterfaceSubclass.open(path + "/bInterfaceSubClass");
    getline(fInterfaceClass, interfaceClass);
    getline(fInterfaceSubclass, interfaceSubclass);
    fInterfaceClass.close();
    fInterfaceSubclass.close();
}

/*
* Load attributes of device on given path.
*/
void Database::load_device_attributes(string path)
{
    ifstream fDeviceClass;
    ifstream fDeviceSubclass;
    ifstream fVendor;
    ifstream fProduct;
    ifstream fPort;
    ifstream fInterfacesTotal;
    fDeviceClass.open(path + "/bDeviceClass");
    fDeviceSubclass.open(path + "/bDeviceSubClass");
    fVendor.open(path + "/idVendor");
    fProduct.open(path + "/idProduct");
    fPort.open(path + "/devpath");
    fInterfacesTotal.open(path + "/bNumInterfaces");
    getline(fDeviceClass, deviceClass);
    getline(fDeviceSubclass, deviceSubclass);
    getline(fVendor, vendor);
    getline(fProduct, product);
    getline(fPort, port);
    getline(fInterfacesTotal, interfacesTotal);
    fDeviceClass.close();
    fDeviceSubclass.close();
    fVendor.close();
    fProduct.close();
    fPort.close();
    fInterfacesTotal.close();
}

/*
* Return position of string when last folder starts.
*/
int Database::find_last_folder(const char *path)
{
    int i = strlen(path);
    for(; path[i] != '/'; i--);
    return i + 1;
}

/*
* Clear class variables, corresponding to device attributes, so new default rule can be added.
*/
void Database::clear_device_attributes()
{
    deviceClass.clear();
    deviceSubclass.clear();
    vendor.clear();
    product.clear();
    interfacesTotal.clear();
    port.clear();
    newGroup = false;
}

/*
* Clear class variables, corresponding to interface attributes, so new default rule can be added.
*/
void Database::clear_interface_attributes()
{
    interfaceClass.clear();
    interfaceSubclass.clear();
}

/*
* Check for all USB devices connected to the system, then add new rules and groups of rules based on their attributes.
*/
int Database::set_default_rules()
{
    regex device("\\d+-\\d+(\\.\\d)*");
    regex interface("\\d+-\\d+(\\.\\d)*:\\d+\\.\\d+");
    regex usbFolder("usb\\d+");
    int rulesCounter = 0;

    for (const auto &item : fs::directory_iterator("/sys/bus/usb/devices/"))
    {
        if(fs::is_directory(item))
        {
            if(regex_match(string(item.path().c_str() + find_last_folder(item.path().c_str())), device) ||
                    regex_match(string(item.path().c_str() + find_last_folder(item.path().c_str())), usbFolder))
            {
                load_device_attributes(item.path());
                groupID = nextFreeGroupID;
                newGroup = true;
                insert();
                rulesCounter++;
                clear_device_attributes();
                for (const auto &interfaceItem : fs::directory_iterator(item.path()))
                {
                    if((regex_match(string(interfaceItem.path().c_str() + find_last_folder(interfaceItem.path().c_str())), interface)))
                    {
                        load_interface_attributes(interfaceItem.path());
                        insert();
                        rulesCounter++;
                        clear_interface_attributes();
                    }
                }
                nextFreeGroupID = to_string((stoi(nextFreeGroupID)) + 1);
                groupID.clear();
            }
        }
    }
    return rulesCounter;
}

/*
* Check if there are any interface attributes when creating new rule group, because only device
* attributes are allowed in this case.
*/
void Database::attributes_check()
{
    if (!(interfaceSubclass.empty() & interfaceClass.empty()))
    {
        throw BadParamExc("-n", "Do not use interface attributes when creating new group.");
    }
}

/*
* Find and save the next free ID for groups, so new groups, created as default rules won't affect
* existing groups of rules.
*/
int Database::find_group_ID(void *data, int argc, char **argv, char **column)
{
    if(*argv == NULL)
    {
        database->nextFreeGroupID = "1";
    }
    else
    {
        database->nextFreeGroupID = string(*argv);
        database->nextFreeGroupID = to_string((stoi(database->nextFreeGroupID)) + 1);
    }

    return 0;
}

/*
* Initialization of application before any rule can be added. 
*/
void Database::init()
{
    int rc;
    string SQLfindGroupID = "SELECT MAX(GROUP_ID) FROM RULE;";
    char *errmsg = NULL;

    rc = sqlite3_exec(db, SQLfindGroupID.c_str(), find_group_ID, NULL, &errmsg);
    if (rc)
    {
        throw DatabaseExc("Could not execute command", string(errmsg));
    }
}

int main(int argc, char **argv)
{
    try
    {
        Database *database = Database::getDatabase();

        database->parse_arguments(argc, argv);
        if (!(database->createRule | database->showRules | !database->ruleIDs.empty() | database->defaultRules | database->deleteAllRules))
        {
            throw BadParamExc("", "No action specified");
        }

        database->init();

        if(database->newGroup)
        {
            database->check_if_group_not_exists();
            database->attributes_check();
        }

        if(database->createRule)
        {
            database->insert();
            cout << "Rule successfully added.\n";
        }

        if(database->showRules)
        {
            database->show();
        }

        if(database->deleteRule)
        {
            database->remove();
        }

        if(database->defaultRules)
        {
            cout << database->set_default_rules() << " rule(s) successfully created.\n";
        }
    }
    catch(exception& e)
    {
        cerr << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        cerr << "Unexpected error\n";
        return 1;
    }

    return 0;
}