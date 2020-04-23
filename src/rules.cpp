#include "rules.h"

#define FAILED 1

using namespace std;

Database *Database::database;

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
        "PORT                   INT," \
        "GROUP_ID                  INT," \
        "IS_GROUP                  INT);";

    rc = sqlite3_open("database/db", &db);
    if (rc) 
    {
        cerr << "Could not open database\n";
        throw FAILED;
    }

    rc = sqlite3_exec(db, SQLCreate.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }
}

Database *Database::getDatabase()
{
    if (database == NULL)
    {
        database = new Database;
    }

    return database;
}

void Database::checkIfTwoHex(string arg)
{
    regex twoHex("[0-9a-fA-F]{2}");

    if (!regex_match(string(arg), twoHex))
    {
        cerr << "Wrong input! Expecting argument in form of two hex numbers.\n";
        throw FAILED;
    }
}

void Database::checkIfFourHex(string arg)
{
    regex fourHex("[0-9a-fA-F]{4}");

    if (!regex_match(string(arg), fourHex))
    {
        cerr << "Wrong input! Expecting argument in form of four hex numbers.\n";
        throw FAILED;
    }
}

void Database::checkIfNum(string arg)
{
    regex num("\\d+");

    if (!regex_match(string(arg), num))
    {
        cerr << "Wrong input! Expecting argument of type number.\n";
        throw FAILED;
    }
}

void Database::parseArguments(int argc, char **argv)
{
    int opt = 0;
    int opt_index = 0;

    const struct option longopts[] = 
    {
        {"addrule", no_argument, 0, 'a'},
        {"showrules", no_argument, 0, 's'},
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
        {0, 0, 0, 0}
    };


    while ((opt = getopt_long(argc, argv, "asnx:d:i:h:e:u:v:p:c:o:g:", longopts, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'a':
            createRule = true;
            break;
        case 'x':
            checkIfNum(string(optarg));
            ruleID = string(optarg);
            break;
        case 's':
            showRules = true;
            break;
        case 'd':
            checkIfTwoHex(string(optarg));
            deviceClass = string(optarg);
            hasAttribute = true;
            break;
        case 'e':
            checkIfTwoHex(string(optarg));
            deviceSubclass = string(optarg);
            hasAttribute = true;
            break;
        case 'i':
            checkIfTwoHex(string(optarg));
            interfaceClass = string(optarg);
            hasAttribute = true;
            break;
        case 'u':
            checkIfTwoHex(string(optarg));
            interfaceSubclass = string(optarg);
            hasAttribute = true;
            break;
        case 'v':
            checkIfFourHex(string(optarg));
            vendor = string(optarg);
            hasAttribute = true;
            break;
        case 'p':
            checkIfFourHex(string(optarg));
            product = string(optarg);
            hasAttribute = true;
            break;
        case 'c':
            checkIfNum(string(optarg));
            interfacesTotal = string(optarg);
            hasAttribute = true;
            break;
        case 'o':
            checkIfNum(string(optarg));
            port = string(optarg);
            hasAttribute = true;
            break;
        case 'g':
            checkIfNum(string(optarg));
            groupID = string(optarg);
            break;
        case 'n':
            newGroup = true;
            break;
        default:
            throw FAILED;
            break;
        }
    }
}

int Database::callback(void *data, int argc, char **argv, char **column)
{
    for(int i = 0; i<argc; i++)
    {
        if(string(column[i]) != "IS_GROUP")
        {
            printf("%s = %s | ", column[i], argv[i] ? argv[i] : "*");
        }
    }
    printf("\n");

   return 0;
}

int Database::checkIfGroupNotExistsCallback(void *data, int argc, char **argv, char **azColName)
{
    if(**argv == '0')
    {
        cerr << "Group with this ID do not exists! Use '-n' parameter to create a new one.\n";
        throw FAILED;
    }
    return 0;
}

void Database::checkIfGroupNotExists()
{
    int rc;
    char *errmsg = NULL;
    string SQLCheck;

    SQLCheck = "SELECT COUNT() FROM RULE WHERE GROUP_ID='" + groupID + "'";

    rc = sqlite3_exec(db, SQLCheck.c_str(), checkIfGroupNotExistsCallback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }
}

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

    if (!(createRule & hasAttribute))
    {
        cerr << "Missing arguments for rule creation!\n";
        throw FAILED;
    }

    if (!groupID.empty() & !newGroup)
    {
        if(!deviceClass.empty() | !deviceSubclass.empty() | !vendor.empty() |
           !product.empty() | !interfacesTotal.empty() | !port.empty())
        {
            cerr << "Do not use device attributes when group already exists.\n";
            throw FAILED;
        }
        checkIfGroupNotExists();
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
            SQLInsertValues = SQLInsertValues + partWithoutComma + deviceClass + partWithoutComma;
        }
        else
        {
            SQLInsert = SQLInsert + comma + "DEVICE_SUBCLASS";
            SQLInsertValues = SQLInsertValues + partWithComma + deviceClass + "\'";   
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
            SQLInsertValues = SQLInsertValues + partWithoutComma + interfaceClass + partWithoutComma;                
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
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }
    cout << "Rule successfully added\n";
}

void Database::show()
{
    int rc;
    char *errmsg = NULL;
    string SQLSelect = "SELECT * FROM RULE";
    
    rc = sqlite3_exec(db, SQLSelect.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }
}

void Database::remove()
{
    int rc;
    char *errmsg = NULL;
    string SQLDelete =  "DELETE FROM RULE WHERE ID = " + ruleID + ";";

    rc = sqlite3_exec(db, SQLDelete.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }
}

int Database::checkIfGroupExistsCallback(void *data, int argc, char **argv, char **azColName)
{
    if(**argv != '0')
    {
        cerr << "Group with this ID already exists\n";
        throw FAILED;
    }
    return 0;
}

void Database::checkIfGroupExists()
{
    int rc;
    char *errmsg = NULL;
    string SQLCheck;

    if(groupID.empty())
    {
        cerr << "Group ID needs to be specified when creating new group\n";
        throw FAILED;
    }

    SQLCheck = "SELECT COUNT() FROM RULE WHERE GROUP_ID='" + groupID + "'";

    rc = sqlite3_exec(db, SQLCheck.c_str(), checkIfGroupExistsCallback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        throw FAILED;
    }
}

void Database::attributesCheck()
{
    if (!(interfaceSubclass.empty() & interfaceClass.empty()))
    {
        cerr << "Do not use interface attributes when creating new group!\n";
        throw FAILED;
    }
}

int main(int argc, char **argv)
{
    try
    {
        Database *database = Database::getDatabase();

        database->parseArguments(argc, argv);
        if (!(database->createRule | database->showRules | !database->ruleID.empty()))
        {
            cerr << "No action specified.\n";
            return 1;
        }

        if(database->newGroup)
        {
            database->checkIfGroupExists();
            database->attributesCheck();
        }

        if(database->createRule)
        {
            database->insert();
        }

        if(database->showRules)
        {
            database->show();
        }

        if(!database->ruleID.empty())
        {
            database->remove();
        }
    }
    catch (int)
    {
        return 1;
    }

    return 0;
}