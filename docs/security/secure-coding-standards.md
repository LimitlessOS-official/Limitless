# LimitlessOS Secure Coding Standards

**Version**: 1.0  
**Last Updated**: 2025-10-15  
**Status**: Active

## Overview

This document defines the secure coding standards for LimitlessOS kernel and system components. All code contributions must adhere to these standards.

## Language and Compiler

### C Language Standard
- **Required**: C11 (ISO/IEC 9899:2011)
- **Allowed**: GNU extensions where necessary for kernel development
- **Prohibited**: Pre-C99 features (K&R style declarations, etc.)

### Compiler Requirements
- **GCC**: Version 13.x or later
- **Clang**: Version 18.x or later
- **Flags**: All mandatory security flags must be enabled (see Makefile)

## Security Requirements

### 1. Memory Safety

#### Buffer Overflow Prevention
```c
// ❌ WRONG: Unsafe string copy
char dest[64];
strcpy(dest, user_input);

// ✅ CORRECT: Bounded copy with null termination
char dest[64];
strncpy(dest, user_input, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';

// ✅ BETTER: Use safe string functions
strlcpy(dest, user_input, sizeof(dest));
```

#### Integer Overflow Protection
```c
// ❌ WRONG: Unchecked arithmetic
size_t total = size1 + size2;
void *ptr = malloc(total);

// ✅ CORRECT: Check for overflow
size_t total;
if (__builtin_add_overflow(size1, size2, &total)) {
    return -EOVERFLOW;
}
void *ptr = malloc(total);
```

#### Variable Length Arrays Prohibited
```c
// ❌ WRONG: VLA on stack
void process(int n) {
    int array[n];  // PROHIBITED
}

// ✅ CORRECT: Fixed size or heap allocation
void process(int n) {
    int array[MAX_SIZE];
    if (n > MAX_SIZE) return -EINVAL;
}
```

### 2. Input Validation

#### Always Validate User Input
```c
// ❌ WRONG: No validation
void set_size(size_t size) {
    config.size = size;
}

// ✅ CORRECT: Validate all inputs
int set_size(size_t size) {
    if (size == 0 || size > MAX_ALLOWED_SIZE) {
        return -EINVAL;
    }
    if (!is_valid_alignment(size)) {
        return -EINVAL;
    }
    config.size = size;
    return 0;
}
```

#### Sanitize File Paths
```c
// ❌ WRONG: Path traversal vulnerability
void open_file(const char *path) {
    int fd = open(path, O_RDONLY);
}

// ✅ CORRECT: Validate and sanitize path
int open_file(const char *path) {
    if (!path || strlen(path) >= PATH_MAX) {
        return -EINVAL;
    }
    
    // Check for path traversal attempts
    if (strstr(path, "..") || path[0] != '/') {
        return -EINVAL;
    }
    
    // Canonicalize and validate
    char realpath_buf[PATH_MAX];
    if (!realpath(path, realpath_buf)) {
        return -ENOENT;
    }
    
    return open(realpath_buf, O_RDONLY);
}
```

### 3. Resource Management

#### Memory Lifecycle
```c
// ❌ WRONG: Unclear ownership
void *alloc_and_init(void) {
    void *ptr = malloc(SIZE);
    init_data(ptr);
    return ptr;  // Who frees this?
}

// ✅ CORRECT: Clear ownership and lifecycle
/**
 * Allocate and initialize data structure.
 * 
 * @return Pointer to allocated structure. Caller must free with free_data().
 *         Returns NULL on allocation failure.
 */
void *alloc_and_init(void) {
    void *ptr = malloc(SIZE);
    if (!ptr) {
        return NULL;
    }
    
    if (init_data(ptr) != 0) {
        free(ptr);
        return NULL;
    }
    
    return ptr;
}

void free_data(void *ptr) {
    if (ptr) {
        cleanup_data(ptr);
        free(ptr);
    }
}
```

#### File Descriptor Management
```c
// ❌ WRONG: Descriptor leak on error path
int process_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    
    if (validate_file(fd) != 0) {
        return -1;  // Leaked fd!
    }
    
    return fd;
}

// ✅ CORRECT: Clean up on all paths
int process_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    
    int result = validate_file(fd);
    if (result != 0) {
        close(fd);
        return -1;
    }
    
    return fd;
}
```

### 4. Cryptography

#### Use Approved Algorithms
- **Hashing**: BLAKE3, SHA-256, SHA-3
- **Encryption**: AES-256-GCM, ChaCha20-Poly1305
- **Key Exchange**: X25519, ECDH (P-256, P-384)
- **Signatures**: Ed25519, ECDSA (P-256, P-384)

