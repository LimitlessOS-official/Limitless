#ifndef LIMITLESS_CONTAINER_H
#define LIMITLESS_CONTAINER_H

int container_start(const char* image);
int container_stop(const char* container_id);
int container_list(char* out, int max);

#endif // LIMITLESS_CONTAINER_H
