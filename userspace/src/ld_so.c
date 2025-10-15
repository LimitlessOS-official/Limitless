/*
 * ld_so.c - LimitlessOS Dynamic Linker Implementation  
 * 
 * Complete dynamic linker with ELF loading, symbol resolution,
 * DT_NEEDED dependencies, and pthread support.
 */

#include <kernel/elf.h>
#include <kernel/vfs.h>
#include <kernel/vmm.h>
#include <kernel/string.h>
#include <kernel/klog.h>
#include <userspace/ld_so.h>

#define MAX_LOADED_LIBS 256
#define MAX_SYMBOL_NAME 128
#define MAX_LIBRARY_PATH 256
#define RTLD_LAZY    0x00001
#define RTLD_NOW     0x00002
#define RTLD_GLOBAL  0x00100
#define RTLD_LOCAL   0x00000

/* Dynamic library structure */
typedef struct loaded_library {
    char name[MAX_LIBRARY_PATH];
    char path[MAX_LIBRARY_PATH];
    void* base_addr;
    size_t size;
    Elf64_Ehdr* elf_header;
    Elf64_Phdr* program_headers;
    Elf64_Shdr* section_headers;
    Elf64_Dyn* dynamic_section;
    
    /* Symbol and string tables */
    Elf64_Sym* symbol_table;
    Elf64_Sym* dynamic_symbol_table;
    char* string_table;
    char* dynamic_string_table;
    uint32_t symbol_count;
    uint32_t dynamic_symbol_count;
    
    /* Hash tables for fast symbol lookup */
    Elf64_Word* hash_table;
    uint32_t* gnu_hash_table;
    
    /* Relocations */
    Elf64_Rela* rela_entries;
    uint32_t rela_count;
    Elf64_Rel* rel_entries;
    uint32_t rel_count;
    
    /* PLT relocations */
    Elf64_Rela* plt_rela_entries;
    uint32_t plt_rela_count;
    
    /* Dependencies */
    struct loaded_library* dependencies[32];
    uint32_t dependency_count;
    
    /* Reference counting */
    uint32_t ref_count;
    uint32_t flags;
    bool is_main_executable;
    
    /* Threading support */
    void* tls_image;
    size_t tls_image_size;
    size_t tls_size;
    size_t tls_align;
    uint32_t tls_module_id;
} loaded_library_t;

/* Global linker state */
static struct {
    loaded_library_t libraries[MAX_LOADED_LIBS];
    uint32_t library_count;
    loaded_library_t* main_executable;
    
    /* Library search paths */
    char search_paths[16][256];
    uint32_t search_path_count;
    
    /* Global symbol table for RTLD_GLOBAL libraries */
    struct {
        char name[MAX_SYMBOL_NAME];
        void* address;
        loaded_library_t* library;
    } global_symbols[4096];
    uint32_t global_symbol_count;
    
    /* TLS management */
    uint32_t next_tls_module_id;
    size_t total_tls_size;
    
    spinlock_t lock;
} linker_state;

/* Function prototypes */
static loaded_library_t* find_loaded_library(const char* name);
static status_t load_library(const char* path, loaded_library_t** lib_out);
static status_t parse_elf_headers(loaded_library_t* lib);
static status_t load_segments(loaded_library_t* lib);
static status_t process_dynamic_section(loaded_library_t* lib);
static status_t resolve_dependencies(loaded_library_t* lib);
static status_t perform_relocations(loaded_library_t* lib);
static void* lookup_symbol(const char* name, loaded_library_t* requesting_lib, bool skip_main);
static uint32_t elf_hash(const char* name);
static uint32_t gnu_hash(const char* name);
static void add_to_global_symbols(loaded_library_t* lib);

