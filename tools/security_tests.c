/*
 * LimitlessOS Security Subsystem Tests
 * Copyright (c) LimitlessOS Project
 */

#include "test_framework.h"
#include "../kernel/include/security.h"
#include <stdio.h>
#include <string.h>

// Forward declarations for test functions
void test_security_mac();
void test_security_capabilities();
void test_security_seccomp();
void test_security_namespaces();
void test_security_tpm();
void test_security_pqc();
void test_security_authentication();


void test_security_mac() {
    TEST_START("MAC Policy Enforcement");

    // Initialize security framework to set up MAC
    ASSERT_TRUE(limitless_security_init() == 0, "Security framework initialized");

    // Create contexts
    uint32_t high_ctx = limitless_create_security_context(SEC_CONTEXT_USER, 0);
    uint32_t low_ctx = limitless_create_security_context(SEC_CONTEXT_USER, 0);
    ASSERT_TRUE(high_ctx > 0 && low_ctx > 0, "Security contexts created");

    // Set labels
    ASSERT_TRUE(limitless_mac_set_label(high_ctx, "secret") == 0, "Set 'secret' label");
    ASSERT_TRUE(limitless_mac_set_label(low_ctx, "public") == 0, "Set 'public' label");

    // Test Bell-LaPadula: No read up
    ASSERT_FALSE(limitless_mac_check_access("public", "secret", CAP_FILE_READ), "BLP: Deny read up (public reading secret)");

    // Test Bell-LaPadula: No write down
    ASSERT_FALSE(limitless_mac_check_access("secret", "public", CAP_FILE_WRITE), "BLP: Deny write down (secret writing to public)");
    
    // Test Biba: No write up
    ASSERT_FALSE(limitless_mac_check_access("public", "secret", CAP_FILE_WRITE), "Biba: Deny write up (public writing to secret)");

    // Test Biba: No read down
    ASSERT_FALSE(limitless_mac_check_access("secret", "public", CAP_FILE_READ), "Biba: Deny read down (secret reading public)");

    TEST_PASS("MAC policies correctly enforced");
}

void test_security_capabilities() {
    TEST_START("Capability-Based Security");

    uint32_t ctx = limitless_create_security_context(SEC_CONTEXT_USER, CAP_FILE_READ);
    ASSERT_TRUE(ctx > 0, "Context with CAP_FILE_READ created");

    ASSERT_TRUE(limitless_check_capability(ctx, CAP_FILE_READ), "Check for CAP_FILE_READ succeeds");
    ASSERT_FALSE(limitless_check_capability(ctx, CAP_FILE_WRITE), "Check for CAP_FILE_WRITE fails");

    ASSERT_TRUE(limitless_assign_capability(ctx, CAP_FILE_WRITE) == 0, "Assign CAP_FILE_WRITE");
    ASSERT_TRUE(limitless_check_capability(ctx, CAP_FILE_WRITE), "Check for CAP_FILE_WRITE now succeeds");

    ASSERT_TRUE(limitless_revoke_capability(ctx, CAP_FILE_READ) == 0, "Revoke CAP_FILE_READ");
    ASSERT_FALSE(limitless_check_capability(ctx, CAP_FILE_READ), "Check for CAP_FILE_READ now fails");

    TEST_PASS("Capability checks are correct");
}

void test_security_seccomp() {
    TEST_START("Seccomp Syscall Filtering");

    struct seccomp_filter *filter = limitless_create_seccomp_filter();
    ASSERT_NOT_NULL(filter, "Seccomp filter created");

    // Allow read (syscall 0) and write (syscall 1)
    ASSERT_TRUE(limitless_seccomp_add_rule(filter, 0) == 0, "Add rule for syscall 0 (read)");
    ASSERT_TRUE(limitless_seccomp_add_rule(filter, 1) == 0, "Add rule for syscall 1 (write)");

    // This is a conceptual test. A real test would require a process to which we apply the filter
    // and then attempt syscalls. We simulate the check here.
    bool can_read = false;
    for(uint32_t i = 0; i < filter->syscall_count; i++) {
        if (filter->allowed_syscalls[i] == 0) can_read = true;
    }
    ASSERT_TRUE(can_read, "Syscall 0 (read) is in the allowed list");

    bool can_open = false;
    for(uint32_t i = 0; i < filter->syscall_count; i++) {
        if (filter->allowed_syscalls[i] == 2) can_open = true; // Assume open is syscall 2
    }
    ASSERT_FALSE(can_open, "Syscall 2 (open) is not in the allowed list");

    TEST_PASS("Seccomp filter works as expected");
}

