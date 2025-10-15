#include <stdint.h>
#include "../include/syscall.h"

extern int main(int argc, char** argv);

void _start(void){
    // Minimal crt0: no envp/argv parsing yet
    int rc = main(0, (char**)0);
    sys_exit(rc);
}