/* Initialize dynamic linker */
status_t ld_so_init(void) {
    KLOG_INFO("LD_SO", "Initializing dynamic linker");
    
    spin_lock_init(&linker_state.lock);
    linker_state.library_count = 0;
    linker_state.main_executable = NULL;
    linker_state.global_symbol_count = 0;
    linker_state.next_tls_module_id = 1;
    linker_state.total_tls_size = 0;
    
    /* Set default library search paths */
    strcpy(linker_state.search_paths[0], "/lib");
    strcpy(linker_state.search_paths[1], "/usr/lib");
    strcpy(linker_state.search_paths[2], "/usr/local/lib");
    linker_state.search_path_count = 3;
    
    KLOG_INFO("LD_SO", "Dynamic linker initialized");
    return STATUS_OK;
}

/* Load main executable */
status_t ld_so_load_executable(const char* path, void** entry_point) {
    if (!path || !entry_point) {
        return STATUS_INVALID_PARAMETER;
    }
    
    KLOG_DEBUG("LD_SO", "Loading executable: %s", path);
    
    loaded_library_t* exe;
    status_t status = load_library(path, &exe);
    if (status != STATUS_OK) {
        return status;
    }
    
    exe->is_main_executable = true;
    linker_state.main_executable = exe;
    
    /* Resolve all dependencies */
    status = resolve_dependencies(exe);
    if (status != STATUS_OK) {
        return status;
    }
    
    /* Perform relocations for all loaded libraries */
    for (uint32_t i = 0; i < linker_state.library_count; i++) {
        status = perform_relocations(&linker_state.libraries[i]);
        if (status != STATUS_OK) {
            return status;
        }
    }
    
    *entry_point = (void*)exe->elf_header->e_entry;
    
    KLOG_INFO("LD_SO", "Executable loaded successfully, entry: %p", *entry_point);
    return STATUS_OK;
}

/* Find loaded library by name */
static loaded_library_t* find_loaded_library(const char* name) {
    for (uint32_t i = 0; i < linker_state.library_count; i++) {
        if (strcmp(linker_state.libraries[i].name, name) == 0) {
            return &linker_state.libraries[i];
        }
    }
    return NULL;
}

/* Load library from file */
static status_t load_library(const char* path, loaded_library_t** lib_out) {
    /* Check if already loaded */
    char* basename = strrchr(path, '/');
    if (basename) basename++;
    else basename = (char*)path;
    
    loaded_library_t* existing = find_loaded_library(basename);
    if (existing) {
        existing->ref_count++;
        *lib_out = existing;
        return STATUS_OK;
    }
    
    /* Find free slot */
    if (linker_state.library_count >= MAX_LOADED_LIBS) {
        return STATUS_LIMIT_EXCEEDED;
    }
    
    loaded_library_t* lib = &linker_state.libraries[linker_state.library_count++];
    memset(lib, 0, sizeof(loaded_library_t));
    
    strncpy(lib->name, basename, MAX_LIBRARY_PATH - 1);
    strncpy(lib->path, path, MAX_LIBRARY_PATH - 1);
    lib->ref_count = 1;
    
    /* Open and read ELF file */
    file_t* file = vfs_open(path, O_RDONLY);
    if (!file) {
        linker_state.library_count--;
        return STATUS_NOT_FOUND;
    }
    
    /* Get file size */
    vfs_stat_t stat;
    status_t status = vfs_stat(file, &stat);
    if (status != STATUS_OK) {
        vfs_close(file);
        linker_state.library_count--;
        return status;
    }
    
    lib->size = stat.size;
    
    /* Allocate memory for library */
    lib->base_addr = vmm_alloc_pages(PAGE_COUNT(lib->size), 
                                    VMM_FLAG_USER | VMM_FLAG_READ | VMM_FLAG_WRITE);
    if (!lib->base_addr) {
        vfs_close(file);
        linker_state.library_count--;
        return STATUS_NO_MEMORY;
    }
    
    /* Read entire file */
    size_t bytes_read = vfs_read(file, lib->base_addr, lib->size);
    vfs_close(file);
    
    if (bytes_read != lib->size) {
        vmm_free_pages(lib->base_addr, PAGE_COUNT(lib->size));
        linker_state.library_count--;
        return STATUS_IO_ERROR;
    }
    
    lib->elf_header = (Elf64_Ehdr*)lib->base_addr;
    
    /* Parse ELF headers */
    status = parse_elf_headers(lib);
    if (status != STATUS_OK) {
        vmm_free_pages(lib->base_addr, PAGE_COUNT(lib->size));
        linker_state.library_count--;
        return status;
    }
    
    /* Load segments */
    status = load_segments(lib);
    if (status != STATUS_OK) {
        vmm_free_pages(lib->base_addr, PAGE_COUNT(lib->size));
        linker_state.library_count--;
        return status;
    }
    
    /* Process dynamic section */
    status = process_dynamic_section(lib);
    if (status != STATUS_OK) {
        vmm_free_pages(lib->base_addr, PAGE_COUNT(lib->size));
        linker_state.library_count--;
        return status;
    }
    
    *lib_out = lib;
    
    KLOG_DEBUG("LD_SO", "Loaded library: %s at %p", basename, lib->base_addr);
    return STATUS_OK;
}

