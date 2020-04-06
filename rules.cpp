#include <iostream>
#include <string>
#include <cstring>

#include <sqlite3.h>
#include <getopt.h>

using namespace std;

static int callback(void *data, int argc, char **argv, char **column)
{
   for(int i = 0; i<argc; i++)
   {
      printf("%s = %s | ", column[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");

   return 0;
}

int main(int argc, char **argv)
{
    sqlite3 *db;
    char *errmsg = NULL;
    int opt = 0;
    int opt_index = 0;
    bool createRule = false;
    bool showRules = false;
    string deviceClass;
    string interfaceClass;
    string deviceSubclass;
    string interfaceSubclass;
    string deviceSublclass;
    //string hid;
    string vendor;
    string product;
    string interfacesTotal;
    string ruleID;
    string port;
    string partWithComma = ", \'";
    string partWithoutComma = "\'";
    string comma = ", ";
    bool firstEntry = true;
    int rc;

    string SQLCreate = "CREATE TABLE IF NOT EXISTS RULE("  \
        "ID                 INTEGER PRIMARY KEY," \
        "DEVICE_CLASS           TEXT," \
        "DEVICE_SUBCLASS         TEXT," \
        "VENDOR                 TEXT," \
        "PRODUCT                TEXT," \
        "INTERFACE_CLASS        TEXT," \
        "INTERFACE_SUBCLASS     TEXT," \
        "INTERFACE_COUNT        INT," \
        "PORT                   INT);";
        /*"HID                    INT);" ;*/

    string SQLSelect = "SELECT * FROM RULE";

    const struct option longopts[] = 
    {
        {"addrule", no_argument, 0, 'a'},
        {"showrules", no_argument, 0, 's'},
        {"device-class", required_argument, 0, 'd'},
        {"device-subclass", required_argument, 0, 'e'},
        {"vendor", required_argument, 0, 'v'},
        {"product", required_argument, 0, 'p'},
        {"interface-class", required_argument, 0, 'i'},
        {"interface-subclass", required_argument, 0, 'u'},
        {"interfaces-total", required_argument, 0, 't'},
        {"port", required_argument, 0, 'o'},
        /*{"hid", required_argument, 0, 'h'},*/
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "asx:d:i:h:e:u:v:p:c:o:", longopts, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'a':
            createRule = true;
            break;
        case 'x':
            ruleID = string(optarg);
            break;
        case 's':
            showRules = true;
            break;
        case 'd':
            deviceClass = string(optarg);
            break;
        case 'e':
            deviceSubclass = string(optarg);
            break;
        case 'i':
            interfaceClass = string(optarg);
            break;
        case 'u':
            interfaceSubclass = string(optarg);
            break;
        /*case 'h':
            hid = string(optarg);
            break;*/
        case 'v':
            vendor = string(optarg);
            break;
        case 'p':
            product = string(optarg);
            break;
        case 'c':
            interfacesTotal = string(optarg);
            break;
        case 'o':
            port = string(optarg);
            break;
        default:
            break;
        }
    }

    rc = sqlite3_open("db", &db);
    if (rc) 
    {
        cerr << "Could not open database\n";
        return 1;
    }

    rc = sqlite3_exec(db, SQLCreate.c_str(), callback, NULL, &errmsg);
    if (rc)
    {
        cerr << "Could not execute command, " << errmsg << "\n";
        return 1;
    }

    if(createRule)
    {
        string SQLInsert =  "INSERT INTO RULE (";
        string SQLInsertValues = "VALUES (";

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

        /*if (!hid.empty())
        {
            SQLInsert = SQLInsert + ", " + "HID";
            SQLInsertValues = SQLInsertValues + ", " + hid;
        }*/

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

        SQLInsert += ") " + SQLInsertValues + ");";

        //cout << SQLInsert << "\n";
        
        rc = sqlite3_exec(db, SQLInsert.c_str(), callback, NULL, &errmsg);
        if (rc)
        {
            cerr << "Could not execute command, " << errmsg << "\n";
            return 1;
        }
        cout << "Rule successfully added\n";
    }

    if(!ruleID.empty())
    {
        string SQLDelete =  "DELETE FROM RULE WHERE ID = " + ruleID + ";";

        rc = sqlite3_exec(db, SQLDelete.c_str(), callback, NULL, &errmsg);
        if (rc)
        {
            cerr << "Could not execute command, " << errmsg << "\n";
            return 1;
        }
    }

    if(showRules)
    {
        rc = sqlite3_exec(db, SQLSelect.c_str(), callback, NULL, &errmsg);
        if (rc)
        {
            cerr << "Could not execute command, " << errmsg << "\n";
            return 1;
        }
    }

    return 0;
}