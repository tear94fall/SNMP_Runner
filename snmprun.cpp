#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

struct host{
    const char *name;
    const char *community;
} hosts[] = {
    {"127.0.0.1", "public"}
    {NULL}
};

struct oid_struct
{
    const char *Name;
    oid Oid[MAX_OID_LEN];
    int OidLen;
} oids[] = {
    {".1.3.6.1.2.1.1.1.0"},
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


void initialize(void){
    struct oid_struct *op = oids;
    SOCK_STARTUP;

    init_snmp("asynchapp");

    while (op->Name){
        op->OidLen = sizeof(op->Oid) / sizeof(op->Oid[0]);
        if (!read_objid(op->Name, op->Oid, (size_t *)&op->OidLen)){
            snmp_perror("read_objid");
            exit(1);
        }
        op++;
    }
}

int print_result(int status, struct snmp_session *sp, struct snmp_pdu *pdu){
char buf[1024];
    struct variable_list *vp;
    int ix;
    struct timeval now;
    struct timezone tz;
    struct tm *tm;

    gettimeofday(&now, &tz);
    tm = localtime(&now.tv_sec);
    fprintf(stdout, "%.2d:%.2d:%.2d.%.6d ", tm->tm_hour, tm->tm_min, tm->tm_sec, now.tv_usec);
    switch (status){
    case STAT_SUCCESS:
        vp = pdu->variables;

        if (pdu->errstat == SNMP_ERR_NOERROR){
            while (vp){
                snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);
                fprintf(stdout, "%s: %s\n", sp->peername, buf);
                vp = vp->next_variable;
            }
        }else{
            for (ix = 1; vp && ix != pdu->errindex; vp = vp->next_variable, ix++){
                ;
            }

            if (vp){
                snprint_objid(buf, sizeof(buf), vp->name, vp->name_length);
            }
            else{
                strcpy(buf, "(none)");
            }
            fprintf(stdout, "%s: %s: %s\n", sp->peername, buf, snmp_errstring(pdu->errstat));
        }
        return 1;

    case STAT_TIMEOUT:
        fprintf(stdout, "%s: Timeout\n", sp->peername);
        return 0;

    case STAT_ERROR:
        snmp_perror(sp->peername);
        return 0;
    }

    return 0;
}

struct session{
    struct snmp_session *sess;
    struct oid_struct *current_oid;
}

sessions[sizeof(hosts) / sizeof(hosts[0])];

int active_hosts; 

int asynch_response(int operation, struct snmp_session *sp, int reqid, struct snmp_pdu *pdu, void *magic){
    struct session *host = (struct session *)magic;
    struct snmp_pdu *req;

    if (operation == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE){
        if (print_result(STAT_SUCCESS, host->sess, pdu)){
            host->current_oid++;
            if (host->current_oid->Name){
                req = snmp_pdu_create(SNMP_MSG_GET);
                snmp_add_null_var(req, host->current_oid->Oid, host->current_oid->OidLen);
                if (snmp_send(host->sess, req)){
                    return 1;
                }
                else{
                    snmp_perror("snmp_send");
                    snmp_free_pdu(req);
                }
            }
        }
    }
    else{
        print_result(STAT_TIMEOUT, host->sess, pdu);
    }

    active_hosts--;
    return 1;
}

void asynchronous(void){
    struct session *hs;
    struct host *hp;

    for (hs = sessions, hp = hosts; hp->name; hs++, hp++){
        struct snmp_pdu *req;
        struct snmp_session sess;
        snmp_sess_init(&sess); 
        sess.version = SNMP_VERSION_2c;
        sess.peername = strdup(hp->name);
        sess.community = (u_char*)strdup(hp->community);
        sess.community_len = strlen((const char* )sess.community);
        sess.callback = asynch_response; 
        sess.callback_magic = hs;
        if (!(hs->sess = snmp_open(&sess))){
            snmp_perror("snmp_open");
            continue;
        }
        hs->current_oid = oids;
        req = snmp_pdu_create(SNMP_MSG_GET); 
        snmp_add_null_var(req, hs->current_oid->Oid, hs->current_oid->OidLen);
        if (snmp_send(hs->sess, req)){
            active_hosts++;
        }
        else{
            snmp_perror("snmp_send");
            snmp_free_pdu(req);
        }
    }

    while (active_hosts){
        int fds = 0, block = 1;
        fd_set fdset;
        struct timeval timeout;

        FD_ZERO(&fdset);
        snmp_select_info(&fds, &fdset, &timeout, &block);
        fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);

        if (fds < 0){
            perror("select failed");
            exit(1);
        }

        if (fds){
            snmp_read(&fdset);
        }
        else{
            snmp_timeout();
        }
    }

    for (hp = hosts, hs = sessions; hp->name; hs++, hp++){
        if (hs->sess){
            snmp_close(hs->sess);
        }
    }
}

int main(int argc, char **argv){
    initialize();

    printf("---------- asynchronous -----------\n");
    asynchronous();

    return 0;
}