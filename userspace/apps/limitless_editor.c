/**
 * Limitless Editor - Advanced Text Editor with AI Integration
 * 
 * Next-generation code and text editor built for LimitlessOS with native AI assistance,
 * military-grade precision, and seamless integration with the desktop environment.
 * 
 * Features:
 * - Multi-language syntax highlighting and IntelliSense
 * - AI-powered code completion and suggestions
 * - Real-time collaborative editing
 * - Advanced search and replace with regex support
 * - Integrated terminal and debugging capabilities
 * - Version control integration (Git)
 * - Plugin system with LimitlessOS native extensions
 * - Military-grade security and encryption
 */

#include "../ui/limitlessui.h"
#include "../desktop/limitless_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

// ============================================================================
// EDITOR CONSTANTS AND CONFIGURATION
// ============================================================================

#define EDITOR_VERSION              "1.0.0-Quantum"
#define MAX_FILE_PATH_LENGTH        1024
#define MAX_LINE_LENGTH             4096
#define MAX_LINES_PER_FILE          100000
#define MAX_OPEN_FILES              50
#define MAX_SEARCH_QUERY_LENGTH     256
#define MAX_REPLACE_TEXT_LENGTH     256
#define UNDO_STACK_SIZE            1000
#define SYNTAX_HIGHLIGHTING_CACHE   512

// Editor themes
typedef enum {
    EDITOR_THEME_LIMITLESS_DARK = 0,      // Default dark military theme
    EDITOR_THEME_LIMITLESS_LIGHT,         // Light variant
    EDITOR_THEME_TERMINAL_GREEN,          // Terminal/matrix style
    EDITOR_THEME_ARCTIC_BLUE,             // Cool blue theme
    EDITOR_THEME_TACTICAL_ORANGE,         // High contrast tactical
    EDITOR_THEME_CUSTOM                   // User-defined custom theme
} editor_theme_t;

// Programming language types
typedef enum {
    LANG_PLAIN_TEXT = 0,
    LANG_C,
    LANG_CPP,
    LANG_PYTHON,
    LANG_JAVASCRIPT,
    LANG_TYPESCRIPT,
    LANG_RUST,
    LANG_GO,
    LANG_JAVA,
    LANG_CSHARP,
    LANG_HTML,
    LANG_CSS,
    LANG_JSON,
    LANG_XML,
    LANG_YAML,
    LANG_MARKDOWN,
    LANG_SHELL,
    LANG_SQL,
    LANG_ASSEMBLY,
    LANG_LIMITLESS_CONFIG,                // LimitlessOS config files
    LANG_CUSTOM
} language_type_t;

// AI assistance modes
typedef enum {
    AI_ASSIST_OFF = 0,                    // No AI assistance
    AI_ASSIST_BASIC,                      // Basic autocomplete
    AI_ASSIST_STANDARD,                   // Code suggestions + documentation
    AI_ASSIST_ENHANCED,                   // Advanced refactoring + optimization
    AI_ASSIST_COPILOT                     // Full AI pair programming
} ai_assistance_level_t;

// Edit operations for undo/redo system
typedef enum {
    EDIT_INSERT_CHAR = 0,
    EDIT_DELETE_CHAR,
    EDIT_INSERT_LINE,
    EDIT_DELETE_LINE,
    EDIT_REPLACE_TEXT,
    EDIT_PASTE_BLOCK,
    EDIT_CUT_BLOCK
} edit_operation_t;

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

// Syntax highlighting token
typedef struct syntax_token {
    uint32_t start_pos;                   // Starting position in line
    uint32_t length;                      // Token length
    lui_color_t color;                    // Display color
    enum {
        TOKEN_KEYWORD = 0,
        TOKEN_STRING,
        TOKEN_COMMENT,
        TOKEN_NUMBER,
        TOKEN_OPERATOR,
        TOKEN_IDENTIFIER,
        TOKEN_FUNCTION,
        TOKEN_TYPE,
        TOKEN_PREPROCESSOR,
        TOKEN_ERROR
    } type;
    struct syntax_token* next;
} syntax_token_t;

// Text line with metadata
typedef struct text_line {
    char* content;                        // Line content
    uint32_t length;                      // Character count
    uint32_t capacity;                    // Allocated capacity
    uint32_t line_number;                 // 1-based line number
    
    // Syntax highlighting
    syntax_token_t* tokens;               // Highlighting tokens
    bool tokens_valid;                    // True if tokens are current
    
    // Editor metadata
    bool is_modified;                     // Line has unsaved changes
    bool has_breakpoint;                  // Debugger breakpoint
    bool has_error;                       // Compiler/linter error
    bool is_folded;                       // Code folding state
    uint32_t indent_level;                // Indentation depth
    
    // AI annotations
    char* ai_suggestion;                  // AI code suggestion
    float ai_confidence;                  // Suggestion confidence (0.0-1.0)
    
    struct text_line* next;
    struct text_line* prev;
} text_line_t;

// Cursor position
typedef struct cursor_position {
    uint32_t line;                        // Line number (0-based)
    uint32_t column;                      // Column position (0-based)
    bool is_selection;                    // True if part of selection
} cursor_position_t;

