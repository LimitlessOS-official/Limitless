/*
 * doc_generator.c - LimitlessOS Documentation Generator
 * 
 * Automated documentation generation system with API documentation,
 * code analysis, and multi-format output support.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

#define MAX_FUNCTIONS           5000
#define MAX_STRUCTURES          1000
#define MAX_CONSTANTS           2000
#define MAX_FILES               1000
#define MAX_LINE_LENGTH         1024
#define MAX_COMMENT_LENGTH      4096
#define MAX_NAME_LENGTH         256

/* Documentation output formats */
typedef enum {
    DOC_FORMAT_HTML,        /* HTML documentation */
    DOC_FORMAT_MARKDOWN,    /* Markdown documentation */
    DOC_FORMAT_LATEX,       /* LaTeX documentation */
    DOC_FORMAT_PDF,         /* PDF documentation */
    DOC_FORMAT_MAN,         /* Man pages */
    DOC_FORMAT_XML          /* XML documentation */
} doc_format_t;

/* Code element types */
typedef enum {
    ELEMENT_TYPE_FUNCTION,      /* Function definition */
    ELEMENT_TYPE_STRUCTURE,     /* Structure definition */
    ELEMENT_TYPE_ENUM,          /* Enumeration definition */
    ELEMENT_TYPE_CONSTANT,      /* Constant/macro definition */
    ELEMENT_TYPE_VARIABLE,      /* Global variable */
    ELEMENT_TYPE_TYPEDEF        /* Type definition */
} element_type_t;

/* Function parameter */
typedef struct function_param {
    char type[MAX_NAME_LENGTH];     /* Parameter type */
    char name[MAX_NAME_LENGTH];     /* Parameter name */
    char description[512];          /* Parameter description */
} function_param_t;

/* Function documentation */
typedef struct function_doc {
    char name[MAX_NAME_LENGTH];     /* Function name */
    char return_type[MAX_NAME_LENGTH]; /* Return type */
    char description[MAX_COMMENT_LENGTH]; /* Function description */
    char detailed_desc[MAX_COMMENT_LENGTH]; /* Detailed description */
    
    /* Parameters */
    function_param_t params[16];
    uint32_t param_count;
    
    /* Return value */
    char return_desc[512];          /* Return value description */
    
    /* Additional information */
    char filename[MAX_NAME_LENGTH]; /* Source file */
    uint32_t line_number;           /* Line number */
    char see_also[512];             /* See also references */
    char examples[MAX_COMMENT_LENGTH]; /* Usage examples */
    char notes[MAX_COMMENT_LENGTH]; /* Additional notes */
    
    /* Classification */
    char category[128];             /* Function category */
    char subsystem[128];            /* Subsystem */
    bool is_public;                 /* Public API */
    bool is_deprecated;             /* Deprecated function */
    
} function_doc_t;

/* Structure field */
typedef struct struct_field {
    char type[MAX_NAME_LENGTH];     /* Field type */
    char name[MAX_NAME_LENGTH];     /* Field name */
    char description[512];          /* Field description */
    size_t offset;                  /* Field offset */
    size_t size;                    /* Field size */
} struct_field_t;

/* Structure documentation */
typedef struct structure_doc {
    char name[MAX_NAME_LENGTH];     /* Structure name */
    char description[MAX_COMMENT_LENGTH]; /* Structure description */
    char detailed_desc[MAX_COMMENT_LENGTH]; /* Detailed description */
    
    /* Fields */
    struct_field_t fields[64];
    uint32_t field_count;
    
    /* Additional information */
    char filename[MAX_NAME_LENGTH]; /* Source file */
    uint32_t line_number;           /* Line number */
    size_t total_size;              /* Total structure size */
    char usage_notes[MAX_COMMENT_LENGTH]; /* Usage notes */
    
    /* Classification */
    char category[128];             /* Structure category */
    bool is_public;                 /* Public structure */
    
} structure_doc_t;

/* Constant documentation */
typedef struct constant_doc {
    char name[MAX_NAME_LENGTH];     /* Constant name */
    char value[MAX_NAME_LENGTH];    /* Constant value */
    char description[MAX_COMMENT_LENGTH]; /* Description */
    
    char filename[MAX_NAME_LENGTH]; /* Source file */
    uint32_t line_number;           /* Line number */
    char category[128];             /* Category */
    
} constant_doc_t;

