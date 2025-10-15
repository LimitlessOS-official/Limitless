#ifndef LIMITLESS_UPDATE_MANAGER_H
#define LIMITLESS_UPDATE_MANAGER_H

int update_manager_run(void);
int update_manager_rollback(const char* package_name);
int update_manager_set_policy(const char* policy);

#endif // LIMITLESS_UPDATE_MANAGER_H
