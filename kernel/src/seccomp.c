#include "seccomp.h"
#include <stddef.h>

// --- BPF Virtual Machine ---

#define BPF_A     0 // Accumulator
#define BPF_X     1 // Index register

// A simple BPF interpreter
uint32_t seccomp_run_bpf(const struct bpf_insn *insns, int insn_count, const struct seccomp_data *data) {
    uint32_t acc = 0; // Accumulator
    uint32_t x = 0;   // Index register
    uint32_t pc = 0;  // Program counter

    while (pc < insn_count) {
        const struct bpf_insn *insn = &insns[pc];
        uint16_t code = insn->code;
        uint32_t k = insn->k;

        switch (code) {
            // --- Load instructions ---
            case BPF_LD | BPF_W | BPF_ABS:
                // Load a word from seccomp_data at offset k
                if (k >= sizeof(struct seccomp_data)) return SECCOMP_RET_KILL;
                acc = *((uint32_t*)((uint8_t*)data + k));
                break;

            // --- ALU operations ---
            // (Not implemented for this simple example)

            // --- Jump instructions ---
            case BPF_JMP | BPF_JEQ | BPF_K:
                pc += (acc == k) ? insn->jt : insn->jf;
                break;
            case BPF_JMP | BPF_JGT | BPF_K:
                pc += (acc > k) ? insn->jt : insn->jf;
                break;
            case BPF_JMP | BPF_JGE | BPF_K:
                pc += (acc >= k) ? insn->jt : insn->jf;
                break;
            case BPF_JMP | BPF_JSET | BPF_K:
                pc += (acc & k) ? insn->jt : insn->jf;
                break;

            // --- Return instructions ---
            case BPF_RET | BPF_K:
                return k; // Return value is in k

            // Default case for unsupported instructions
            default:
                return SECCOMP_RET_KILL;
        }
        pc++;
    }
    // If the filter doesn't return, default to killing the process.
    return SECCOMP_RET_KILL;
}


// --- Seccomp Public API Implementation ---

void seccomp_init(void) {
    // Nothing to do for this simple implementation
}

/**
 * @brief Validates a BPF filter for safety.
 *        A real implementation would check for loops, out-of-bounds jumps, etc.
 * @param filter The filter to validate.
 * @return true if the filter is considered safe, false otherwise.
 */
bool seccomp_validate_filter(const struct seccomp_filter *filter) {
    if (!filter || !filter->insns || filter->len == 0) {
        return false;
    }
    // A real validator is much more complex. This is a placeholder.
    return true;
}

/**
 * @brief Runs a validated BPF filter.
 * @param filter The filter to run.
 * @param data The syscall data to evaluate.
 * @return The action to take (e.g., SECCOMP_RET_ALLOW, SECCOMP_RET_KILL).
 */
uint32_t seccomp_run_filter(const struct seccomp_filter *filter, const struct seccomp_data *data) {
    if (!filter || !data) {
        return SECCOMP_RET_KILL;
    }
    return seccomp_run_bpf(filter->insns, filter->len, data);
}
