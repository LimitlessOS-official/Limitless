#ifndef LIMITLESS_SECURITY_H
#define LIMITLESS_SECURITY_H

#include "app_store.h"

// Security API stub for app store
int security_check_app_permissions(app_entry_t* app);
int security_enforce_sandbox(app_entry_t* app);

#endif // LIMITLESS_SECURITY_H
