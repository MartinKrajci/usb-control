/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        25.4.2020
*/

#include "usb-control.h"

int main()
{
    Control *con = Control::getControl();
    
    signal(SIGABRT, (__sighandler_t ) at_exit);
    signal(SIGINT, (__sighandler_t ) at_exit);
    signal(SIGTERM, (__sighandler_t ) at_exit);
    signal(SIGTSTP, (__sighandler_t ) at_exit);

    if (geteuid())
    {
        cerr << "This tool has to be run with root privileges.\n";
        return 1;
    }

    try
    {
        if (geteuid()) throw 1;
        con->read_rules();
        Netlink mon;
        mon.init_enviroment();
        mon.listen_events();
    }
    catch(bad_alloc& e)
    {
        cerr << e.what() << "\n";
        at_exit();
        return 1;
    }
    catch(SocketExc& e)
    {
        cerr << e.what() << "\n";
        at_exit();
        return 1;
    }
    catch(DatabaseExc& e)
    {
        cerr << e.what() << "\n";
        at_exit();
        return 1;
    }
    catch(GeneralExc& e)
    {
        cerr << e.what() << "\n";
        at_exit();
        return 1;
    }
    catch(...)
    {
        cerr << "Unexpected error\n";
        at_exit();
        return 1;
    }

    return 0;
}