/* File documentation */
typedef struct file_doc {
    char filename[MAX_NAME_LENGTH]; /* File name */
    char full_path[512];            /* Full file path */
    char description[MAX_COMMENT_LENGTH]; /* File description */
    char author[256];               /* File author */
    char copyright[256];            /* Copyright information */
    char version[64];               /* File version */
    
    /* Statistics */
    uint32_t line_count;            /* Total lines */
    uint32_t code_lines;            /* Code lines */
    uint32_t comment_lines;         /* Comment lines */
    uint32_t function_count;        /* Functions in file */
    uint32_t struct_count;          /* Structures in file */
    
} file_doc_t;

/* Documentation generator state */
typedef struct doc_generator {
    bool initialized;
    
    /* Configuration */
    char project_name[256];         /* Project name */
    char project_version[64];       /* Project version */
    char output_directory[512];     /* Output directory */
    doc_format_t output_format;     /* Output format */
    
    /* Documentation database */
    function_doc_t functions[MAX_FUNCTIONS];
    uint32_t function_count;
    
    structure_doc_t structures[MAX_STRUCTURES];
    uint32_t structure_count;
    
    constant_doc_t constants[MAX_CONSTANTS];
    uint32_t constant_count;
    
    file_doc_t files[MAX_FILES];
    uint32_t file_count;
    
    /* Generation options */
    bool include_private;           /* Include private functions */
    bool include_source;            /* Include source code */
    bool generate_index;            /* Generate index */
    bool generate_cross_refs;       /* Generate cross-references */
    
    /* Statistics */
    struct {
        uint32_t files_processed;
        uint32_t comments_parsed;
        uint32_t elements_documented;
        time_t generation_time;
    } stats;
    
} doc_generator_t;

static doc_generator_t g_doc_gen = {0};

/* Function prototypes */
static int parse_source_file(const char* filename);
static int parse_header_file(const char* filename);
static int extract_comment_block(FILE* file, char* comment, size_t max_size);
static int parse_function_declaration(const char* line, function_doc_t* func_doc);
static int parse_structure_definition(FILE* file, const char* line, structure_doc_t* struct_doc);
static int parse_constant_definition(const char* line, constant_doc_t* const_doc);
static void parse_doxygen_comments(const char* comment, function_doc_t* func_doc);
static int generate_html_documentation(void);
static int generate_markdown_documentation(void);
static int generate_pdf_documentation(void);
static void write_html_header(FILE* file);
static void write_html_footer(FILE* file);
static void write_function_html(FILE* file, const function_doc_t* func_doc);
static void write_structure_html(FILE* file, const structure_doc_t* struct_doc);
static int scan_directory_recursive(const char* directory);

/* Initialize documentation generator */
int doc_generator_init(const char* project_name, const char* project_version) {
    if (g_doc_gen.initialized) {
        return 0;
    }
    
    memset(&g_doc_gen, 0, sizeof(doc_generator_t));
    
    strncpy(g_doc_gen.project_name, project_name ? project_name : "LimitlessOS",
            sizeof(g_doc_gen.project_name) - 1);
    strncpy(g_doc_gen.project_version, project_version ? project_version : "1.0.0",
            sizeof(g_doc_gen.project_version) - 1);
    
    strcpy(g_doc_gen.output_directory, "docs");
    g_doc_gen.output_format = DOC_FORMAT_HTML;
    
    /* Default options */
    g_doc_gen.include_private = false;
    g_doc_gen.include_source = true;
    g_doc_gen.generate_index = true;
    g_doc_gen.generate_cross_refs = true;
    
    g_doc_gen.initialized = true;
    
    printf("Documentation generator initialized for %s v%s\n",
           g_doc_gen.project_name, g_doc_gen.project_version);
    
    return 0;
}

/* Parse source directory */
int doc_generator_parse_directory(const char* directory) {
    if (!g_doc_gen.initialized) {
        return -1;
    }
    
    printf("Parsing source directory: %s\n", directory);
    
    g_doc_gen.stats.generation_time = time(NULL);
    
    int result = scan_directory_recursive(directory);
    
    printf("Parsed %u files, found %u functions, %u structures, %u constants\n",
           g_doc_gen.stats.files_processed, g_doc_gen.function_count,
           g_doc_gen.structure_count, g_doc_gen.constant_count);
    
    return result;
}

