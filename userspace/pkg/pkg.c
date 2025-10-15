/**
 * LimitlessOS Package Manager - Main Entry Point
 * 
 * This is a minimal entry point that delegates to the full
 * package manager implementation in limitless_pkg.c
 * 
 * Usage: pkg [command] [options]
 */

#include <stdio.h>
#include <stdlib.h>

/* External declaration from limitless_pkg.c */
extern int limitless_pkg_main(int argc, char **argv);

int main(int argc, char **argv) {
    /* Delegate to the full implementation */
    return limitless_pkg_main(argc, argv);
}
