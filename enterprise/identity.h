#ifndef LIMITLESS_IDENTITY_H
#define LIMITLESS_IDENTITY_H

int identity_ad_connect(const char* domain, const char* user, const char* pass);
int identity_ldap_connect(const char* server, const char* user, const char* pass);
int identity_sso_auth(const char* provider, const char* token);
int identity_user_add(const char* username, const char* group);
int identity_user_remove(const char* username);

#endif // LIMITLESS_IDENTITY_H