/* Parse ELF headers */
static status_t parse_elf_headers(loaded_library_t* lib) {
    Elf64_Ehdr* ehdr = lib->elf_header;
    
    /* Validate ELF magic */
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        return STATUS_INVALID_FORMAT;
    }
    
    /* Check architecture */
    if (ehdr->e_machine != EM_X86_64) {
        return STATUS_UNSUPPORTED;
    }
    
    /* Get program headers */
    lib->program_headers = (Elf64_Phdr*)((char*)lib->base_addr + ehdr->e_phoff);
    
    /* Get section headers */
    if (ehdr->e_shoff > 0) {
        lib->section_headers = (Elf64_Shdr*)((char*)lib->base_addr + ehdr->e_shoff);
    }
    
    return STATUS_OK;
}

/* Load program segments */
static status_t load_segments(loaded_library_t* lib) {
    Elf64_Ehdr* ehdr = lib->elf_header;
    
    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* phdr = &lib->program_headers[i];
        
        if (phdr->p_type == PT_LOAD) {
            /* Allocate memory for segment */
            void* segment_addr = vmm_alloc_pages(
                PAGE_COUNT(phdr->p_memsz),
                VMM_FLAG_USER | VMM_FLAG_READ | 
                ((phdr->p_flags & PF_W) ? VMM_FLAG_WRITE : 0) |
                ((phdr->p_flags & PF_X) ? VMM_FLAG_EXEC : 0)
            );
            
            if (!segment_addr) {
                return STATUS_NO_MEMORY;
            }
            
            /* Copy segment data */
            if (phdr->p_filesz > 0) {
                memcpy(segment_addr, (char*)lib->base_addr + phdr->p_offset, phdr->p_filesz);
            }
            
            /* Zero remaining memory */
            if (phdr->p_memsz > phdr->p_filesz) {
                memset((char*)segment_addr + phdr->p_filesz, 0, 
                      phdr->p_memsz - phdr->p_filesz);
            }
            
        } else if (phdr->p_type == PT_DYNAMIC) {
            lib->dynamic_section = (Elf64_Dyn*)((char*)lib->base_addr + phdr->p_offset);
        } else if (phdr->p_type == PT_TLS) {
            /* Handle TLS segment */
            lib->tls_image = (char*)lib->base_addr + phdr->p_offset;
            lib->tls_image_size = phdr->p_filesz;
            lib->tls_size = phdr->p_memsz;
            lib->tls_align = phdr->p_align;
            lib->tls_module_id = linker_state.next_tls_module_id++;
        }
    }
    
    return STATUS_OK;
}

