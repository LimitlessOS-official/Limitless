#include "pam.h"
#include "../include/syscall.h"

int pam_authenticate(const char* service, const char* user, const char* password){
    (void)service;
    long uid = sys_auth_login(user, password);
    return uid >= 0 ? 0 : -1;
}

int pam_acct_mgmt(const char* service, const char* user){
    (void)service; (void)user; return 0;
}
