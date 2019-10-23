

#ifndef __SNMP_RUN_HPP__
#define __SNMP_RUN_HPP__

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <cstdio>
#include <vector>

const int host_count=4;
const int oid_count=9;

const std::string timeout = "TIMEOUT";
pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;

std::string response[host_count][oid_count];
/*
이런 형태
{아이피, 응답, 응답, ...},
{아이피, 응답, 응답, ...},
{아이피, 응답, 응답, ...}
*/

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
    {"127.0.0.2", "public"},
    {"127.0.0.3", "public"},
    {"127.0.0.4", "public"},
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