/* Scan directory recursively */
static int scan_directory_recursive(const char* directory) {
    DIR* dir = opendir(directory);
    if (!dir) {
        printf("Warning: Could not open directory %s\n", directory);
        return -1;
    }
    
    struct dirent* entry;
    int files_processed = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext) {
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
                
                if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || 
                    strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0) {
                    
                    printf("Parsing source file: %s\n", full_path);
                    if (parse_source_file(full_path) == 0) {
                        files_processed++;
                    }
                    
                } else if (strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0 ||
                          strcmp(ext, ".hxx") == 0) {
                    
                    printf("Parsing header file: %s\n", full_path);
                    if (parse_header_file(full_path) == 0) {
                        files_processed++;
                    }
                }
            }
        } else if (entry->d_type == DT_DIR && 
                   strcmp(entry->d_name, ".") != 0 && 
                   strcmp(entry->d_name, "..") != 0 &&
                   strcmp(entry->d_name, ".git") != 0) {
            
            char subdir_path[512];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", directory, entry->d_name);
            files_processed += scan_directory_recursive(subdir_path);
        }
    }
    
    closedir(dir);
    g_doc_gen.stats.files_processed += files_processed;
    
    return files_processed;
}

/* Parse source file */
static int parse_source_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    /* Add file to documentation */
    if (g_doc_gen.file_count < MAX_FILES) {
        file_doc_t* file_doc = &g_doc_gen.files[g_doc_gen.file_count++];
        
        char* base_name = strrchr(filename, '/');
        strncpy(file_doc->filename, base_name ? base_name + 1 : filename,
                sizeof(file_doc->filename) - 1);
        strncpy(file_doc->full_path, filename, sizeof(file_doc->full_path) - 1);
        
        strcpy(file_doc->description, "Source file");
        file_doc->line_count = 0;
        file_doc->code_lines = 0;
        file_doc->comment_lines = 0;
    }
    
    char line[MAX_LINE_LENGTH];
    char comment_block[MAX_COMMENT_LENGTH];
    uint32_t line_number = 0;
    bool in_comment = false;
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        /* Remove newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip empty lines */
        if (strlen(line) == 0) continue;
        
        /* Check for comment blocks */
        if (strstr(line, "/*") != NULL) {
            in_comment = true;
            
            /* Extract complete comment block */
            if (extract_comment_block(file, comment_block, sizeof(comment_block)) > 0) {
                g_doc_gen.stats.comments_parsed++;
                
                /* Look for function definition after comment */
                char next_line[MAX_LINE_LENGTH];
                if (fgets(next_line, sizeof(next_line), file)) {
                    line_number++;
                    next_line[strcspn(next_line, "\n")] = 0;
                    
                    /* Check if this is a function declaration/definition */
                    if (strchr(next_line, '(') && g_doc_gen.function_count < MAX_FUNCTIONS) {
                        function_doc_t* func_doc = &g_doc_gen.functions[g_doc_gen.function_count];
                        
                        if (parse_function_declaration(next_line, func_doc) == 0) {
                            strncpy(func_doc->filename, filename, sizeof(func_doc->filename) - 1);
                            func_doc->line_number = line_number;
                            parse_doxygen_comments(comment_block, func_doc);
                            
                            g_doc_gen.function_count++;
                            g_doc_gen.stats.elements_documented++;
                        }
                    }
                }
            }
            continue;
        }
        
        /* Parse structure definitions */
        if ((strstr(line, "struct") || strstr(line, "typedef struct")) && 
            g_doc_gen.structure_count < MAX_STRUCTURES) {
            
            structure_doc_t* struct_doc = &g_doc_gen.structures[g_doc_gen.structure_count];
            
            if (parse_structure_definition(file, line, struct_doc) == 0) {
                strncpy(struct_doc->filename, filename, sizeof(struct_doc->filename) - 1);
                struct_doc->line_number = line_number;
                
                g_doc_gen.structure_count++;
                g_doc_gen.stats.elements_documented++;
            }
        }
        
        /* Parse constant definitions */
        if ((strstr(line, "#define") || strstr(line, "const")) && 
            g_doc_gen.constant_count < MAX_CONSTANTS) {
            
            constant_doc_t* const_doc = &g_doc_gen.constants[g_doc_gen.constant_count];
            
            if (parse_constant_definition(line, const_doc) == 0) {
                strncpy(const_doc->filename, filename, sizeof(const_doc->filename) - 1);
                const_doc->line_number = line_number;
                
                g_doc_gen.constant_count++;
                g_doc_gen.stats.elements_documented++;
            }
        }
    }
    
    fclose(file);
    return 0;
}

