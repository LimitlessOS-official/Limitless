#ifndef LIMITLESS_REMOTE_MGMT_H
#define LIMITLESS_REMOTE_MGMT_H

int remote_mgmt_inventory(void);
int remote_mgmt_enforce_policy(const char* policy);
int remote_mgmt_remote_wipe(const char* device_id);
int remote_mgmt_mdm_enroll(const char* device_id);

#endif // LIMITLESS_REMOTE_MGMT_H
