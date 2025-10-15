#ifndef LIMITLESS_PATCH_MANAGER_H
#define LIMITLESS_PATCH_MANAGER_H

#define PATCH_ID_MAX 32
#define PATCH_DESC_MAX 128

typedef struct {
    char id[PATCH_ID_MAX];
    char description[PATCH_DESC_MAX];
} patch_entry_t;

int patch_manager_apply(const char* patch_id);
int patch_manager_list(patch_entry_t* out, int max);
int patch_manager_add(const char* patch_id, const char* description);

#endif // LIMITLESS_PATCH_MANAGER_H