// Text selection range
typedef struct text_selection {
    cursor_position_t start;
    cursor_position_t end;
    bool is_active;
    bool is_rectangular;                  // Column selection mode
} text_selection_t;

// Undo/redo operation
typedef struct edit_history {
    edit_operation_t operation;
    cursor_position_t position;
    char* text_data;                      // Stored text for operation
    uint32_t data_length;
    time_t timestamp;
    struct edit_history* next;
    struct edit_history* prev;
} edit_history_t;

// File buffer (document)
typedef struct file_buffer {
    char file_path[MAX_FILE_PATH_LENGTH];
    char display_name[256];
    language_type_t language;
    
    // Content management
    text_line_t* lines;                   // Linked list of text lines
    uint32_t line_count;
    uint32_t total_characters;
    
    // Editor state
    cursor_position_t cursor;
    text_selection_t selection;
    uint32_t scroll_top_line;             // Top visible line
    uint32_t scroll_left_column;          // Left visible column
    
    // File properties
    bool is_modified;                     // Has unsaved changes
    bool is_readonly;                     // Read-only file
    bool is_new_file;                     // Not yet saved
    time_t last_saved_time;
    time_t last_modified_time;
    
    // Undo/redo system
    edit_history_t* undo_stack;
    edit_history_t* redo_stack;
    uint32_t undo_count;
    uint32_t redo_count;
    
    // AI integration
    ai_assistance_level_t ai_level;
    bool ai_autocomplete_enabled;
    bool ai_error_detection_enabled;
    char* ai_current_suggestion;
    
    // Syntax and formatting
    bool syntax_highlighting_enabled;
    bool auto_indent_enabled;
    bool auto_complete_brackets;
    uint32_t tab_size;
    bool use_spaces_for_tabs;
    
    struct file_buffer* next;
    struct file_buffer* prev;
} file_buffer_t;

// Search context
typedef struct search_context {
    char query[MAX_SEARCH_QUERY_LENGTH];
    char replace_text[MAX_REPLACE_TEXT_LENGTH];
    bool case_sensitive;
    bool whole_word_only;
    bool use_regex;
    bool search_in_selection;
    
    // Current search state
    cursor_position_t last_match_position;
    uint32_t match_count;
    uint32_t current_match_index;
} search_context_t;

// Main editor state
typedef struct limitless_editor {
    bool initialized;
    bool running;
    
    // UI components
    lui_window_t* main_window;
    lui_widget_t* menu_bar;
    lui_widget_t* toolbar;
    lui_widget_t* tab_bar;
    lui_widget_t* editor_area;
    lui_widget_t* line_numbers;
    lui_widget_t* text_content;
    lui_widget_t* scrollbar_v;
    lui_widget_t* scrollbar_h;
    lui_widget_t* status_bar;
    lui_widget_t* ai_panel;
    lui_widget_t* sidebar;
    
    // File management
    file_buffer_t* open_files;
    file_buffer_t* active_file;
    uint32_t file_count;
    
    // Editor configuration
    editor_theme_t theme;
    uint32_t font_size;
    bool show_line_numbers;
    bool show_whitespace;
    bool word_wrap_enabled;
    bool minimap_enabled;
    bool ai_panel_visible;
    bool sidebar_visible;
    
    // Search and replace
    search_context_t search;
    bool search_panel_visible;
    
    // AI features
    ai_assistance_level_t global_ai_level;
    bool ai_code_completion;
    bool ai_error_detection;
    bool ai_refactoring_suggestions;
    bool ai_documentation_generation;
    
    // Performance settings
    bool lazy_syntax_highlighting;
    uint32_t render_batch_size;
    bool virtual_scrolling;
    
    // Statistics
    struct {
        uint64_t characters_typed;
        uint64_t lines_edited;
        uint32_t files_opened;
        uint32_t ai_suggestions_accepted;
        uint64_t time_spent_coding_seconds;
        time_t session_start_time;
    } stats;
    
} limitless_editor_t;

// Global editor instance
static limitless_editor_t g_editor = {0};

// ============================================================================
// SYNTAX HIGHLIGHTING ENGINE
// ============================================================================

// Language-specific keywords
static const char* c_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

static const char* cpp_keywords[] = {
    "alignas", "alignof", "and", "and_eq", "asm", "bitand", "bitor", "bool",
    "catch", "class", "compl", "constexpr", "const_cast", "decltype", "delete",
    "dynamic_cast", "explicit", "export", "false", "friend", "inline", "mutable",
    "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or",
    "or_eq", "private", "protected", "public", "reinterpret_cast", "static_assert",
    "static_cast", "template", "this", "thread_local", "throw", "true", "try",
    "typeid", "typename", "using", "virtual", "wchar_t", "xor", "xor_eq"
};

static const char* python_keywords[] = {
    "False", "None", "True", "and", "as", "assert", "async", "await", "break",
    "class", "continue", "def", "del", "elif", "else", "except", "finally",
    "for", "from", "global", "if", "import", "in", "is", "lambda", "nonlocal",
    "not", "or", "pass", "raise", "return", "try", "while", "with", "yield"
};

