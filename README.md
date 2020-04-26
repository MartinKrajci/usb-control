# USBControl
USBControl, Linux command-line tool, is able to block potentionaly harmfull USB devices. Blocking is based on database of rules, made by user. Every allowed device need to be whitelisted.  

## Prerequisites
You need to install sqlite3 library to make database working.  
   ```sudo dnf install libsqlite3x-devel```  
or (depends on your linux distro)  
   ```sudo apt-get install libsqlite3-dev```

## Instalation
Run `make` command in project directory.  

## Running the tool
### Make database of rules
To run USBControl tool, database of rules is needed.  

#### Save new rule
```
./bin/rules -a {-d <ARG>} {-e <ARG>} {-v <ARG>} {-p <ARG>} {-i <ARG>} {-u <ARG>} {-c <ARG>} {-o <ARG>} {-g <ARG>} {-n}
```
long option: --addrule  
When adding new rule, one or more parameters can be used.

##### -d
long option: --device-class  
description: Device class, described later.  

##### -e
long option: --device-subclass  
description: Device subclass, described later.  

##### -v
long option: --vendor  
description: ID specific for vendor of device. Can be found *[here](http://www.linux-usb.org/usb.ids)* or by using `lsusb` command in linux command line.  

##### -p
long option: --product  
description: ID specific for exact kind of product, always depends on vendor ID. Can be found *[here](http://www.linux-usb.org/usb.ids)* or by using `lsusb` command in linux command line.  


##### -i
long option: --interface-class  
description: Interface class, described later.  

##### -u
long option: --interface-subclass  
description: Interface class, described later.  

##### -c
long option: --interfaces-total  
description: Total number of interfaces that connected device can have.  

##### -o
long option: --port  
description: Number of port on your personal computer.  
tip: You can find what numbers your ports have by inserting known and trusted device into your computer and typing `lsusb -t`. Find your device in listing you got from command and you will see corresponding port number.  


##### -n
long option: --new-group  
description: Create a new group. You can create groups of rules, where new groups will consist only of device attributes (everything except interface class and subclass). Later, you can add new rules to the same group with interface attributes (interface class and subclass). When comparing newly connected device to group of rules, every interface of device needs to find a match with interface rule in group of rules, while every interface rule can be used only once. This option needs to be used with `-g` parameter.  

##### -g
long option: --group-id  
description: Sets ID for new group or place new rule in existing group with ID specified in argument.  

##### Class and subclass
It's number defining function of device. Following table shows all possible classes and if they can be used in device, interface or both.

| Class number | Name                            | Device | Interface |
|:------------:|---------------------------------|:------:|:---------:|
|      00      | Function described in interface |   yes  |     no    |
|      01      | Audio                           |   no   |    yes    |
|      02      | Communications and CDC Control  |   yes  |    yes    |
|      03      | Human Interface Device          |   no   |    yes    |
|      05      | Physical                        |   no   |    yes    |
|      06      | Image                           |   no   |    yes    |
|      07      | Printer                         |   no   |    yes    |
|      08      | Mass storage                    |   no   |    yes    |
|      09      | Hub                             |   yes  |     no    |
|      0A      | CDC-Data                        |   no   |    yes    |
|      0B      | Smart card                      |   no   |    yes    |
|      0D      | Content security                |   no   |    yes    |
|      0E      | Video                           |   no   |    yes    |
|      0F      | Personal healthcare             |   no   |    yes    |
|      10      | Audio/Video devices             |   no   |    yes    |
|      11      | Billboard device class          |   yes  |     no    |
|      12      | USB Type-C Bridge class         |   no   |    yes    |
|      DC      | Diagnostic device               |   yes  |    yes    |
|      E0      | Wireless controller             |   no   |    yes    |
|      EF      | Miscellaneous                   |   yes  |    yes    |
|      FE      | Application specific            |   no   |    yes    |
|      FF      | Vendor specific                 |   yes  |    yes    |

You can learn more about classes *[here](https://www.usb.org/defined-class-codes).*  

#### Show all rules
```
./bin/rules -s
```
long option: --showrules  
Running **rules** with ` -s` parameter will print every rule saved by user.  

#### Remove rule
```
./bin/rules -x <ARG>
```
long option: --remove-rule  
You can remove any rule by typing ` -x` parameter and using ID of rule you want to remove as argument.  

#### Examples
```
./bin/rules -a -i 08 -o 3 -c 1
```  
Mass storage (for example USB flash disk) with one interface can be used on port 3.  
```
./bin/rules -a -d 00 -e 00 -i 03 -u 01 -o 1
```  
USB mouse or keyboard can be used on port 1.  
```
./bin/rules -a -d 00 -o 1 -n -g 6
./bin/rules -a -i 03 -g 6
./bin/rules -a -i E0 -g 6
```
Group of rules with ID 6 is created. Device has to have function described in interface (because -d 00) and can have one or two interfaces - HID, Wireless controller or both. Every another interface will cause that device won't be authorized.  

### Run tool itself
Run tool with
```
make run
```
You will be promped to type your password. Every new USB device from now will be checked against your database of rules. In case that new device will get authorized, message "Device auhtorized" will appear, "! Disconnecting potenctialy dangerous device !" otherwise.  

### Exiting application
You can exit this tool by pressing combination ` CTRL + C` .  

### Author
Martin Krajči  
