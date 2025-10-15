#pragma once
#include <stddef.h>

int pam_authenticate(const char* service, const char* user, const char* password);
int pam_acct_mgmt(const char* service, const char* user);
