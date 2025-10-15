/**
 * Architecture-specific signal handling interface
 */

#pragma once
#include "kernel.h"

/* Forward declarations */
typedef struct signal_frame signal_frame_t;
typedef struct process process_t;

/* Architecture-specific signal functions */
uint64_t arch_get_user_stack_pointer(void);
void arch_save_user_context(signal_frame_t* frame);
void arch_restore_user_context(signal_frame_t* frame);
void arch_set_user_context(uint64_t rip, uint64_t rsp, uint64_t rdi, uint64_t rsi, uint64_t rdx);
uint64_t arch_get_signal_restorer(void);

/* Process signal setup */
int arch_setup_signal_handling(process_t* proc);
int arch_handle_signal_fault(process_t* proc, uint64_t fault_addr, uint32_t error_code);

/* Exception to signal mapping */
int arch_exception_to_signal(uint32_t exception_code);
void arch_send_exception_signal(uint32_t exception_code, uint64_t fault_addr);