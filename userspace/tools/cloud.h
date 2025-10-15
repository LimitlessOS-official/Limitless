#ifndef LIMITLESS_CLOUD_H
#define LIMITLESS_CLOUD_H

int cloud_sync(const char* remote);
int cloud_backup(const char* remote);
int cloud_federated_identity(const char* provider, const char* token);
int cloud_deploy(const char* target);

#endif // LIMITLESS_CLOUD_H
