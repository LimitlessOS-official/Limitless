#include "service_manager.h"
#include <stdio.h>
#include <string.h>

static service_t g_services[SM_MAX_SERVICES];
static size_t g_service_count = 0;

void sm_init(void) {
    memset(g_services, 0, sizeof(g_services));
    g_service_count = 0;
}

static int find_service_index(const char* name) {
    if (!name) return -1;
    for (size_t i=0;i<g_service_count;i++) {
        if (g_services[i].desc.name && strcmp(g_services[i].desc.name, name)==0) return (int)i;
    }
    return -1;
}

int sm_register(const service_desc_t* list) {
    if (!list) return -1;
    for (const service_desc_t* d = list; d->name; ++d) {
        if (g_service_count >= SM_MAX_SERVICES) return -2;
        g_services[g_service_count].desc = *d;
        g_services[g_service_count].state = SM_STOPPED;
        g_services[g_service_count].handle = NULL;
        g_service_count++;
    }
    return 0;
}

static int start_one(size_t idx) {
    service_t* s = &g_services[idx];
    if (s->state == SM_RUNNING) return 0;
    s->state = SM_STARTING;

    /* Ensure deps are running */
    for (size_t i=0; i<SM_MAX_DEPS && s->desc.deps[i]; ++i) {
        int dep_idx = find_service_index(s->desc.deps[i]);
        if (dep_idx < 0) {
            printf("[sm] missing dependency '%s' for '%s'\n", s->desc.deps[i], s->desc.name);
            s->state = SM_FAILED; return -3;
        }
        int rc = start_one((size_t)dep_idx);
        if (rc != 0) { s->state = SM_FAILED; return rc; }
    }

    /* Resolve persona for the target path */
    persona_id_t id = 0;
    if (persona_resolve_for_path(s->desc.path, &id) != PERSONA_OK) {
        printf("[sm] no persona for '%s'\n", s->desc.path);
        s->state = SM_FAILED; return -4;
    }
    if (persona_launch(id, s->desc.path, &s->handle) != PERSONA_OK) {
        printf("[sm] launch failed for '%s'\n", s->desc.name);
        s->state = SM_FAILED; return -5;
    }
    s->state = SM_RUNNING;
    printf("[sm] started '%s' -> %s\n", s->desc.name, s->desc.path);
    return 0;
}

int sm_start_all(void) {
    for (size_t i=0;i<g_service_count;i++) {
        int rc = start_one(i);
        if (rc != 0) return rc;
    }
    return 0;
}

void sm_status_dump(void) {
    puts("[sm] service status:");
    for (size_t i=0;i<g_service_count;i++) {
        const char* st = "?";
        switch (g_services[i].state) {
            case SM_STOPPED: st="stopped"; break;
            case SM_STARTING: st="starting"; break;
            case SM_RUNNING: st="running"; break;
            case SM_FAILED: st="failed"; break;
        }
        printf(" - %s: %s\n", g_services[i].desc.name, st);
    }
}

void sm_stop_all(void) {
    for (size_t i=0;i<g_service_count;i++) {
        if (g_services[i].handle) {
            persona_close(g_services[i].handle);
            g_services[i].handle = NULL;
        }
        g_services[i].state = SM_STOPPED;
    }
}