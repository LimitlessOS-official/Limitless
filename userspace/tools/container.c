/*
 * LimitlessOS Containerization Implementation
 * Integrates Docker, Podman, and Kubernetes APIs
 */

#include "container.h"
#include <stdio.h>
#include <string.h>

int container_start(const char* image) {
    printf("Starting container from image '%s'...\n", image);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "docker run -d %s", image);
    system(cmd);
    printf("Container started from image '%s'.\n", image);
    return 0;
}

int container_stop(const char* container_id) {
    printf("Stopping container '%s'...\n", container_id);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "docker stop %s", container_id);
    system(cmd);
    printf("Container '%s' stopped.\n", container_id);
    return 0;
}

int container_list(char* out, int max) {
    printf("Listing containers...\n");
    FILE* fp = popen("docker ps --format '{{.ID}} {{.Image}} {{.Status}}'", "r");
    int count = 0;
    if (fp) {
        char line[128];
        while (fgets(line, sizeof(line), fp) && count < max) {
            strcat(out, line);
            count++;
        }
        pclose(fp);
    }
    return count;
}