/* Parse header file */
static int parse_header_file(const char* filename) {
    /* Header files use the same parsing logic as source files */
    return parse_source_file(filename);
}

/* Extract comment block */
static int extract_comment_block(FILE* file, char* comment, size_t max_size) {
    char line[MAX_LINE_LENGTH];
    size_t comment_len = 0;
    bool first_line = true;
    
    comment[0] = '\0';
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        /* Check for end of comment */
        if (strstr(line, "*/")) {
            /* Extract text before */ 
            char* end_pos = strstr(line, "*/");
            *end_pos = '\0';
            
            /* Add remaining text */
            if (strlen(line) > 0) {
                strncat(comment, line, max_size - comment_len - 1);
                strncat(comment, " ", max_size - comment_len - 1);
            }
            break;
        }
        
        /* Remove comment markers and leading whitespace */
        char* content = line;
        while (*content && (*content == ' ' || *content == '\t' || *content == '*')) {
            content++;
        }
        
        if (strlen(content) > 0) {
            if (!first_line) {
                strncat(comment, " ", max_size - comment_len - 1);
            }
            strncat(comment, content, max_size - comment_len - 1);
            comment_len = strlen(comment);
            first_line = false;
        }
    }
    
    return comment_len;
}

/* Parse function declaration */
static int parse_function_declaration(const char* line, function_doc_t* func_doc) {
    memset(func_doc, 0, sizeof(function_doc_t));
    
    /* Simple function parsing - extract return type and name */
    char* line_copy = strdup(line);
    char* func_start = strchr(line_copy, ' ');
    if (!func_start) {
        free(line_copy);
        return -1;
    }
    
    /* Extract return type */
    *func_start = '\0';
    strncpy(func_doc->return_type, line_copy, sizeof(func_doc->return_type) - 1);
    
    /* Extract function name */
    func_start++;
    char* paren_pos = strchr(func_start, '(');
    if (!paren_pos) {
        free(line_copy);
        return -1;
    }
    
    *paren_pos = '\0';
    strncpy(func_doc->name, func_start, sizeof(func_doc->name) - 1);
    
    /* Extract parameters (simplified) */
    paren_pos++;
    char* param_end = strchr(paren_pos, ')');
    if (param_end) {
        *param_end = '\0';
        
        /* Parse parameters */
        char* param = strtok(paren_pos, ",");
        while (param && func_doc->param_count < 16) {
            /* Trim whitespace */
            while (*param == ' ' || *param == '\t') param++;
            
            if (strlen(param) > 0 && strcmp(param, "void") != 0) {
                function_param_t* p = &func_doc->params[func_doc->param_count++];
                
                /* Split type and name (simplified) */
                char* last_space = strrchr(param, ' ');
                if (last_space) {
                    *last_space = '\0';
                    strncpy(p->type, param, sizeof(p->type) - 1);
                    strncpy(p->name, last_space + 1, sizeof(p->name) - 1);
                } else {
                    strncpy(p->type, param, sizeof(p->type) - 1);
                    strcpy(p->name, "param");
                }
            }
            
            param = strtok(NULL, ",");
        }
    }
    
    func_doc->is_public = true;  /* Default to public */
    
    free(line_copy);
    return 0;
}

