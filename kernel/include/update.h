#ifndef LIMITLESS_UPDATE_H
#define LIMITLESS_UPDATE_H

#include "kernel.h"

/* Simple A/B slot metadata recorded at /boot/slots.meta
   key=value lines:
     active=A|B
     previous=A|B|none
     pending=0|1
     boot_success=0|1
     boot_count=N
*/

typedef struct update_meta {
    char active;     /* 'A' or 'B' */
    char previous;   /* 'A', 'B', or '-' for none */
    int pending;     /* 1 if an update has been applied but not switched */
    int boot_success;/* 1 if current active slot has marked success */
    int boot_count;  /* increments on boot until success */
} update_meta_t;

/* Initialize update system: load metadata and handle rollback if needed */
int update_init(void);

/* Get current metadata (copy) */
int update_get_meta(update_meta_t* out);

/* Begin an update: mark target inactive slot as pending destination */
int update_begin(char* out_target_slot /* optional: returns 'A' or 'B' */);

/* Apply OTA package from path to inactive slot.
   OTA format (tar):
     - root.img (raw rootfs image)
     - ota.sha256 (hex sha256 of root.img) */
int update_apply_ota(const char* ota_path);

/* Finalize update: switch active to updated slot and mark boot_success=0 */
int update_finalize(void);

/* Mark current boot as successful */
int update_mark_success(void);

/* Roll back to previous slot (if available) */
int update_rollback(void);

#endif /* LIMITLESS_UPDATE_H */
