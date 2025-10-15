#include <ctype.h>

// Check if character is alphanumeric
int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

// Check if character is alphabetic
int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Check if character is ASCII
int isascii(int c) {
    return (c >= 0 && c <= 127);
}

// Check if character is blank (space or tab)
int isblank(int c) {
    return (c == ' ' || c == '\t');
}

// Check if character is control character
int iscntrl(int c) {
    return (c >= 0 && c < 32) || (c == 127);
}

// Check if character is digit
int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

// Check if character is graphical (printable except space)
int isgraph(int c) {
    return (c > 32 && c < 127);
}

// Check if character is lowercase
int islower(int c) {
    return (c >= 'a' && c <= 'z');
}

// Check if character is printable
int isprint(int c) {
    return (c >= 32 && c < 127);
}

// Check if character is punctuation
int ispunct(int c) {
    return isgraph(c) && !isalnum(c);
}

// Check if character is whitespace
int isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || 
            c == '\r' || c == '\f' || c == '\v');
}

// Check if character is uppercase
int isupper(int c) {
    return (c >= 'A' && c <= 'Z');
}

// Check if character is hexadecimal digit
int isxdigit(int c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Convert to lowercase
int tolower(int c) {
    if (isupper(c)) {
        return c + ('a' - 'A');
    }
    return c;
}

// Convert to uppercase
int toupper(int c) {
    if (islower(c)) {
        return c - ('a' - 'A');
    }
    return c;
}

// Convert to ASCII (mask to 7 bits)
int toascii(int c) {
    return c & 0x7F;
}