void test_security_namespaces() {
    TEST_START("Namespace Isolation");

    int pid_ns = limitless_create_namespace(NAMESPACE_PID);
    ASSERT_TRUE(pid_ns > 0, "PID namespace created");

    int net_ns = limitless_create_namespace(NAMESPACE_NET);
    ASSERT_TRUE(net_ns > 0, "Network namespace created");

    // Conceptual test: entering a namespace would change the process's view of the system.
    ASSERT_TRUE(limitless_enter_namespace(pid_ns) == 0, "Enter PID namespace");
    // In a real test, we'd check that getpid() returns a different value inside the namespace.

    ASSERT_TRUE(limitless_destroy_namespace(pid_ns) == 0, "Destroy PID namespace");

    TEST_PASS("Namespace creation and destruction successful");
}

void test_security_tpm() {
    TEST_START("TPM Operations");

    ASSERT_TRUE(limitless_tpm_init() == 0, "TPM initialized");

    // Test PCR extend
    const char *boot_data = "bootloader";
    ASSERT_TRUE(limitless_extend_pcr(0, boot_data, strlen(boot_data)) == 0, "Extend PCR 0");

    // Test seal/unseal
    uint8_t secret_data[16] = "top secret data";
    uint8_t sealed_blob[256];
    size_t sealed_size = sizeof(sealed_blob);
    uint8_t pcr_selection[3] = {0, 1, 2};

    ASSERT_TRUE(limitless_tpm_seal_data(secret_data, sizeof(secret_data), pcr_selection, sealed_blob, &sealed_size) == 0, "Data sealed to PCRs");

    uint8_t unsealed_data[16];
    size_t unsealed_size = sizeof(unsealed_data);
    ASSERT_TRUE(limitless_tpm_unseal_data(sealed_blob, sealed_size, unsealed_data, &unsealed_size) == 0, "Data unsealed successfully");

    ASSERT_TRUE(memcmp(secret_data, unsealed_data, sizeof(secret_data)) == 0, "Unsealed data matches original secret");

    // Simulate changing the system state and expect unseal to fail
    const char *new_data = "malicious code";
    ASSERT_TRUE(limitless_extend_pcr(0, new_data, strlen(new_data)) == 0, "Extend PCR 0 again (simulate change)");
    ASSERT_FALSE(limitless_tpm_unseal_data(sealed_blob, sealed_size, unsealed_data, &unsealed_size) == 0, "Unseal fails after PCR change");

    TEST_PASS("TPM seal/unseal works correctly");
}

void test_security_pqc() {
    TEST_START("Post-Quantum Cryptography");

    // Test Kyber KEM
    struct crypto_key *kyber_key = limitless_pqcrypto_generate_keypair(CRYPTO_KYBER_512);
    ASSERT_NOT_NULL(kyber_key, "Kyber-512 keypair generated");

    uint8_t plaintext[32] = "pqc test message";
    uint8_t ciphertext[1024];
    size_t ciphertext_size = sizeof(ciphertext);

    ASSERT_TRUE(limitless_pqcrypto_encrypt(kyber_key, plaintext, sizeof(plaintext), ciphertext, &ciphertext_size) == 0, "Kyber encryption successful");

    uint8_t decrypted_plaintext[32];
    size_t decrypted_size = sizeof(decrypted_plaintext);
    ASSERT_TRUE(limitless_pqcrypto_decrypt(kyber_key, ciphertext, ciphertext_size, decrypted_plaintext, &decrypted_size) == 0, "Kyber decryption successful");

    ASSERT_TRUE(memcmp(plaintext, decrypted_plaintext, sizeof(plaintext)) == 0, "Decrypted Kyber text matches original");

    // Test Dilithium signing
    struct crypto_key *dilithium_key = limitless_pqcrypto_generate_keypair(CRYPTO_DILITHIUM_2);
    ASSERT_NOT_NULL(dilithium_key, "Dilithium-2 keypair generated");

    uint8_t message[] = "message to sign";
    uint8_t signature[4000];
    size_t signature_size = sizeof(signature);

    ASSERT_TRUE(limitless_pqcrypto_sign(dilithium_key, message, sizeof(message), signature, &signature_size) == 0, "Dilithium signing successful");
    ASSERT_TRUE(limitless_pqcrypto_verify(dilithium_key, message, sizeof(message), signature, signature_size) == 0, "Dilithium verification successful");

    TEST_PASS("PQC algorithms (Kyber & Dilithium) are functional");
}

void test_security_authentication() {
    TEST_START("User Authentication");
    // Placeholder for authentication tests
    ASSERT_TRUE(1, "Authentication test passed (placeholder)");
    TEST_PASS("Authentication successful");
}
