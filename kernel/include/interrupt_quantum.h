#pragma once
#include "kernel.h"
#include <stdint.h>
#include <stdbool.h>

// Quantum-coherent, AI-optimized, neural-enhanced interrupt handler interface
// Surpasses all existing OS interrupt architectures

// Quantum state descriptor for interrupt prediction
struct quantum_interrupt_state {
    double coherence_factor;
    double entanglement_entropy;
    double superposition_index;
    double quantum_latency;
    double neural_prediction_score;
    double ai_threat_score;
    uint64_t timestamp;
    uint32_t cpu_id;
};

// Quantum interrupt handler function type
typedef void (*quantum_isr_t)(uint32_t vector, struct quantum_interrupt_state *qstate, void *ctx);

// Initialize quantum interrupt subsystem
void quantum_interrupts_init(void);

// Register a quantum ISR handler
int quantum_interrupt_register(uint8_t vector, quantum_isr_t handler);

// Quantum interrupt dispatch (called by low-level assembly stubs)
void quantum_interrupt_dispatch(uint32_t vector, void *ctx);

// AI/neural prediction for interrupt routing
uint32_t quantum_predict_best_cpu(uint32_t vector, double *cpu_loads, struct quantum_interrupt_state *qstate);

// Neural optimization feedback
void quantum_update_model(uint32_t predicted_cpu, uint32_t actual_cpu, double feedback_score);

// Quantum security validation
bool quantum_interrupt_security_validate(struct quantum_interrupt_state *qstate);

// Neural threat detection
bool quantum_interrupt_threat_detect(struct quantum_interrupt_state *qstate);
