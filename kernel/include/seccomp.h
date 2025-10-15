#ifndef SECCOMP_H
#define SECCOMP_H

#include <stdint.h>
#include <stdbool.h>

// --- BPF (Berkeley Packet Filter) Definitions ---

// BPF Instruction format
struct bpf_insn {
    uint16_t code; // Operation code
    uint8_t  jt;   // Jump if true
    uint8_t  jf;   // Jump if false
    uint32_t k;    // Generic operand
};

// Seccomp filter structure
struct seccomp_filter {
    int len;                 // Number of instructions
    struct bpf_insn *insns;  // Pointer to BPF instructions
};

// --- Seccomp Public API ---

// Initialize the seccomp subsystem
void seccomp_init(void);

// Validate a BPF filter to ensure it's safe to run
bool seccomp_validate_filter(const struct seccomp_filter *filter);

// Run a BPF filter against a set of syscall arguments
uint32_t seccomp_run_filter(const struct seccomp_filter *filter, const struct seccomp_data *data);

// Attach a filter to a security context
int seccomp_attach_filter(uint32_t context_id, struct seccomp_filter *filter);

// Data passed to the BPF filter for evaluation
struct seccomp_data {
    int      nr;                    // System call number
    uint32_t arch;                  // Architecture
    uint64_t instruction_pointer;   // RIP
    uint64_t args[6];               // Syscall arguments
};

// --- BPF Instruction Classes ---
#define BPF_CLASS(code) ((code) & 0x07)
#define         BPF_LD          0x00
#define         BPF_LDX         0x01
#define         BPF_ST          0x02
#define         BPF_STX         0x03
#define         BPF_ALU         0x04
#define         BPF_JMP         0x05
#define         BPF_RET         0x06
#define         BPF_MISC        0x07

// --- BPF Instruction Opcodes ---
// (A subset for seccomp)
#define BPF_JEQ         0x10
#define BPF_JGT         0x20
#define BPF_JGE         0x30
#define BPF_JSET        0x40

#define BPF_K           0x00
#define BPF_X           0x08

#define BPF_W           0x00 // Word
#define BPF_ABS         0x20 // Absolute offset

// --- Seccomp Return Values ---
#define SECCOMP_RET_KILL    0x00000000U // Kill the process
#define SECCOMP_RET_TRAP    0x00030000U // Send SIGSYS
#define SECCOMP_RET_ALLOW   0x7FFF0000U // Allow the syscall
#define SECCOMP_RET_LOG     0x7FFC0000U // Log the syscall
#define SECCOMP_RET_ERRNO   0x00050000U // Return an errno

#endif // SECCOMP_H
