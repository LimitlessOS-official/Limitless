/**
 * LimitlessOS Simple Application Framework Implementation
 * Common utilities for system applications
 */

#include "simple_app_framework.h"

void app_print_header(const app_info_t* info) {
    printf("===============================================================================\n");
    printf("                               %s\n", info->name);
    printf("                               Version %s\n", info->version);
    printf("                               %s\n", info->description);
    printf("                               By %s\n", info->author);
    printf("===============================================================================\n");
    printf("\n");
}

void app_print_separator(void) {
    printf("-------------------------------------------------------------------------------\n");
}

void app_wait_for_input(void) {
    printf("\nPress Enter to continue...");
    getchar();
}

int app_get_choice(int min, int max) {
    int choice;
    char buffer[32];
    
    while (1) {
        printf("Enter your choice (%d-%d): ", min, max);
        
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            choice = atoi(buffer);
            if (choice >= min && choice <= max) {
                return choice;
            }
        }
        
        printf("Invalid choice. Please try again.\n");
    }
}