/* Parse structure definition */
static int parse_structure_definition(FILE* file, const char* line, structure_doc_t* struct_doc) {
    memset(struct_doc, 0, sizeof(structure_doc_t));
    
    /* Extract structure name */
    char* struct_pos = strstr(line, "struct");
    if (struct_pos) {
        struct_pos += 6;  /* Skip "struct" */
        while (*struct_pos == ' ' || *struct_pos == '\t') struct_pos++;
        
        char* name_end = strchr(struct_pos, ' ');
        if (!name_end) name_end = strchr(struct_pos, '{');
        if (!name_end) name_end = struct_pos + strlen(struct_pos);
        
        size_t name_len = name_end - struct_pos;
        strncpy(struct_doc->name, struct_pos, name_len < sizeof(struct_doc->name) ? name_len : sizeof(struct_doc->name) - 1);
    }
    
    /* Parse structure fields */
    char field_line[MAX_LINE_LENGTH];
    while (fgets(field_line, sizeof(field_line), file)) {
        field_line[strcspn(field_line, "\n")] = 0;
        
        /* Check for end of structure */
        if (strchr(field_line, '}')) {
            break;
        }
        
        /* Parse field (simplified) */
        if (strchr(field_line, ';') && struct_doc->field_count < 64) {
            struct_field_t* field = &struct_doc->fields[struct_doc->field_count++];
            
            char* semicolon = strchr(field_line, ';');
            *semicolon = '\0';
            
            /* Trim whitespace */
            char* field_content = field_line;
            while (*field_content == ' ' || *field_content == '\t') field_content++;
            
            /* Split type and name */
            char* last_space = strrchr(field_content, ' ');
            if (last_space) {
                *last_space = '\0';
                strncpy(field->type, field_content, sizeof(field->type) - 1);
                strncpy(field->name, last_space + 1, sizeof(field->name) - 1);
            }
        }
    }
    
    struct_doc->is_public = true;
    return 0;
}

/* Parse constant definition */
static int parse_constant_definition(const char* line, constant_doc_t* const_doc) {
    memset(const_doc, 0, sizeof(constant_doc_t));
    
    if (strstr(line, "#define")) {
        /* Parse #define */
        char* define_pos = strstr(line, "#define") + 7;
        while (*define_pos == ' ' || *define_pos == '\t') define_pos++;
        
        char* space_pos = strchr(define_pos, ' ');
        if (space_pos) {
            size_t name_len = space_pos - define_pos;
            strncpy(const_doc->name, define_pos, name_len < sizeof(const_doc->name) ? name_len : sizeof(const_doc->name) - 1);
            
            space_pos++;
            while (*space_pos == ' ' || *space_pos == '\t') space_pos++;
            strncpy(const_doc->value, space_pos, sizeof(const_doc->value) - 1);
        } else {
            strncpy(const_doc->name, define_pos, sizeof(const_doc->name) - 1);
        }
    }
    
    return strlen(const_doc->name) > 0 ? 0 : -1;
}

/* Parse Doxygen-style comments */
static void parse_doxygen_comments(const char* comment, function_doc_t* func_doc) {
    char* comment_copy = strdup(comment);
    char* line = strtok(comment_copy, "\n");
    
    while (line) {
        /* Trim whitespace */
        while (*line == ' ' || *line == '\t') line++;
        
        if (strstr(line, "@brief") || strstr(line, "\\brief")) {
            char* desc_start = strstr(line, "@brief");
            if (!desc_start) desc_start = strstr(line, "\\brief");
            desc_start += 6;
            while (*desc_start == ' ' || *desc_start == '\t') desc_start++;
            
            strncpy(func_doc->description, desc_start, sizeof(func_doc->description) - 1);
            
        } else if (strstr(line, "@param") || strstr(line, "\\param")) {
            char* param_start = strstr(line, "@param");
            if (!param_start) param_start = strstr(line, "\\param");
            param_start += 6;
            while (*param_start == ' ' || *param_start == '\t') param_start++;
            
            /* Extract parameter name and description */
            char* space_pos = strchr(param_start, ' ');
            if (space_pos && func_doc->param_count > 0) {
                *space_pos = '\0';
                space_pos++;
                while (*space_pos == ' ' || *space_pos == '\t') space_pos++;
                
                /* Find matching parameter */
                for (uint32_t i = 0; i < func_doc->param_count; i++) {
                    if (strcmp(func_doc->params[i].name, param_start) == 0) {
                        strncpy(func_doc->params[i].description, space_pos, 
                               sizeof(func_doc->params[i].description) - 1);
                        break;
                    }
                }
            }
            
        } else if (strstr(line, "@return") || strstr(line, "\\return")) {
            char* return_start = strstr(line, "@return");
            if (!return_start) return_start = strstr(line, "\\return");
            return_start += 7;
            while (*return_start == ' ' || *return_start == '\t') return_start++;
            
            strncpy(func_doc->return_desc, return_start, sizeof(func_doc->return_desc) - 1);
            
        } else if (strlen(func_doc->description) == 0) {
            /* Use first non-empty line as description if no @brief found */
            strncpy(func_doc->description, line, sizeof(func_doc->description) - 1);
        }
        
        line = strtok(NULL, "\n");
    }
    
    free(comment_copy);
}