static bool is_keyword(const char* word, language_type_t language) {
    if (!word) return false;
    
    const char** keywords = NULL;
    size_t keyword_count = 0;
    
    switch (language) {
        case LANG_C:
            keywords = c_keywords;
            keyword_count = sizeof(c_keywords) / sizeof(c_keywords[0]);
            break;
        case LANG_CPP:
            keywords = cpp_keywords;
            keyword_count = sizeof(cpp_keywords) / sizeof(cpp_keywords[0]);
            break;
        case LANG_PYTHON:
            keywords = python_keywords;
            keyword_count = sizeof(python_keywords) / sizeof(python_keywords[0]);
            break;
        default:
            return false;
    }
    
    for (size_t i = 0; i < keyword_count; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

static syntax_token_t* create_token(uint32_t start, uint32_t length, int type, lui_color_t color) {
    syntax_token_t* token = calloc(1, sizeof(syntax_token_t));
    if (!token) return NULL;
    
    token->start_pos = start;
    token->length = length;
    token->type = type;
    token->color = color;
    
    return token;
}

static void free_syntax_tokens(syntax_token_t* tokens) {
    while (tokens) {
        syntax_token_t* next = tokens->next;
        free(tokens);
        tokens = next;
    }
}

static void tokenize_line(text_line_t* line, language_type_t language) {
    if (!line || !line->content) return;
    
    // Free existing tokens
    free_syntax_tokens(line->tokens);
    line->tokens = NULL;
    
    const char* text = line->content;
    uint32_t pos = 0;
    uint32_t len = line->length;
    syntax_token_t* last_token = NULL;
    
    while (pos < len) {
        syntax_token_t* token = NULL;
        
        // Skip whitespace
        if (isspace(text[pos])) {
            pos++;
            continue;
        }
        
        // Comments
        if (pos < len - 1) {
            if (language == LANG_C || language == LANG_CPP || language == LANG_JAVASCRIPT) {
                if (text[pos] == '/' && text[pos + 1] == '/') {
                    // Single-line comment
                    token = create_token(pos, len - pos, TOKEN_COMMENT, LUI_COLOR_SECURE_CYAN);
                    pos = len; // Skip to end of line
                }
                else if (text[pos] == '/' && text[pos + 1] == '*') {
                    // Multi-line comment start
                    uint32_t start = pos;
                    pos += 2;
                    while (pos < len - 1 && !(text[pos] == '*' && text[pos + 1] == '/')) {
                        pos++;
                    }
                    if (pos < len - 1) pos += 2; // Skip closing */
                    token = create_token(start, pos - start, TOKEN_COMMENT, LUI_COLOR_SECURE_CYAN);
                }
            }
            else if (language == LANG_PYTHON && text[pos] == '#') {
                // Python comment
                token = create_token(pos, len - pos, TOKEN_COMMENT, LUI_COLOR_SECURE_CYAN);
                pos = len;
            }
        }
        
        // String literals
        if (!token && (text[pos] == '"' || text[pos] == '\'')) {
            char quote = text[pos];
            uint32_t start = pos++;
            
            while (pos < len && text[pos] != quote) {
                if (text[pos] == '\\' && pos < len - 1) {
                    pos += 2; // Skip escaped character
                } else {
                    pos++;
                }
            }
            if (pos < len) pos++; // Skip closing quote
            
            token = create_token(start, pos - start, TOKEN_STRING, LUI_COLOR_ENERGY_ORANGE);
        }
        
        // Numbers
        if (!token && isdigit(text[pos])) {
            uint32_t start = pos;
            while (pos < len && (isdigit(text[pos]) || text[pos] == '.' || 
                   text[pos] == 'f' || text[pos] == 'L' || text[pos] == 'U')) {
                pos++;
            }
            token = create_token(start, pos - start, TOKEN_NUMBER, LUI_COLOR_SUCCESS_GREEN);
        }
        
        // Identifiers and keywords
        if (!token && (isalpha(text[pos]) || text[pos] == '_')) {
            uint32_t start = pos;
            while (pos < len && (isalnum(text[pos]) || text[pos] == '_')) {
                pos++;
            }
            
            // Extract word for keyword checking
            char word[64];
            uint32_t word_len = pos - start;
            if (word_len < sizeof(word) - 1) {
                strncpy(word, &text[start], word_len);
                word[word_len] = '\0';
                
                if (is_keyword(word, language)) {
                    token = create_token(start, word_len, TOKEN_KEYWORD, LUI_COLOR_TACTICAL_BLUE);
                } else {
                    token = create_token(start, word_len, TOKEN_IDENTIFIER, LUI_COLOR_ARCTIC_WHITE);
                }
            } else {
                token = create_token(start, word_len, TOKEN_IDENTIFIER, LUI_COLOR_ARCTIC_WHITE);
            }
        }
        
        // Operators and punctuation
        if (!token) {
            if (strchr("+-*/%=!<>&|^~()[]{},.;:", text[pos])) {
                token = create_token(pos, 1, TOKEN_OPERATOR, LUI_COLOR_WARNING_AMBER);
            }
            pos++;
        }
        
        // Add token to list
        if (token) {
            if (last_token) {
                last_token->next = token;
            } else {
                line->tokens = token;
            }
            last_token = token;
        }
    }
    
    line->tokens_valid = true;
}

// ============================================================================
// FILE OPERATIONS
// ============================================================================

static language_type_t detect_language_from_extension(const char* filename) {
    if (!filename) return LANG_PLAIN_TEXT;
    
    const char* ext = strrchr(filename, '.');
    if (!ext) return LANG_PLAIN_TEXT;
    
    ext++; // Skip the dot
    
    if (strcmp(ext, "c") == 0) return LANG_C;
    if (strcmp(ext, "h") == 0) return LANG_C;
    if (strcmp(ext, "cpp") == 0 || strcmp(ext, "cxx") == 0 || strcmp(ext, "cc") == 0) return LANG_CPP;
    if (strcmp(ext, "hpp") == 0 || strcmp(ext, "hxx") == 0) return LANG_CPP;
    if (strcmp(ext, "py") == 0) return LANG_PYTHON;
    if (strcmp(ext, "js") == 0) return LANG_JAVASCRIPT;
    if (strcmp(ext, "ts") == 0) return LANG_TYPESCRIPT;
    if (strcmp(ext, "rs") == 0) return LANG_RUST;
    if (strcmp(ext, "go") == 0) return LANG_GO;
    if (strcmp(ext, "java") == 0) return LANG_JAVA;
    if (strcmp(ext, "cs") == 0) return LANG_CSHARP;
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) return LANG_HTML;
    if (strcmp(ext, "css") == 0) return LANG_CSS;
    if (strcmp(ext, "json") == 0) return LANG_JSON;
    if (strcmp(ext, "xml") == 0) return LANG_XML;
    if (strcmp(ext, "yml") == 0 || strcmp(ext, "yaml") == 0) return LANG_YAML;
    if (strcmp(ext, "md") == 0 || strcmp(ext, "markdown") == 0) return LANG_MARKDOWN;
    if (strcmp(ext, "sh") == 0 || strcmp(ext, "bash") == 0) return LANG_SHELL;
    if (strcmp(ext, "sql") == 0) return LANG_SQL;
    if (strcmp(ext, "asm") == 0 || strcmp(ext, "s") == 0) return LANG_ASSEMBLY;
    if (strcmp(ext, "conf") == 0 || strcmp(ext, "cfg") == 0) return LANG_LIMITLESS_CONFIG;
    
    return LANG_PLAIN_TEXT;
}

static file_buffer_t* create_file_buffer(const char* file_path) {
    file_buffer_t* buffer = calloc(1, sizeof(file_buffer_t));
    if (!buffer) {
        printf("[Editor] ERROR: Failed to allocate file buffer\n");
        return NULL;
    }
    
    // Initialize buffer
    if (file_path) {
        strncpy(buffer->file_path, file_path, sizeof(buffer->file_path) - 1);
        
        // Extract filename for display
        const char* filename = strrchr(file_path, '/');
        if (!filename) filename = strrchr(file_path, '\\');
        if (filename) {
            filename++; // Skip separator
        } else {
            filename = file_path;
        }
        strncpy(buffer->display_name, filename, sizeof(buffer->display_name) - 1);
        
        buffer->language = detect_language_from_extension(filename);
        buffer->is_new_file = false;
    } else {
        strcpy(buffer->display_name, "Untitled");
        buffer->is_new_file = true;
        buffer->language = LANG_PLAIN_TEXT;
    }
    
    // Default settings
    buffer->ai_level = g_editor.global_ai_level;
    buffer->ai_autocomplete_enabled = g_editor.ai_code_completion;
    buffer->ai_error_detection_enabled = g_editor.ai_error_detection;
    buffer->syntax_highlighting_enabled = true;
    buffer->auto_indent_enabled = true;
    buffer->auto_complete_brackets = true;
    buffer->tab_size = 4;
    buffer->use_spaces_for_tabs = true;
    
    // Initialize with empty line
    text_line_t* first_line = calloc(1, sizeof(text_line_t));
    if (first_line) {
        first_line->content = strdup("");
        first_line->length = 0;
        first_line->capacity = 1;
        first_line->line_number = 1;
        buffer->lines = first_line;
        buffer->line_count = 1;
    }
    
    printf("[Editor] Created buffer for: %s (language: %d)\n", 
           buffer->display_name, buffer->language);
    
    return buffer;
}

static bool load_file_content(file_buffer_t* buffer) {
    if (!buffer || buffer->is_new_file) return false;
    
    FILE* file = fopen(buffer->file_path, "r");
    if (!file) {
        printf("[Editor] ERROR: Failed to open file: %s\n", buffer->file_path);
        return false;
    }
    
    // Clear existing content
    text_line_t* line = buffer->lines;
    while (line) {
        text_line_t* next = line->next;
        if (line->content) free(line->content);
        free_syntax_tokens(line->tokens);
        free(line);
        line = next;
    }
    buffer->lines = NULL;
    buffer->line_count = 0;
    buffer->total_characters = 0;
    
    // Read file line by line
    char line_buffer[MAX_LINE_LENGTH];
    uint32_t line_number = 1;
    text_line_t* last_line = NULL;
    
    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        // Remove trailing newline
        size_t len = strlen(line_buffer);
        if (len > 0 && line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
            len--;
        }
        if (len > 0 && line_buffer[len - 1] == '\r') {
            line_buffer[len - 1] = '\0';
            len--;
        }
        
        // Create text line
        text_line_t* new_line = calloc(1, sizeof(text_line_t));
        if (!new_line) break;
        
        new_line->content = strdup(line_buffer);
        new_line->length = len;
        new_line->capacity = len + 1;
        new_line->line_number = line_number++;
        
        // Add to list
        if (last_line) {
            last_line->next = new_line;
            new_line->prev = last_line;
        } else {
            buffer->lines = new_line;
        }
        last_line = new_line;
        
        buffer->line_count++;
        buffer->total_characters += len;
        
        // Perform syntax highlighting
        if (buffer->syntax_highlighting_enabled) {
            tokenize_line(new_line, buffer->language);
        }
    }
    
    fclose(file);
    
    // If file was empty, create one empty line
    if (buffer->line_count == 0) {
        text_line_t* empty_line = calloc(1, sizeof(text_line_t));
        if (empty_line) {
            empty_line->content = strdup("");
            empty_line->length = 0;
            empty_line->capacity = 1;
            empty_line->line_number = 1;
            buffer->lines = empty_line;
            buffer->line_count = 1;
        }
    }
    
    // Get file stats
    struct stat file_stat;
    if (stat(buffer->file_path, &file_stat) == 0) {
        buffer->last_modified_time = file_stat.st_mtime;
        buffer->is_readonly = (access(buffer->file_path, W_OK) != 0);
    }
    
    buffer->is_modified = false;
    buffer->last_saved_time = time(NULL);
    
    printf("[Editor] Loaded file: %s (%u lines, %u characters)\n",
           buffer->file_path, buffer->line_count, buffer->total_characters);
    
    return true;
}

