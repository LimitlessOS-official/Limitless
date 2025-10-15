/*
 * LimitlessOS Event Bus Cleanup Module
 * Production-grade memory management and subscription cleanup
 * Copyright (c) LimitlessOS Project
 */

#include "limitless_driver_api.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// External reference to subscription map from driver_event_bus.c
extern struct subscription_hash_map subscription_map;
extern uint32_t hash_driver_name(const char *driver_name);

// Remove specific subscription by driver name and file descriptor
void remove_subscription(const char *driver_name, int fd) {
    uint32_t bucket = hash_driver_name(driver_name);
    
    pthread_rwlock_wrlock(&subscription_map.bucket_locks[bucket]);
    
    struct subscription_entry **entry_ptr = &subscription_map.buckets[bucket];
    struct subscription_entry *entry = *entry_ptr;
    
    while (entry) {
        if (strcmp(entry->driver_name, driver_name) == 0) {
            // Find and remove the specific file descriptor
            for (size_t i = 0; i < entry->subscriber_count; i++) {
                if (entry->subscriber_fds[i] == fd) {
                    // Close file descriptor
                    close(fd);
                    
                    // Shift remaining descriptors
                    for (size_t j = i; j < entry->subscriber_count - 1; j++) {
                        entry->subscriber_fds[j] = entry->subscriber_fds[j + 1];
                    }
                    entry->subscriber_count--;
                    
                    // Remove entire entry if no subscribers remain
                    if (entry->subscriber_count == 0) {
                        *entry_ptr = entry->next;
                        free(entry->subscriber_fds);
                        free(entry);
                    }
                    break;
                }
            }
            break;
        }
        entry_ptr = &entry->next;
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&subscription_map.bucket_locks[bucket]);
}

// Complete cleanup of event bus system
void cleanup_event_bus() {
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        pthread_rwlock_wrlock(&subscription_map.bucket_locks[i]);
        
        struct subscription_entry *entry = subscription_map.buckets[i];
        while (entry) {
            struct subscription_entry *next = entry->next;
            
            // Close all file descriptors
            for (size_t j = 0; j < entry->subscriber_count; j++) {
                close(entry->subscriber_fds[j]);
            }
            
            // Free allocated memory
            free(entry->subscriber_fds);
            free(entry);
            
            entry = next;
        }
        
        subscription_map.buckets[i] = NULL;
        pthread_rwlock_unlock(&subscription_map.bucket_locks[i]);
        pthread_rwlock_destroy(&subscription_map.bucket_locks[i]);
    }
}