#### Never Roll Your Own Crypto
```c
// ❌ WRONG: Custom crypto implementation
uint32_t my_hash(const char *data) {
    uint32_t hash = 0;
    for (int i = 0; data[i]; i++) {
        hash = hash * 31 + data[i];
    }
    return hash;
}

// ✅ CORRECT: Use approved library
int compute_hash(const uint8_t *data, size_t len, uint8_t *hash) {
    return blake3_hash(data, len, hash, BLAKE3_OUT_LEN);
}
```

### 5. Concurrency Safety

#### Use Proper Locking
```c
// ❌ WRONG: Race condition
void increment_counter(void) {
    global_counter++;  // Not atomic!
}

// ✅ CORRECT: Proper synchronization
void increment_counter(void) {
    spin_lock(&counter_lock);
    global_counter++;
    spin_unlock(&counter_lock);
}

// ✅ BETTER: Atomic operations where applicable
void increment_counter(void) {
    atomic_inc(&global_counter);
}
```

#### Avoid TOCTOU Vulnerabilities
```c
// ❌ WRONG: Time-of-check to time-of-use race
if (access(path, R_OK) == 0) {
    int fd = open(path, O_RDONLY);  // File could change!
}

// ✅ CORRECT: Atomic operation
int fd = open(path, O_RDONLY);
if (fd >= 0) {
    // Use fd
}
```

## Code Quality Standards

### Error Handling

#### Always Check Return Values
```c
// ❌ WRONG: Unchecked return value
malloc(size);
strcpy(dest, src);

// ✅ CORRECT: Check and handle errors
void *ptr = malloc(size);
if (!ptr) {
    return -ENOMEM;
}

if (strlcpy(dest, src, sizeof(dest)) >= sizeof(dest)) {
    free(ptr);
    return -EINVAL;  // Truncation
}
```

#### Use Consistent Error Codes
- Return negative errno values (-EINVAL, -ENOMEM, etc.)
- Use 0 for success
- Document error conditions in function comments

### Documentation

#### Function Documentation Required
```c
/**
 * Allocate and initialize a security context.
 *
 * @param label Security label for the context
 * @param flags Context creation flags (MAC_FLAG_*)
 * @return Pointer to initialized context, or NULL on failure
 *
 * This function allocates a new security context with the specified
 * label and initializes it according to the current MAC policy.
 * The caller must free the context with security_context_free().
 *
 * Errors:
 *   - Returns NULL if allocation fails
 *   - Returns NULL if label is invalid
 *   - Returns NULL if MAC policy denies context creation
 */
security_context_t *security_context_create(const char *label, uint32_t flags);
```

### Code Organization

#### One Function, One Purpose
- Functions should be < 50 lines ideally
- Complex functions should be split into helpers
- Use meaningful function and variable names

#### Minimize Global State
- Use static for file-local symbols
- Pass state explicitly rather than using globals
- Document shared state carefully

## Prohibited Patterns

### Absolutely Forbidden
1. `strcpy()`, `strcat()`, `sprintf()` - Use bounded versions
2. `gets()` - Never use, use `fgets()`
3. `alloca()` - Use explicit heap allocation or bounded stack arrays
4. Variable Length Arrays (VLAs)
5. `goto` into scope (jumping over initialization)
6. Unbounded recursion
7. Assumptions about pointer sizes or alignment
8. Type punning via union (use memcpy)

### Highly Discouraged
1. `strncpy()` without null termination
2. Casting function pointers to void* and back
3. Ignoring const correctness
4. Arithmetic on void pointers
5. Using magic numbers instead of named constants

## Static Analysis

### Required Checks
All code must pass:
- GCC with `-Wall -Wextra -Werror`
- Clang static analyzer
- sparse (kernel sparse checking)

### Recommended Tools
- Coverity Scan
- Clang sanitizers (ASan, UBSan, MSan)
- Valgrind

## Security Review Requirements

### Review Triggers
Security review required for:
- All changes to security-critical code
- New system call interfaces
- Changes to MAC or capability systems
- Cryptographic code
- Network protocol implementations
- Parser code (especially for file formats)

### Review Criteria
- Memory safety verified
- Input validation complete
- Error handling correct
- Resource cleanup on all paths
- Concurrency safety reviewed
- Documentation complete

## Testing Requirements

### Unit Tests
- All security-critical functions must have unit tests
- Test normal cases and error cases
- Test boundary conditions

### Fuzzing
- Parsers must be fuzzed (minimum 24 hours)
- System call interfaces must be fuzzed
- File format handlers must be fuzzed

### Security Tests
- Privilege escalation attempts
- Sandbox escape attempts
- MAC policy bypass attempts

## References

- CERT C Coding Standard
- MISRA C:2012
- CWE Top 25
- OWASP Secure Coding Practices

## Enforcement

Violations of these standards will result in:
1. Code review rejection
2. Required fixes before merge
3. Post-merge security audit if violations slip through

Critical violations (memory safety, crypto misuse) require immediate remediation.

---

**Questions?** Contact: security@limitlessos.org  
**Updates**: This document is reviewed quarterly