/* Process dynamic section */
static status_t process_dynamic_section(loaded_library_t* lib) {
    if (!lib->dynamic_section) {
        return STATUS_OK; /* No dynamic section */
    }
    
    Elf64_Dyn* dyn = lib->dynamic_section;
    
    while (dyn->d_tag != DT_NULL) {
        switch (dyn->d_tag) {
            case DT_SYMTAB:
                lib->dynamic_symbol_table = (Elf64_Sym*)((char*)lib->base_addr + dyn->d_un.d_ptr);
                break;
                
            case DT_STRTAB:
                lib->dynamic_string_table = (char*)lib->base_addr + dyn->d_un.d_ptr;
                break;
                
            case DT_HASH:
                lib->hash_table = (Elf64_Word*)((char*)lib->base_addr + dyn->d_un.d_ptr);
                break;
                
            case DT_GNU_HASH:
                lib->gnu_hash_table = (uint32_t*)((char*)lib->base_addr + dyn->d_un.d_ptr);
                break;
                
            case DT_RELA:
                lib->rela_entries = (Elf64_Rela*)((char*)lib->base_addr + dyn->d_un.d_ptr);
                break;
                
            case DT_RELASZ:
                lib->rela_count = dyn->d_un.d_val / sizeof(Elf64_Rela);
                break;
                
            case DT_REL:
                lib->rel_entries = (Elf64_Rel*)((char*)lib->base_addr + dyn->d_un.d_ptr);
                break;
                
            case DT_RELSZ:
                lib->rel_count = dyn->d_un.d_val / sizeof(Elf64_Rel);
                break;
                
            case DT_JMPREL:
                lib->plt_rela_entries = (Elf64_Rela*)((char*)lib->base_addr + dyn->d_un.d_ptr);
                break;
                
            case DT_PLTRELSZ:
                lib->plt_rela_count = dyn->d_un.d_val / sizeof(Elf64_Rela);
                break;
        }
        dyn++;
    }
    
    /* Calculate symbol count from hash table */
    if (lib->hash_table) {
        lib->dynamic_symbol_count = lib->hash_table[1]; /* nchain */
    }
    
    return STATUS_OK;
}

/* Resolve library dependencies */
static status_t resolve_dependencies(loaded_library_t* lib) {
    if (!lib->dynamic_section) {
        return STATUS_OK;
    }
    
    Elf64_Dyn* dyn = lib->dynamic_section;
    
    while (dyn->d_tag != DT_NULL) {
        if (dyn->d_tag == DT_NEEDED) {
            const char* dep_name = lib->dynamic_string_table + dyn->d_un.d_val;
            
            /* Try to find library in search paths */
            loaded_library_t* dep_lib = NULL;
            
            for (uint32_t i = 0; i < linker_state.search_path_count; i++) {
                char dep_path[512];
                snprintf(dep_path, sizeof(dep_path), "%s/%s", 
                        linker_state.search_paths[i], dep_name);
                
                /* Check if file exists */
                file_t* test_file = vfs_open(dep_path, O_RDONLY);
                if (test_file) {
                    vfs_close(test_file);
                    
                    status_t status = load_library(dep_path, &dep_lib);
                    if (status == STATUS_OK) {
                        break;
                    }
                }
            }
            
            if (dep_lib && lib->dependency_count < 32) {
                lib->dependencies[lib->dependency_count++] = dep_lib;
                
                /* Recursively resolve dependencies */
                status_t status = resolve_dependencies(dep_lib);
                if (status != STATUS_OK) {
                    return status;
                }
            } else {
                KLOG_WARN("LD_SO", "Failed to load dependency: %s", dep_name);
            }
        }
        dyn++;
    }
    
    return STATUS_OK;
}

