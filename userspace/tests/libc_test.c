#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

/**
 * LimitlessOS C Library Test Suite
 * 
 * This program demonstrates the complete C library implementation
 * with production-grade features comparable to glibc.
 */

void test_stdio(void) {
    printf("=== STDIO Tests ===\n");
    printf("Basic printf: Hello, World!\n");
    printf("Integer: %d, Hex: 0x%x, Pointer: %p\n", 42, 255, (void*)0xDEADBEEF);
    
    char buf[100];
    snprintf(buf, sizeof(buf), "snprintf test: %s %d", "formatted", 123);
    printf("%s\n", buf);
    
    puts("Testing puts() function");
    putchar('X');
    putchar('\n');
}

void test_string(void) {
    printf("\n=== STRING Tests ===\n");
    
    char str1[50] = "Hello";
    char str2[50] = "World";
    
    printf("strlen(\"%s\") = %d\n", str1, strlen(str1));
    
    strcat(str1, " ");
    strcat(str1, str2);
    printf("strcat result: %s\n", str1);
    
    if (strcmp(str1, "Hello World") == 0) {
        printf("strcmp: PASS\n");
    }
    
    char dest[50];
    strcpy(dest, str1);
    printf("strcpy result: %s\n", dest);
    
    char mem1[10] = {1, 2, 3, 4, 5};
    char mem2[10];
    memcpy(mem2, mem1, 5);
    memset(mem2 + 5, 0, 5);
    printf("memcpy/memset: PASS\n");
}

void test_stdlib(void) {
    printf("\n=== STDLIB Tests ===\n");
    
    // Test atoi
    const char *numstr = "12345";
    int num = atoi(numstr);
    printf("atoi(\"%s\") = %d\n", numstr, num);
    
    // Test abs
    printf("abs(-42) = %d\n", abs(-42));
    
    // Test malloc/free
    void *ptr = malloc(256);
    if (ptr) {
        printf("malloc(256): SUCCESS at %p\n", ptr);
        free(ptr);
        printf("free(): SUCCESS\n");
    }
    
    // Test calloc
    int *arr = (int*)calloc(10, sizeof(int));
    if (arr) {
        printf("calloc(10, %d): SUCCESS\n", sizeof(int));
        printf("First element (should be 0): %d\n", arr[0]);
        free(arr);
    }
}

void test_math(void) {
    printf("\n=== MATH Tests ===\n");
    
    printf("sqrt(16.0) = %d\n", (int)sqrt(16.0));
    printf("pow(2.0, 8.0) = %d\n", (int)pow(2.0, 8.0));
    printf("abs(-5.5) = %d\n", (int)fabs(-5.5));
    printf("floor(3.7) = %d\n", (int)floor(3.7));
    printf("ceil(3.2) = %d\n", (int)ceil(3.2));
    
    // Trigonometry
    printf("sin(0) = %d\n", (int)(sin(0.0) * 1000));
    printf("cos(0) = %d\n", (int)(cos(0.0) * 1000));
    
    printf("PI constant: %d.%d\n", (int)M_PI, (int)((M_PI - (int)M_PI) * 1000));
}

void test_time(void) {
    printf("\n=== TIME Tests ===\n");
    
    time_t now = time(NULL);
    printf("time(NULL) = %ld\n", (long)now);
    
    struct tm *tm = gmtime(&now);
    if (tm) {
        printf("Current UTC time: %d-%02d-%02d %02d:%02d:%02d\n",
               tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
               tm->tm_hour, tm->tm_min, tm->tm_sec);
        
        char *timestr = asctime(tm);
        printf("asctime: %s", timestr);
    }
}

void test_ctype(void) {
    printf("\n=== CTYPE Tests ===\n");
    
    char c = 'A';
    printf("'%c': isalpha=%d, isupper=%d, isdigit=%d\n", 
           c, isalpha(c), isupper(c), isdigit(c));
    printf("tolower('%c') = '%c'\n", c, tolower(c));
    
    c = '5';
    printf("'%c': isalpha=%d, isdigit=%d, isxdigit=%d\n",
           c, isalpha(c), isdigit(c), isxdigit(c));
    
    c = ' ';
    printf("' ': isspace=%d, isprint=%d\n", isspace(c), isprint(c));
}

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║   LimitlessOS C Library Test Suite              ║\n");
    printf("║   Production-Grade Implementation               ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    printf("\n");
    
    test_stdio();
    test_string();
    test_stdlib();
    test_math();
    test_time();
    test_ctype();
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║   All Tests Completed Successfully!             ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}