/* Generate documentation */
int doc_generator_generate(doc_format_t format) {
    if (!g_doc_gen.initialized) {
        return -1;
    }
    
    g_doc_gen.output_format = format;
    
    /* Create output directory */
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", g_doc_gen.output_directory);
    system(mkdir_cmd);
    
    printf("Generating documentation in %s format...\n",
           format == DOC_FORMAT_HTML ? "HTML" :
           format == DOC_FORMAT_MARKDOWN ? "Markdown" :
           format == DOC_FORMAT_PDF ? "PDF" : "Unknown");
    
    int result = -1;
    
    switch (format) {
        case DOC_FORMAT_HTML:
            result = generate_html_documentation();
            break;
        case DOC_FORMAT_MARKDOWN:
            result = generate_markdown_documentation();
            break;
        case DOC_FORMAT_PDF:
            result = generate_pdf_documentation();
            break;
        default:
            printf("Unsupported output format: %d\n", format);
            break;
    }
    
    if (result == 0) {
        printf("Documentation generated successfully in %s\n", g_doc_gen.output_directory);
    }
    
    return result;
}

/* Generate HTML documentation */
static int generate_html_documentation(void) {
    char output_file[512];
    
    /* Generate main index page */
    snprintf(output_file, sizeof(output_file), "%s/index.html", g_doc_gen.output_directory);
    
    FILE* index_file = fopen(output_file, "w");
    if (!index_file) {
        return -1;
    }
    
    write_html_header(index_file);
    
    fprintf(index_file, "<h1>%s Documentation</h1>\n", g_doc_gen.project_name);
    fprintf(index_file, "<p>Version: %s</p>\n", g_doc_gen.project_version);
    
    /* Generate function documentation */
    fprintf(index_file, "<h2>Functions</h2>\n");
    fprintf(index_file, "<ul>\n");
    
    for (uint32_t i = 0; i < g_doc_gen.function_count; i++) {
        fprintf(index_file, "<li><a href=\"#func_%s\">%s</a> - %s</li>\n",
                g_doc_gen.functions[i].name,
                g_doc_gen.functions[i].name,
                g_doc_gen.functions[i].description);
    }
    
    fprintf(index_file, "</ul>\n");
    
    /* Write detailed function documentation */
    fprintf(index_file, "<h2>Function Details</h2>\n");
    
    for (uint32_t i = 0; i < g_doc_gen.function_count; i++) {
        write_function_html(index_file, &g_doc_gen.functions[i]);
    }
    
    /* Generate structure documentation */
    fprintf(index_file, "<h2>Structures</h2>\n");
    
    for (uint32_t i = 0; i < g_doc_gen.structure_count; i++) {
        write_structure_html(index_file, &g_doc_gen.structures[i]);
    }
    
    write_html_footer(index_file);
    fclose(index_file);
    
    printf("Generated HTML documentation: %s\n", output_file);
    return 0;
}

