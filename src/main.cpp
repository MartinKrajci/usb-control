/*
* Project name:                     USB Control
* Author:                           Martin Krajči
* Last date of modification:        2.6.2020
* Description of file:
*
* Main function of the application for controlling newly connected devices.
*
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
        if (geteuid()) throw BadPrivilegesExc("This tool has to be run with root privileges.");
        con->read_rules();
        Netlink mon;
        mon.init_enviroment();
        mon.listen_events();
    }
    catch(exception& e)
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