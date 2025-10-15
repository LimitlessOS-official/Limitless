#pragma once
#include "kernel.h"

/* Interrupt and IPI handling */

typedef void (*isr_handler_t)(u64 vec, u64 err_code, void* ctx);

void interrupts_init(void);

/* Register an ISR handler for a vector (0-255). Returns 0 on success. */
int  interrupt_register(u8 vector, isr_handler_t handler);

/* Timer tick hookup by arch timer ISR */
void interrupt_timer_tick(void);

/* Request reschedule to be performed on interrupt exit */
void interrupt_request_reschedule(void);