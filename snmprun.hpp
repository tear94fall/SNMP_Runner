

#ifndef __SNMP_RUN_HPP__
#define __SNMP_RUN_HPP__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <vector>

typedef struct host{
    const char *name;
    const char *community;
}host;

typedef struct oid_struct
{
    const char *Name;
    oid Oid[MAX_OID_LEN];
    int OidLen;
}oid_struct;

host hosts[] = {
    {"127.0.0.1", "public"},
    {NULL}
};

oid_struct oids[] = {
    {".1.3.6.1.2.1.1.2.0"},
    {".1.3.6.1.2.1.1.3.0"},
    {".1.3.6.1.2.1.1.4.0"},
    {".1.3.6.1.2.1.1.5.0"},
    {".1.3.6.1.2.1.1.6.0"},
    {".1.3.6.1.2.1.1.7.0"},
    {".1.3.6.1.2.1.1.2.0"},
    {".1.3.6.1.2.1.2.1.0"},
    {NULL}
};

struct session{
    struct snmp_session *sess;
    struct oid_struct *current_oid;
}

sessions[sizeof(hosts) / sizeof(hosts[0])];

int active_hosts; 

#endif