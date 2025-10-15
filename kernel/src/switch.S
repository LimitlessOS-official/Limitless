.global switch_context

.text
.align 4
switch_context:
    # Standard function prologue
    pushl %ebp
    movl %esp, %ebp

    # Save callee-saved registers for the old task
    pushl %ebx
    pushl %esi
    pushl %edi

    # Get arguments from the stack
    movl 8(%ebp), %eax   # old_context_ptr
    movl 12(%ebp), %edx  # new_context_ptr

    # Save the old task's stack pointer
    movl %esp, (%eax)

    # Load the new task's stack pointer
    movl (%edx), %esp

    # Restore callee-saved registers for the new task
    popl %edi
    popl %esi
    popl %ebx

    # Standard function epilogue and return
    movl %ebp, %esp
    popl %ebp
    ret