/* Write HTML header */
static void write_html_header(FILE* file) {
    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html lang=\"en\">\n");
    fprintf(file, "<head>\n");
    fprintf(file, "    <meta charset=\"UTF-8\">\n");
    fprintf(file, "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(file, "    <title>%s Documentation</title>\n", g_doc_gen.project_name);
    fprintf(file, "    <style>\n");
    fprintf(file, "        body { font-family: Arial, sans-serif; margin: 40px; }\n");
    fprintf(file, "        h1 { color: #333; }\n");
    fprintf(file, "        h2 { color: #666; border-bottom: 1px solid #ccc; }\n");
    fprintf(file, "        .function { margin: 20px 0; padding: 15px; border: 1px solid #ddd; }\n");
    fprintf(file, "        .signature { font-family: monospace; background: #f5f5f5; padding: 10px; }\n");
    fprintf(file, "        .param { margin: 5px 0; }\n");
    fprintf(file, "        .param-name { font-weight: bold; }\n");
    fprintf(file, "    </style>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
}

/* Write HTML footer */
static void write_html_footer(FILE* file) {
    fprintf(file, "<hr>\n");
    fprintf(file, "<p><small>Generated by LimitlessOS Documentation Generator</small></p>\n");
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");
}

/* Write function HTML */
static void write_function_html(FILE* file, const function_doc_t* func_doc) {
    fprintf(file, "<div class=\"function\" id=\"func_%s\">\n", func_doc->name);
    fprintf(file, "<h3>%s</h3>\n", func_doc->name);
    
    /* Function signature */
    fprintf(file, "<div class=\"signature\">%s %s(", func_doc->return_type, func_doc->name);
    
    for (uint32_t i = 0; i < func_doc->param_count; i++) {
        if (i > 0) fprintf(file, ", ");
        fprintf(file, "%s %s", func_doc->params[i].type, func_doc->params[i].name);
    }
    
    fprintf(file, ")</div>\n");
    
    /* Description */
    if (strlen(func_doc->description) > 0) {
        fprintf(file, "<p>%s</p>\n", func_doc->description);
    }
    
    /* Parameters */
    if (func_doc->param_count > 0) {
        fprintf(file, "<h4>Parameters:</h4>\n");
        fprintf(file, "<ul>\n");
        
        for (uint32_t i = 0; i < func_doc->param_count; i++) {
            fprintf(file, "<li><span class=\"param-name\">%s</span> - %s</li>\n",
                    func_doc->params[i].name, func_doc->params[i].description);
        }
        
        fprintf(file, "</ul>\n");
    }
    
    /* Return value */
    if (strlen(func_doc->return_desc) > 0) {
        fprintf(file, "<h4>Returns:</h4>\n");
        fprintf(file, "<p>%s</p>\n", func_doc->return_desc);
    }
    
    /* File information */
    fprintf(file, "<p><small>Defined in: %s:%u</small></p>\n", 
            func_doc->filename, func_doc->line_number);
    
    fprintf(file, "</div>\n");
}

/* Write structure HTML */
static void write_structure_html(FILE* file, const structure_doc_t* struct_doc) {
    fprintf(file, "<div class=\"structure\" id=\"struct_%s\">\n", struct_doc->name);
    fprintf(file, "<h3>struct %s</h3>\n", struct_doc->name);
    
    if (strlen(struct_doc->description) > 0) {
        fprintf(file, "<p>%s</p>\n", struct_doc->description);
    }
    
    /* Fields */
    if (struct_doc->field_count > 0) {
        fprintf(file, "<h4>Fields:</h4>\n");
        fprintf(file, "<ul>\n");
        
        for (uint32_t i = 0; i < struct_doc->field_count; i++) {
            fprintf(file, "<li><span class=\"param-name\">%s %s</span> - %s</li>\n",
                    struct_doc->fields[i].type, struct_doc->fields[i].name,
                    struct_doc->fields[i].description);
        }
        
        fprintf(file, "</ul>\n");
    }
    
    fprintf(file, "<p><small>Defined in: %s:%u</small></p>\n", 
            struct_doc->filename, struct_doc->line_number);
    
    fprintf(file, "</div>\n");
}

/* Generate Markdown documentation */
static int generate_markdown_documentation(void) {
    char output_file[512];
    snprintf(output_file, sizeof(output_file), "%s/README.md", g_doc_gen.output_directory);
    
    FILE* md_file = fopen(output_file, "w");
    if (!md_file) {
        return -1;
    }
    
    fprintf(md_file, "# %s Documentation\n\n", g_doc_gen.project_name);
    fprintf(md_file, "Version: %s\n\n", g_doc_gen.project_version);
    
    /* Functions */
    fprintf(md_file, "## Functions\n\n");
    
    for (uint32_t i = 0; i < g_doc_gen.function_count; i++) {
        const function_doc_t* func = &g_doc_gen.functions[i];
        
        fprintf(md_file, "### %s\n\n", func->name);
        fprintf(md_file, "```c\n%s %s(", func->return_type, func->name);
        
        for (uint32_t j = 0; j < func->param_count; j++) {
            if (j > 0) fprintf(md_file, ", ");
            fprintf(md_file, "%s %s", func->params[j].type, func->params[j].name);
        }
        
        fprintf(md_file, ")\n```\n\n");
        
        if (strlen(func->description) > 0) {
            fprintf(md_file, "%s\n\n", func->description);
        }
        
        /* Parameters */
        if (func->param_count > 0) {
            fprintf(md_file, "**Parameters:**\n\n");
            for (uint32_t j = 0; j < func->param_count; j++) {
                fprintf(md_file, "- `%s` - %s\n", func->params[j].name, func->params[j].description);
            }
            fprintf(md_file, "\n");
        }
        
        /* Return value */
        if (strlen(func->return_desc) > 0) {
            fprintf(md_file, "**Returns:** %s\n\n", func->return_desc);
        }
        
        fprintf(md_file, "*Defined in: %s:%u*\n\n", func->filename, func->line_number);
    }
    
    fclose(md_file);
    
    printf("Generated Markdown documentation: %s\n", output_file);
    return 0;
}

/* Generate PDF documentation (placeholder) */
static int generate_pdf_documentation(void) {
    printf("PDF generation not implemented yet\n");
    return -1;
}

/* Set configuration options */
void doc_generator_set_output_directory(const char* directory) {
    if (directory) {
        strncpy(g_doc_gen.output_directory, directory, sizeof(g_doc_gen.output_directory) - 1);
    }
}

void doc_generator_set_include_private(bool include_private) {
    g_doc_gen.include_private = include_private;
}

/* Main function */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <source_directory> <output_format> [options]\n", argv[0]);
        printf("Output formats: html, markdown, pdf\n");
        printf("Options:\n");
        printf("  --output <dir>      Output directory (default: docs)\n");
        printf("  --project <name>    Project name\n");
        printf("  --version <ver>     Project version\n");
        printf("  --include-private   Include private functions\n");
        return 1;
    }
    
    const char* source_dir = argv[1];
    const char* format_str = argv[2];
    
    /* Parse format */
    doc_format_t format;
    if (strcmp(format_str, "html") == 0) {
        format = DOC_FORMAT_HTML;
    } else if (strcmp(format_str, "markdown") == 0) {
        format = DOC_FORMAT_MARKDOWN;
    } else if (strcmp(format_str, "pdf") == 0) {
        format = DOC_FORMAT_PDF;
    } else {
        printf("Unknown output format: %s\n", format_str);
        return 1;
    }
    
    /* Parse options */
    const char* project_name = "LimitlessOS";
    const char* project_version = "1.0.0";
    
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            doc_generator_set_output_directory(argv[++i]);
        } else if (strcmp(argv[i], "--project") == 0 && i + 1 < argc) {
            project_name = argv[++i];
        } else if (strcmp(argv[i], "--version") == 0 && i + 1 < argc) {
            project_version = argv[++i];
        } else if (strcmp(argv[i], "--include-private") == 0) {
            doc_generator_set_include_private(true);
        }
    }
    
    /* Initialize and run documentation generator */
    if (doc_generator_init(project_name, project_version) < 0) {
        printf("Failed to initialize documentation generator\n");
        return 1;
    }
    
    if (doc_generator_parse_directory(source_dir) < 0) {
        printf("Failed to parse source directory\n");
        return 1;
    }
    
    if (doc_generator_generate(format) < 0) {
        printf("Failed to generate documentation\n");
        return 1;
    }
    
    printf("Documentation generation completed successfully\n");
    printf("Statistics:\n");
    printf("  Files processed: %u\n", g_doc_gen.stats.files_processed);
    printf("  Functions documented: %u\n", g_doc_gen.function_count);
    printf("  Structures documented: %u\n", g_doc_gen.structure_count);
    printf("  Constants documented: %u\n", g_doc_gen.constant_count);
    
    return 0;
}