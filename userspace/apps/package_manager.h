#ifndef LIMITLESS_PACKAGE_MANAGER_H
#define LIMITLESS_PACKAGE_MANAGER_H

#include "app_store.h"

// Package manager API stub
int package_manager_install(const char* package_name);
int package_manager_remove(const char* package_name);
int package_manager_update(const char* package_name);

#endif // LIMITLESS_PACKAGE_MANAGER_H
