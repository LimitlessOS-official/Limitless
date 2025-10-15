#ifndef LIMITLESS_PACKAGE_MANAGER_H
#define LIMITLESS_PACKAGE_MANAGER_H

#define PACKAGE_NAME_MAX 64
#define PACKAGE_VERSION_MAX 32

typedef struct {
    char name[PACKAGE_NAME_MAX];
    char version[PACKAGE_VERSION_MAX];
} package_entry_t;

int package_manager_install(const char* package_name);
int package_manager_remove(const char* package_name);
int package_manager_update(const char* package_name);
int package_manager_verify_signature(const char* package_name);
int package_manager_list(package_entry_t* out, int max);
int package_manager_add(const char* package_name, const char* version);

#endif // LIMITLESS_PACKAGE_MANAGER_H
