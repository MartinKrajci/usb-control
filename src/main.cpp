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

    try
    {
        con->read_rules();
        Netlink mon;
        mon.init_enviroment();
        mon.listen_events();
    }
    catch(ifstream::failure)
    {
        cerr << "Could not open file. This tool has to be run with root privileges.\n";
        at_exit();
        return 1;
    }
    catch(bad_alloc)
    {
        cerr << "Could not allocate memory.\n";
        at_exit();
        return 1;
    }
    catch(int)
    {
        at_exit();
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unexpected error.\n";
        at_exit();
        return 1;
    }

    return 0;
}