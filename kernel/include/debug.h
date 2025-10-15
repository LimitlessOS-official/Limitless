#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

void terminal_writestring(const char* data);
void print_dec(uint64_t value);
void print_hex(uint64_t value);

#endif // DEBUG_H