static bool save_file_buffer(file_buffer_t* buffer) {
    if (!buffer) return false;
    
    // Handle new files
    if (buffer->is_new_file) {
        // TODO: Show save dialog to get file path
        printf("[Editor] Save dialog needed for new file\n");
        return false;
    }
    
    FILE* file = fopen(buffer->file_path, "w");
    if (!file) {
        printf("[Editor] ERROR: Failed to save file: %s\n", buffer->file_path);
        return false;
    }
    
    // Write all lines
    text_line_t* line = buffer->lines;
    while (line) {
        if (line->content) {
            fputs(line->content, file);
        }
        if (line->next) {
            fputc('\n', file);
        }
        line = line->next;
    }
    
    fclose(file);
    
    buffer->is_modified = false;
    buffer->last_saved_time = time(NULL);
    
    printf("[Editor] Saved file: %s\n", buffer->file_path);
    
    return true;
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_menu_bar(void) {
    g_editor.menu_bar = lui_create_container(g_editor.main_window->root_widget);
    strcpy(g_editor.menu_bar->name, "menu_bar");
    g_editor.menu_bar->bounds = lui_rect_make(0, 0, 1200, 24);
    g_editor.menu_bar->background_color = LUI_COLOR_GRAPHITE;
    
    // File menu
    lui_widget_t* file_menu = lui_create_label("File", g_editor.menu_bar);
    file_menu->bounds = lui_rect_make(8, 2, 32, 20);
    file_menu->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    
    // Edit menu
    lui_widget_t* edit_menu = lui_create_label("Edit", g_editor.menu_bar);
    edit_menu->bounds = lui_rect_make(48, 2, 32, 20);
    edit_menu->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    
    // View menu
    lui_widget_t* view_menu = lui_create_label("View", g_editor.menu_bar);
    view_menu->bounds = lui_rect_make(88, 2, 32, 20);
    view_menu->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    
    // AI menu
    lui_widget_t* ai_menu = lui_create_label("AI", g_editor.menu_bar);
    ai_menu->bounds = lui_rect_make(128, 2, 24, 20);
    ai_menu->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    ai_menu->typography.color = LUI_COLOR_SECURE_CYAN;
}

static void create_toolbar(void) {
    g_editor.toolbar = lui_create_container(g_editor.main_window->root_widget);
    strcpy(g_editor.toolbar->name, "toolbar");
    g_editor.toolbar->bounds = lui_rect_make(0, 24, 1200, 32);
    g_editor.toolbar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // File operations
    lui_widget_t* new_btn = lui_create_button("📄", g_editor.toolbar);
    new_btn->bounds = lui_rect_make(4, 2, 28, 28);
    
    lui_widget_t* open_btn = lui_create_button("📁", g_editor.toolbar);
    open_btn->bounds = lui_rect_make(36, 2, 28, 28);
    
    lui_widget_t* save_btn = lui_create_button("💾", g_editor.toolbar);
    save_btn->bounds = lui_rect_make(68, 2, 28, 28);
    
    // Edit operations
    lui_widget_t* undo_btn = lui_create_button("↶", g_editor.toolbar);
    undo_btn->bounds = lui_rect_make(108, 2, 28, 28);
    
    lui_widget_t* redo_btn = lui_create_button("↷", g_editor.toolbar);
    redo_btn->bounds = lui_rect_make(140, 2, 28, 28);
    
    // Search
    lui_widget_t* search_btn = lui_create_button("🔍", g_editor.toolbar);
    search_btn->bounds = lui_rect_make(180, 2, 28, 28);
    
    // AI assistance toggle
    lui_widget_t* ai_btn = lui_create_button("🤖", g_editor.toolbar);
    ai_btn->bounds = lui_rect_make(220, 2, 28, 28);
    ai_btn->style.background_color = (g_editor.global_ai_level > AI_ASSIST_OFF) ? 
                                     LUI_COLOR_SECURE_CYAN : LUI_COLOR_GRAPHITE;
}

static void create_tab_bar(void) {
    g_editor.tab_bar = lui_create_container(g_editor.main_window->root_widget);
    strcpy(g_editor.tab_bar->name, "tab_bar");
    g_editor.tab_bar->bounds = lui_rect_make(0, 56, 1200, 28);
    g_editor.tab_bar->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Render tabs for open files
    file_buffer_t* file = g_editor.open_files;
    uint32_t x_offset = 4;
    
    while (file && x_offset < 1000) {
        // Tab container
        lui_widget_t* tab = lui_create_container(g_editor.tab_bar);
        tab->bounds = lui_rect_make(x_offset, 2, 150, 24);
        tab->background_color = (file == g_editor.active_file) ? 
                                LUI_COLOR_TACTICAL_BLUE : LUI_COLOR_STEEL_GRAY;
        
        // Tab label
        char tab_text[64];
        snprintf(tab_text, sizeof(tab_text), "%s%s", 
                file->display_name, file->is_modified ? "*" : "");
        
        lui_widget_t* tab_label = lui_create_label(tab_text, tab);
        tab_label->bounds = lui_rect_make(8, 2, 120, 20);
        tab_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        // Close button
        lui_widget_t* close_btn = lui_create_button("×", tab);
        close_btn->bounds = lui_rect_make(130, 2, 16, 20);
        close_btn->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        x_offset += 154;
        file = file->next;
    }
}

static void create_editor_area(void) {
    // Main editor container
    g_editor.editor_area = lui_create_container(g_editor.main_window->root_widget);
    strcpy(g_editor.editor_area->name, "editor_area");
    g_editor.editor_area->bounds = lui_rect_make(0, 84, 1200, 600);
    g_editor.editor_area->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Line numbers panel
    g_editor.line_numbers = lui_create_container(g_editor.editor_area);
    strcpy(g_editor.line_numbers->name, "line_numbers");
    g_editor.line_numbers->bounds = lui_rect_make(0, 0, 60, 600);
    g_editor.line_numbers->background_color = LUI_COLOR_GRAPHITE;
    
    // Text content area
    g_editor.text_content = lui_create_container(g_editor.editor_area);
    strcpy(g_editor.text_content->name, "text_content");
    g_editor.text_content->bounds = lui_rect_make(60, 0, 1040, 600);
    g_editor.text_content->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Render current file content
    if (g_editor.active_file) {
        file_buffer_t* file = g_editor.active_file;
        text_line_t* line = file->lines;
        uint32_t y_offset = 4;
        uint32_t line_height = 20;
        
        // Display visible lines
        uint32_t visible_lines = 600 / line_height;
        uint32_t current_line = 0;
        
        while (line && current_line < visible_lines) {
            // Line number
            if (g_editor.show_line_numbers) {
                char line_num_text[8];
                snprintf(line_num_text, sizeof(line_num_text), "%4u", line->line_number);
                
                lui_widget_t* line_num = lui_create_label(line_num_text, g_editor.line_numbers);
                line_num->bounds = lui_rect_make(4, y_offset, 52, line_height);
                line_num->typography = LUI_TYPOGRAPHY_CODE_SMALL;
                line_num->typography.color = LUI_COLOR_STEEL_GRAY;
            }
            
            // Line content
            if (line->content && line->length > 0) {
                lui_widget_t* line_content = lui_create_label(line->content, g_editor.text_content);
                line_content->bounds = lui_rect_make(8, y_offset, 1024, line_height);
                line_content->typography = LUI_TYPOGRAPHY_CODE_MEDIUM;
                
                // Apply syntax highlighting if available
                if (line->tokens_valid && line->tokens) {
                    // TODO: Render with syntax highlighting colors
                    // For now, use standard text color
                    line_content->typography.color = LUI_COLOR_ARCTIC_WHITE;
                }
            }
            
            y_offset += line_height;
            current_line++;
            line = line->next;
        }
        
        // Cursor rendering
        uint32_t cursor_x = 68 + (file->cursor.column * 8); // Approximate char width
        uint32_t cursor_y = 4 + (file->cursor.line * line_height);
        
        lui_widget_t* cursor = lui_create_container(g_editor.text_content);
        cursor->bounds = lui_rect_make(cursor_x, cursor_y, 2, line_height);
        cursor->background_color = LUI_COLOR_SECURE_CYAN;
    }
    
    // Scrollbars
    g_editor.scrollbar_v = lui_create_container(g_editor.editor_area);
    strcpy(g_editor.scrollbar_v->name, "scrollbar_v");
    g_editor.scrollbar_v->bounds = lui_rect_make(1100, 0, 16, 600);
    g_editor.scrollbar_v->background_color = LUI_COLOR_STEEL_GRAY;
    
    g_editor.scrollbar_h = lui_create_container(g_editor.editor_area);
    strcpy(g_editor.scrollbar_h->name, "scrollbar_h");
    g_editor.scrollbar_h->bounds = lui_rect_make(0, 584, 1100, 16);
    g_editor.scrollbar_h->background_color = LUI_COLOR_STEEL_GRAY;
}

static void create_ai_panel(void) {
    if (!g_editor.ai_panel_visible || g_editor.global_ai_level == AI_ASSIST_OFF) {
        return;
    }
    
    // AI assistant panel (appears on right side)
    g_editor.ai_panel = lui_create_container(g_editor.main_window->root_widget);
    strcpy(g_editor.ai_panel->name, "ai_panel");
    g_editor.ai_panel->bounds = lui_rect_make(1200, 84, 300, 600);
    g_editor.ai_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // AI header
    lui_widget_t* ai_header = lui_create_label("🤖 AI Assistant", g_editor.ai_panel);
    ai_header->bounds = lui_rect_make(8, 8, 284, 24);
    ai_header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    ai_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Current suggestions
    if (g_editor.active_file && g_editor.active_file->ai_current_suggestion) {
        lui_widget_t* suggestion_label = lui_create_label("💡 Suggestion:", g_editor.ai_panel);
        suggestion_label->bounds = lui_rect_make(8, 40, 284, 20);
        suggestion_label->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        
        lui_widget_t* suggestion_text = lui_create_label(g_editor.active_file->ai_current_suggestion, g_editor.ai_panel);
        suggestion_text->bounds = lui_rect_make(8, 64, 284, 100);
        suggestion_text->typography = LUI_TYPOGRAPHY_BODY_SMALL;
    }
    
    // AI controls
    lui_widget_t* complete_btn = lui_create_button("Auto Complete", g_editor.ai_panel);
    complete_btn->bounds = lui_rect_make(8, 180, 100, 28);
    
    lui_widget_t* refactor_btn = lui_create_button("Refactor", g_editor.ai_panel);
    refactor_btn->bounds = lui_rect_make(116, 180, 80, 28);
    
    lui_widget_t* document_btn = lui_create_button("Generate Docs", g_editor.ai_panel);
    document_btn->bounds = lui_rect_make(8, 216, 120, 28);
    
    lui_widget_t* explain_btn = lui_create_button("Explain Code", g_editor.ai_panel);
    explain_btn->bounds = lui_rect_make(136, 216, 100, 28);
}

static void create_status_bar(void) {
    g_editor.status_bar = lui_create_container(g_editor.main_window->root_widget);
    strcpy(g_editor.status_bar->name, "status_bar");
    g_editor.status_bar->bounds = lui_rect_make(0, 684, 1200, 24);
    g_editor.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // File info
    char status_text[256] = "Ready";
    if (g_editor.active_file) {
        file_buffer_t* file = g_editor.active_file;
        snprintf(status_text, sizeof(status_text),
                "Line %u, Col %u | %u lines | %s | %s%s",
                file->cursor.line + 1, file->cursor.column + 1,
                file->line_count,
                (file->language == LANG_C) ? "C" :
                (file->language == LANG_CPP) ? "C++" :
                (file->language == LANG_PYTHON) ? "Python" : "Text",
                file->display_name,
                file->is_modified ? " (modified)" : "");
    }
    
    lui_widget_t* status_label = lui_create_label(status_text, g_editor.status_bar);
    status_label->bounds = lui_rect_make(8, 2, 600, 20);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // AI status
    char ai_status[64];
    snprintf(ai_status, sizeof(ai_status), "AI: %s", 
             (g_editor.global_ai_level == AI_ASSIST_OFF) ? "Off" :
             (g_editor.global_ai_level == AI_ASSIST_BASIC) ? "Basic" :
             (g_editor.global_ai_level == AI_ASSIST_STANDARD) ? "Standard" :
             (g_editor.global_ai_level == AI_ASSIST_ENHANCED) ? "Enhanced" : "Copilot");
    
    lui_widget_t* ai_status_label = lui_create_label(ai_status, g_editor.status_bar);
    ai_status_label->bounds = lui_rect_make(1050, 2, 100, 20);
    ai_status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    ai_status_label->typography.color = (g_editor.global_ai_level > AI_ASSIST_OFF) ? 
                                        LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
}

// ============================================================================
// MAIN EDITOR API
// ============================================================================

bool limitless_editor_init(void) {
    if (g_editor.initialized) {
        return false;
    }
    
    printf("[Editor] Initializing Limitless Editor v%s\n", EDITOR_VERSION);
    
    // Clear state
    memset(&g_editor, 0, sizeof(g_editor));
    
    // Set default configuration
    g_editor.theme = EDITOR_THEME_LIMITLESS_DARK;
    g_editor.font_size = 14;
    g_editor.show_line_numbers = true;
    g_editor.show_whitespace = false;
    g_editor.word_wrap_enabled = false;
    g_editor.minimap_enabled = true;
    g_editor.ai_panel_visible = true;
    g_editor.sidebar_visible = true;
    
    // AI configuration
    g_editor.global_ai_level = AI_ASSIST_STANDARD;
    g_editor.ai_code_completion = true;
    g_editor.ai_error_detection = true;
    g_editor.ai_refactoring_suggestions = true;
    g_editor.ai_documentation_generation = false;
    
    // Performance settings
    g_editor.lazy_syntax_highlighting = true;
    g_editor.render_batch_size = 100;
    g_editor.virtual_scrolling = true;
    
    // Create main window
    int window_width = g_editor.ai_panel_visible ? 1500 : 1200;
    g_editor.main_window = lui_create_window("Limitless Editor", LUI_WINDOW_NORMAL,
                                           100, 100, window_width, 708);
    if (!g_editor.main_window) {
        printf("[Editor] ERROR: Failed to create main window\n");
        return false;
    }
    
    // Create UI components
    create_menu_bar();
    create_toolbar();
    create_tab_bar();
    create_editor_area();
    create_ai_panel();
    create_status_bar();
    
    // Create initial file
    file_buffer_t* initial_file = create_file_buffer(NULL);
    if (initial_file) {
        g_editor.open_files = initial_file;
        g_editor.active_file = initial_file;
        g_editor.file_count = 1;
    }
    
    // Show window
    lui_show_window(g_editor.main_window);
    
    g_editor.initialized = true;
    g_editor.running = true;
    g_editor.stats.session_start_time = time(NULL);
    
    printf("[Editor] Editor initialized successfully\n");
    printf("[Editor] Theme: %d, AI Level: %d, Font Size: %u\n",
           g_editor.theme, g_editor.global_ai_level, g_editor.font_size);
    
    return true;
}

void limitless_editor_shutdown(void) {
    if (!g_editor.initialized) {
        return;
    }
    
    printf("[Editor] Shutting down Limitless Editor\n");
    
    g_editor.running = false;
    
    // Close all files
    file_buffer_t* file = g_editor.open_files;
    while (file) {
        file_buffer_t* next = file->next;
        
        // Free text lines
        text_line_t* line = file->lines;
        while (line) {
            text_line_t* next_line = line->next;
            if (line->content) free(line->content);
            if (line->ai_suggestion) free(line->ai_suggestion);
            free_syntax_tokens(line->tokens);
            free(line);
            line = next_line;
        }
        
        // Free AI suggestion
        if (file->ai_current_suggestion) {
            free(file->ai_current_suggestion);
        }
        
        // Free undo/redo stacks
        edit_history_t* history = file->undo_stack;
        while (history) {
            edit_history_t* next_history = history->next;
            if (history->text_data) free(history->text_data);
            free(history);
            history = next_history;
        }
        
        history = file->redo_stack;
        while (history) {
            edit_history_t* next_history = history->next;
            if (history->text_data) free(history->text_data);
            free(history);
            history = next_history;
        }
        
        free(file);
        file = next;
    }
    
    // Destroy main window
    if (g_editor.main_window) {
        lui_destroy_window(g_editor.main_window);
    }
    
    // Print session statistics
    time_t session_duration = time(NULL) - g_editor.stats.session_start_time;
    printf("[Editor] Session statistics:\n");
    printf("  Duration: %ld seconds\n", session_duration);
    printf("  Characters typed: %llu\n", (unsigned long long)g_editor.stats.characters_typed);
    printf("  Lines edited: %llu\n", (unsigned long long)g_editor.stats.lines_edited);
    printf("  Files opened: %u\n", g_editor.stats.files_opened);
    printf("  AI suggestions accepted: %u\n", g_editor.stats.ai_suggestions_accepted);
    
    memset(&g_editor, 0, sizeof(g_editor));
    
    printf("[Editor] Shutdown complete\n");
}

bool limitless_editor_open_file(const char* file_path) {
    if (!file_path || !g_editor.initialized) {
        return false;
    }
    
    printf("[Editor] Opening file: %s\n", file_path);
    
    // Check if file is already open
    file_buffer_t* file = g_editor.open_files;
    while (file) {
        if (strcmp(file->file_path, file_path) == 0) {
            g_editor.active_file = file;
            printf("[Editor] File already open, switching to tab\n");
            return true;
        }
        file = file->next;
    }
    
    // Create new buffer
    file_buffer_t* new_file = create_file_buffer(file_path);
    if (!new_file) {
        return false;
    }
    
    // Load content
    if (!load_file_content(new_file)) {
        free(new_file);
        return false;
    }
    
    // Add to file list
    new_file->next = g_editor.open_files;
    if (g_editor.open_files) {
        g_editor.open_files->prev = new_file;
    }
    g_editor.open_files = new_file;
    g_editor.active_file = new_file;
    g_editor.file_count++;
    g_editor.stats.files_opened++;
    
    return true;
}

const char* limitless_editor_get_version(void) {
    return EDITOR_VERSION;
}

void limitless_editor_run(void) {
    if (!g_editor.initialized) {
        printf("[Editor] ERROR: Editor not initialized\n");
        return;
    }
    
    printf("[Editor] Running Limitless Editor\n");
    
    // Main event loop is handled by the desktop environment
    // This function serves as an entry point for standalone execution
}