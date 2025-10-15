#pragma once
#include "kernel.h"

/*
 * Input subsystem
 * - Keyboard and mouse events
 * - Route to kernel console/TTY or user input queues
 */

typedef struct {
    u8 scancode;
    u32 unicode;
    u8 pressed;
} key_event_t;

typedef struct {
    int dx;
    int dy;
    u8 buttons;
} mouse_event_t;

void input_init(void);
void input_push_key(const key_event_t* ev);
void input_push_mouse(const mouse_event_t* ev);