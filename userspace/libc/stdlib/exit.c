#include <stdlib.h>
#include <syscall.h>

// Terminate the program
void exit(int status) {
    syscall1(SYS_EXIT, status);
    // Should not return
    while (1);
}

// Abnormal program termination
void abort(void) {
    exit(-1);
}