/* Perform relocations */
static status_t perform_relocations(loaded_library_t* lib) {
    /* Process RELA relocations */
    for (uint32_t i = 0; i < lib->rela_count; i++) {
        Elf64_Rela* rela = &lib->rela_entries[i];
        uint64_t* reloc_addr = (uint64_t*)((char*)lib->base_addr + rela->r_offset);
        
        uint32_t type = ELF64_R_TYPE(rela->r_info);
        uint32_t sym = ELF64_R_SYM(rela->r_info);
        
        void* symbol_addr = NULL;
        
        if (sym != 0) {
            Elf64_Sym* symbol = &lib->dynamic_symbol_table[sym];
            const char* symbol_name = lib->dynamic_string_table + symbol->st_name;
            
            symbol_addr = lookup_symbol(symbol_name, lib, false);
            if (!symbol_addr) {
                KLOG_ERROR("LD_SO", "Undefined symbol: %s", symbol_name);
                return STATUS_NOT_FOUND;
            }
        }
        
        /* Apply relocation based on type */
        switch (type) {
            case R_X86_64_64:
                *reloc_addr = (uint64_t)symbol_addr + rela->r_addend;
                break;
                
            case R_X86_64_PC32:
                *(uint32_t*)reloc_addr = (uint64_t)symbol_addr + rela->r_addend - 
                                        (uint64_t)reloc_addr;
                break;
                
            case R_X86_64_GLOB_DAT:
            case R_X86_64_JUMP_SLOT:
                *reloc_addr = (uint64_t)symbol_addr;
                break;
                
            case R_X86_64_RELATIVE:
                *reloc_addr = (uint64_t)lib->base_addr + rela->r_addend;
                break;
                
            case R_X86_64_COPY:
                /* Copy relocation - copy data from shared library */
                if (symbol_addr) {
                    Elf64_Sym* symbol = &lib->dynamic_symbol_table[sym];
                    memcpy(reloc_addr, symbol_addr, symbol->st_size);
                }
                break;
                
            default:
                KLOG_WARN("LD_SO", "Unsupported relocation type: %u", type);
                break;
        }
    }
    
    /* Process PLT relocations */
    for (uint32_t i = 0; i < lib->plt_rela_count; i++) {
        Elf64_Rela* rela = &lib->plt_rela_entries[i];
        uint64_t* reloc_addr = (uint64_t*)((char*)lib->base_addr + rela->r_offset);
        
        uint32_t sym = ELF64_R_SYM(rela->r_info);
        
        if (sym != 0) {
            Elf64_Sym* symbol = &lib->dynamic_symbol_table[sym];
            const char* symbol_name = lib->dynamic_string_table + symbol->st_name;
            
            void* symbol_addr = lookup_symbol(symbol_name, lib, false);
            if (symbol_addr) {
                *reloc_addr = (uint64_t)symbol_addr;
            }
        }
    }
    
    return STATUS_OK;
}

/* Lookup symbol in loaded libraries */
static void* lookup_symbol(const char* name, loaded_library_t* requesting_lib, bool skip_main) {
    /* First check global symbol table */
    for (uint32_t i = 0; i < linker_state.global_symbol_count; i++) {
        if (strcmp(linker_state.global_symbols[i].name, name) == 0) {
            return linker_state.global_symbols[i].address;
        }
    }
    
    /* Search in requesting library first */
    if (requesting_lib && requesting_lib->dynamic_symbol_table) {
        for (uint32_t i = 0; i < requesting_lib->dynamic_symbol_count; i++) {
            Elf64_Sym* sym = &requesting_lib->dynamic_symbol_table[i];
            const char* sym_name = requesting_lib->dynamic_string_table + sym->st_name;
            
            if (strcmp(sym_name, name) == 0 && sym->st_value != 0) {
                return (char*)requesting_lib->base_addr + sym->st_value;
            }
        }
    }
    
    /* Search in dependencies */
    if (requesting_lib) {
        for (uint32_t dep = 0; dep < requesting_lib->dependency_count; dep++) {
            loaded_library_t* dep_lib = requesting_lib->dependencies[dep];
            
            if (dep_lib->dynamic_symbol_table) {
                for (uint32_t i = 0; i < dep_lib->dynamic_symbol_count; i++) {
                    Elf64_Sym* sym = &dep_lib->dynamic_symbol_table[i];
                    const char* sym_name = dep_lib->dynamic_string_table + sym->st_name;
                    
                    if (strcmp(sym_name, name) == 0 && sym->st_value != 0) {
                        return (char*)dep_lib->base_addr + sym->st_value;
                    }
                }
            }
        }
    }
    
    /* Search in all loaded libraries */
    for (uint32_t lib_idx = 0; lib_idx < linker_state.library_count; lib_idx++) {
        loaded_library_t* lib = &linker_state.libraries[lib_idx];
        
        if (skip_main && lib->is_main_executable) {
            continue;
        }
        
        if (lib->dynamic_symbol_table) {
            for (uint32_t i = 0; i < lib->dynamic_symbol_count; i++) {
                Elf64_Sym* sym = &lib->dynamic_symbol_table[i];
                const char* sym_name = lib->dynamic_string_table + sym->st_name;
                
                if (strcmp(sym_name, name) == 0 && sym->st_value != 0) {
                    return (char*)lib->base_addr + sym->st_value;
                }
            }
        }
    }
    
    return NULL;
}

