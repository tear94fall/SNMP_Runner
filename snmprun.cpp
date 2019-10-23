
#include "snmprun.hpp"

void initialize(void);
int print_result(int status, struct snmp_session *sp, struct snmp_pdu *pdu);
int asynch_response(int operation, struct snmp_session *sp, int reqid, struct snmp_pdu *pdu, void *magic);
void set_response(char* ipaddr, char* buf);
void* asynchronous(void*);
void* receiver(void*); 

int main(int argc, char **argv){
    for(int i=0;i<host_count;i++){
        for(int j=0;j<oid_count;j++){
            response[i][j]="";
        }
    }

    pthread_t sender_thread;
    pthread_t receiver_thread;

    pthread_create(&sender_thread, NULL, asynchronous, NULL);
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    initialize();

    printf("-------------- start --------------\n");
    pthread_join(sender_thread, NULL);
    pthread_join(receiver_thread, NULL);

    return 0;
}

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

void set_response(char* ipaddr, struct variable_list *vp, struct snmp_pdu *pdu){
    bool is_exist123 = false;
    int col_index123 = -1;

    char result_buffer[2048];
    memset(result_buffer, 0x00, sizeof(2048));

    strcat(result_buffer, ipaddr);
    strcat(result_buffer, " : ");

    pthread_mutex_lock(&pmutex);
    // 아이피가 이미 존재하는지 확인
    for(int i=0;i<host_count;i++){
        if(strcmp(response[i][0].c_str(),ipaddr)==0){
            // 존재하면 인덱스와 플래그 값을 바꿈
            is_exist123 = true;
            col_index123 = i;
        }
    }

    // 이미 아이피가 존재하는 경우
    if(is_exist123){
        // 존재하는 경우 oid 값의 빈값을 찾음
        int row_index=-1;
        for(int i=2;i<oid_count; i++){
            if(response[col_index123][i]==""){
                vp = pdu->variables;
                char temp_buf[1024];
                snprint_variable(temp_buf, sizeof(temp_buf), vp->name, vp->name_length, vp);
                row_index=i;

                strcat(result_buffer, temp_buf);
                response[col_index123][row_index] = result_buffer;

                break;
            }
        }

    }else{
        for(int i=0;i<host_count;i++){
            if(response[i][0]==""){
                response[i][0]=ipaddr;

                vp = pdu->variables;
                char temp_buf[1024];
                snprint_variable(temp_buf, sizeof(temp_buf), vp->name, vp->name_length, vp);

                strcat(result_buffer, temp_buf);
                response[i][1] = result_buffer;
                break;
            }
        }
    }
    memset(result_buffer, 0x00, sizeof(result_buffer));
    
    pthread_mutex_unlock(&pmutex);
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
    //fprintf(stdout, "%.2d:%.2d:%.2d.%.6d ", tm->tm_hour, tm->tm_min, tm->tm_sec, now.tv_usec);

    char ip_addr[100];
    strcpy(ip_addr, sp->peername);

    switch (status){
    case STAT_SUCCESS:
        vp = pdu->variables;

        if (pdu->errstat == SNMP_ERR_NOERROR){
            while (vp){
                snprint_variable(buf, sizeof(buf), vp->name, vp->name_length, vp);
                // fprintf(stdout, "%s: %s\n", sp->peername, buf);
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
            // fprintf(stdout, "%s: %s: %s\n", sp->peername, buf, snmp_errstring(pdu->errstat));
        }

        set_response(ip_addr, vp, pdu);

        return 1;

    case STAT_TIMEOUT:{
        bool is_exist123 = false;
        int col_index123 = -1;


        pthread_mutex_lock(&pmutex);
        // 아이피가 이미 존재하는지 확인
        for(int i=0;i<host_count;i++){
            if(strcmp(response[i][0].c_str(),ip_addr)==0){
                // 존재하면 인덱스와 플래그 값을 바꿈
                is_exist123 = true;
                col_index123 = i;
            }
        }

        // 이미 아이피가 존재하는 경우
        if(is_exist123){
        }else{
            for(int i=0;i<host_count;i++){
                if(response[i][0]==""){
                    response[i][0]=ip_addr;
                    response[i][1] = "TIME OUT";
                    break;
                }
            }
        }   
        
        pthread_mutex_unlock(&pmutex);
        return 0;
    }
    case STAT_ERROR:
        snmp_perror(sp->peername);
        return 0;
    }

    return 0;
}

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

void* asynchronous(void*){
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

void* receiver(void*){
    int cnt=0;
    while(1){
        if(cnt==host_count){
            printf("--------------- end ---------------\n");
            break;
        }

        for (int i = 0; i < host_count; i++){
            pthread_mutex_lock(&pmutex);
            bool is_end=false;
            if(response[i][1] == "TIME OUT"){
                is_end=true;
            }else{
                for(int j=0;j<oid_count;j++){
                    if(response[i][j]!=""&&response[i][j]!="null"){
                        is_end = true;
                    }else{
                        is_end = false;
                    }
                }
            }

            if(is_end){
                for (int j = 0; j < oid_count; j++){
                    if (j == 0){
                        printf("*** num.%d host is : %s ***\n", i + 1, response[i][0].c_str());
                    }
                    if (j != 0 && response[i][j] != "" && response[i][j] != "null"){
                        if(response[i][j]=="TIME OUT"){
                            printf("TIME OUT\n");
                        }else{
                            printf("%d.oid is : %s\n", j, response[i][j].c_str());
                        }
                        response[i][j] = "null";
                    }
                }
                cnt++;
            }

            pthread_mutex_unlock(&pmutex);
        }
    }
}