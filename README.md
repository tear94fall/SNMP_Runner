# SNMP_Runner

### if you want to run in your computer, you must modified snmprun.hpp line 13
hosts[] = {  
    {"127.0.0.1", "public"}, 
    {NULL}  
};


### if error occured like this:  
[root@centos7]$ ./snmprun  
**./snmprun: error while loading shared libraries: libnetsnmp.so.30:cannot open shared object file: No such file or directory**  
[root@centos7]$ **export LD_LIBRARY_PATH=/usr/local/lib**  
[root@centos7]$ ./snmprun  
---------- synchronous -----------  
10:21:48.386554 xxx.xxx.xxx.xxx: SNMPv2-MIB::sysDescr.0 = STRING: XXXXXX XXXX 