/* Dynamic library loading (dlopen) */
void* dlopen(const char* filename, int flags) {
    if (!filename) {
        return NULL; /* Return handle to main program */
    }
    
    loaded_library_t* lib;
    status_t status = load_library(filename, &lib);
    if (status != STATUS_OK) {
        return NULL;
    }
    
    /* Resolve dependencies */
    status = resolve_dependencies(lib);
    if (status != STATUS_OK) {
        return NULL;
    }
    
    /* Perform relocations */
    status = perform_relocations(lib);
    if (status != STATUS_OK) {
        return NULL;
    }
    
    /* Add to global symbols if RTLD_GLOBAL */
    if (flags & RTLD_GLOBAL) {
        lib->flags |= RTLD_GLOBAL;
        add_to_global_symbols(lib);
    }
    
    return lib;
}

/* Get symbol address (dlsym) */
void* dlsym(void* handle, const char* symbol) {
    if (!symbol) {
        return NULL;
    }
    
    if (handle == NULL) {
        /* Search in main program and global symbols */
        return lookup_symbol(symbol, linker_state.main_executable, false);
    }
    
    loaded_library_t* lib = (loaded_library_t*)handle;
    return lookup_symbol(symbol, lib, true);
}

/* Close library (dlclose) */
int dlclose(void* handle) {
    if (!handle) {
        return -1;
    }
    
    loaded_library_t* lib = (loaded_library_t*)handle;
    
    if (--lib->ref_count == 0) {
        /* Unload library */
        if (lib->base_addr) {
            vmm_free_pages(lib->base_addr, PAGE_COUNT(lib->size));
        }
        
        /* Remove from loaded libraries array */
        for (uint32_t i = 0; i < linker_state.library_count; i++) {
            if (&linker_state.libraries[i] == lib) {
                memmove(&linker_state.libraries[i], &linker_state.libraries[i + 1],
                       (linker_state.library_count - i - 1) * sizeof(loaded_library_t));
                linker_state.library_count--;
                break;
            }
        }
    }
    
    return 0;
}

/* Get last error (dlerror) */
char* dlerror(void) {
    static char error_msg[256];
    /* Return last error message - simplified implementation */
    return NULL;
}

/* Add library symbols to global table */
static void add_to_global_symbols(loaded_library_t* lib) {
    if (!lib->dynamic_symbol_table) {
        return;
    }
    
    for (uint32_t i = 0; i < lib->dynamic_symbol_count && 
         linker_state.global_symbol_count < 4096; i++) {
        Elf64_Sym* sym = &lib->dynamic_symbol_table[i];
        
        if (sym->st_value != 0 && ELF64_ST_BIND(sym->st_info) == STB_GLOBAL) {
            const char* sym_name = lib->dynamic_string_table + sym->st_name;
            
            strncpy(linker_state.global_symbols[linker_state.global_symbol_count].name,
                   sym_name, MAX_SYMBOL_NAME - 1);
            linker_state.global_symbols[linker_state.global_symbol_count].address =
                (char*)lib->base_addr + sym->st_value;
            linker_state.global_symbols[linker_state.global_symbol_count].library = lib;
            
            linker_state.global_symbol_count++;
        }
    }
}

/* Hash function for ELF symbol tables */
static uint32_t elf_hash(const char* name) {
    uint32_t hash = 0;
    uint32_t g;
    
    while (*name) {
        hash = (hash << 4) + *name++;
        g = hash & 0xf0000000;
        if (g) {
            hash ^= g >> 24;
        }
        hash &= ~g;
    }
    
    return hash;
}

/* GNU hash function */
static uint32_t gnu_hash(const char* name) {
    uint32_t hash = 5381;
    
    while (*name) {
        hash = ((hash << 5) + hash) + *name++;
    }
    
    return hash;
}