#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* name;      // package name
    const char* version;   // version string
    const char* arch;      // optional arch
} pkg_manifest_t;

// Install package from tarball to root_dir ("/")
int pkg_install_tar(const char* tar_path, const char* root_dir);

// Remove package by name (uses manifest db under /var/db/pkg)
int pkg_remove(const char* name, const char* root_dir);

// List installed packages
int pkg_list(const char* root_dir);

// CLI entry
int pkg_main(int argc, char** argv);

#ifdef __cplusplus
}
#endif
