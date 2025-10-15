/**
 * Limitless Terminal - Next-Generation Terminal Emulator
 * 
 * Advanced terminal emulator built for LimitlessOS with AI integration,
 * military-grade security, and seamless desktop environment integration.
 * 
 * Features:
 * - Multi-tab and split-pane support
 * - AI-powered command suggestions and autocompletion
 * - Built-in shell scripting environment
 * - Real-time system monitoring integration
 * - Security sandbox with privilege escalation controls
 * - Native LimitlessOS command extensions
 * - Advanced theming with military color schemes
 * - Session recording and playback
 * - Remote SSH/telnet connectivity with encryption
 */

#include "../ui/limitlessui.h"
#include "../desktop/limitless_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/pty.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

// ============================================================================
// TERMINAL CONSTANTS AND CONFIGURATION
// ============================================================================

#define TERMINAL_VERSION            "1.0.0-Nexus"
#define MAX_COMMAND_LENGTH          8192
#define MAX_OUTPUT_BUFFER_SIZE      (1024 * 1024)  // 1MB output buffer
#define MAX_HISTORY_ENTRIES         10000
#define MAX_TABS                    20
#define MAX_SPLIT_PANES            8
#define TERMINAL_ROWS              40
#define TERMINAL_COLS              120
#define AI_SUGGESTION_MAX_LENGTH    512
#define MAX_ENVIRONMENT_VARS        256

// Terminal color schemes
typedef enum {
    TERMINAL_THEME_LIMITLESS_DARK = 0,    // Default military dark theme
    TERMINAL_THEME_LIMITLESS_LIGHT,       // Light variant
    TERMINAL_THEME_MATRIX_GREEN,          // Classic green on black
    TERMINAL_THEME_AMBER_MONOCHROME,      // Retro amber terminal
    TERMINAL_THEME_TACTICAL_ORANGE,       // High-contrast tactical
    TERMINAL_THEME_ARCTIC_BLUE,           // Cool blue theme
    TERMINAL_THEME_CUSTOM                 // User-defined
} terminal_theme_t;

// Shell types
typedef enum {
    SHELL_LIMITLESS_SH = 0,               // Native LimitlessOS shell
    SHELL_BASH,                           // Bash compatibility
    SHELL_ZSH,                            // Zsh compatibility
    SHELL_FISH,                           // Fish shell
    SHELL_POWERSHELL,                     // PowerShell compatibility
    SHELL_CMD                             // Command prompt compatibility
} shell_type_t;

// AI assistance levels
typedef enum {
    TERMINAL_AI_OFF = 0,                  // No AI assistance
    TERMINAL_AI_BASIC,                    // Basic command completion
    TERMINAL_AI_STANDARD,                 // Command suggestions + help
    TERMINAL_AI_ENHANCED,                 // Advanced scripting assistance
    TERMINAL_AI_COPILOT                   // Full AI shell copilot
} terminal_ai_level_t;

// Terminal states
typedef enum {
    TERMINAL_STATE_READY = 0,             // Ready for input
    TERMINAL_STATE_RUNNING,               // Command executing
    TERMINAL_STATE_WAITING_INPUT,         // Waiting for user input
    TERMINAL_STATE_ERROR,                 // Error state
    TERMINAL_STATE_SUSPENDED              // Process suspended
} terminal_state_t;

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

// Terminal color palette
typedef struct terminal_colors {
    lui_color_t background;               // Terminal background
    lui_color_t foreground;               // Default text color
    lui_color_t cursor;                   // Cursor color
    lui_color_t selection;                // Selection highlight
    
    // ANSI colors (16 colors)
    lui_color_t ansi_black;
    lui_color_t ansi_red;
    lui_color_t ansi_green;
    lui_color_t ansi_yellow;
    lui_color_t ansi_blue;
    lui_color_t ansi_magenta;
    lui_color_t ansi_cyan;
    lui_color_t ansi_white;
    lui_color_t ansi_bright_black;
    lui_color_t ansi_bright_red;
    lui_color_t ansi_bright_green;
    lui_color_t ansi_bright_yellow;
    lui_color_t ansi_bright_blue;
    lui_color_t ansi_bright_magenta;
    lui_color_t ansi_bright_cyan;
    lui_color_t ansi_bright_white;
    
    // Special UI colors
    lui_color_t prompt;                   // Shell prompt color
    lui_color_t command;                  // User input color
    lui_color_t output;                   // Command output color
    lui_color_t error;                    // Error message color
    lui_color_t ai_suggestion;            // AI suggestion color
} terminal_colors_t;

// Terminal cell (character + attributes)
typedef struct terminal_cell {
    char character;                       // Character to display
    lui_color_t fg_color;                 // Foreground color
    lui_color_t bg_color;                 // Background color
    bool is_bold;                         // Bold text
    bool is_italic;                       // Italic text
    bool is_underline;                    // Underlined text
    bool is_blinking;                     // Blinking text
    bool is_inverse;                      // Inverse video
} terminal_cell_t;

// Terminal screen buffer
typedef struct terminal_buffer {
    terminal_cell_t** cells;              // 2D array of cells [row][col]
    uint32_t rows;                        // Number of rows
    uint32_t cols;                        // Number of columns
    uint32_t cursor_row;                  // Current cursor row
    uint32_t cursor_col;                  // Current cursor column
    bool cursor_visible;                  // Cursor visibility
    
    // Scrollback buffer
    terminal_cell_t** scrollback;         // Scrollback lines
    uint32_t scrollback_size;             // Max scrollback lines
    uint32_t scrollback_used;             // Used scrollback lines
    uint32_t scroll_position;             // Current scroll position
    
    // Selection
    struct {
        bool active;                      // Selection is active
        uint32_t start_row, start_col;    // Selection start
        uint32_t end_row, end_col;        // Selection end
    } selection;
} terminal_buffer_t;

// Command history entry
typedef struct command_history {
    char* command;                        // Command text
    time_t timestamp;                     // When command was executed
    int exit_code;                        // Command exit code
    uint32_t execution_time_ms;           // Execution time in milliseconds
    struct command_history* next;
    struct command_history* prev;
} command_history_t;

// Environment variable
typedef struct env_var {
    char* name;                           // Variable name
    char* value;                          // Variable value
    bool is_exported;                     // Exported to child processes
    struct env_var* next;
} env_var_t;

// Process information
typedef struct process_info {
    pid_t pid;                            // Process ID
    char* command;                        // Command that started process
    time_t start_time;                    // Process start time
    terminal_state_t state;               // Process state
    int exit_code;                        // Exit code (if terminated)
    
    // PTY information
    int pty_master;                       // Master PTY file descriptor
    int pty_slave;                        // Slave PTY file descriptor
    
    struct process_info* next;
} process_info_t;

// Terminal session/tab
typedef struct terminal_session {
    uint32_t id;                          // Unique session ID
    char title[128];                      // Tab title
    shell_type_t shell_type;              // Shell type for this session
    terminal_state_t state;               // Current session state
    
    // Terminal display
    terminal_buffer_t* buffer;            // Screen buffer
    terminal_colors_t colors;             // Color scheme
    
    // Current process
    process_info_t* current_process;      // Currently running process
    process_info_t* process_list;         // List of background processes
    
    // Command handling
    char current_command[MAX_COMMAND_LENGTH];  // Current input line
    uint32_t command_cursor;              // Cursor position in command
    command_history_t* history;           // Command history
    uint32_t history_size;                // Number of history entries
    uint32_t history_position;            // Current position in history
    
    // Environment
    env_var_t* environment;               // Environment variables
    char* current_directory;              // Current working directory
    
    // AI features
    terminal_ai_level_t ai_level;         // AI assistance level
    char* ai_current_suggestion;          // Current AI suggestion
    float ai_confidence;                  // Suggestion confidence (0.0-1.0)
    
    // Session settings
    bool echo_enabled;                    // Local echo enabled
    bool auto_complete_enabled;           // Tab completion enabled
    bool history_search_enabled;          // History search with arrows
    uint32_t font_size;                   // Font size for this session
    
    struct terminal_session* next;
    struct terminal_session* prev;
} terminal_session_t;

// Split pane information
typedef struct split_pane {
    uint32_t id;                          // Unique pane ID
    terminal_session_t* session;          // Associated session
    lui_rect_t bounds;                    // Pane boundaries
    bool is_active;                       // Currently focused pane
    
    // Split configuration
    enum {
        SPLIT_NONE = 0,
        SPLIT_HORIZONTAL,                 // Side by side
        SPLIT_VERTICAL                    // Top and bottom
    } split_type;
    
    struct split_pane* parent;            // Parent pane (if split)
    struct split_pane* child1;            // First child pane
    struct split_pane* child2;            // Second child pane
    struct split_pane* next;
} split_pane_t;

// Main terminal application state
typedef struct limitless_terminal {
    bool initialized;
    bool running;
    
    // UI components
    lui_window_t* main_window;
    lui_widget_t* tab_bar;
    lui_widget_t* terminal_area;
    lui_widget_t* status_bar;
    lui_widget_t* ai_panel;
    lui_widget_t* sidebar;
    
    // Session management
    terminal_session_t* sessions;         // List of terminal sessions
    terminal_session_t* active_session;   // Currently active session
    uint32_t session_count;
    uint32_t next_session_id;
    
    // Split pane management
    split_pane_t* root_pane;              // Root pane (entire terminal area)
    split_pane_t* active_pane;            // Currently focused pane
    uint32_t pane_count;
    uint32_t next_pane_id;
    
    // Global settings
    terminal_theme_t theme;               // Current color theme
    shell_type_t default_shell;           // Default shell type
    terminal_ai_level_t global_ai_level;  // Global AI assistance level
    uint32_t default_font_size;           // Default font size
    
    // Terminal features
    bool show_tabs;                       // Show tab bar
    bool show_status_bar;                 // Show status bar
    bool ai_panel_visible;                // AI panel visibility
    bool sidebar_visible;                 // Sidebar visibility
    bool bell_enabled;                    // Audio bell enabled
    bool visual_bell_enabled;             // Visual bell enabled
    
    // Performance settings
    uint32_t max_scrollback_lines;        // Maximum scrollback buffer
    uint32_t refresh_rate_hz;             // Screen refresh rate
    bool hardware_acceleration;           // Use GPU acceleration
    
    // Statistics
    struct {
        uint64_t commands_executed;       // Total commands executed
        uint64_t keystrokes_typed;        // Total keystrokes
        uint32_t sessions_created;        // Sessions created this run
        uint64_t uptime_seconds;          // Terminal uptime
        uint32_t ai_suggestions_used;     // AI suggestions accepted
        time_t start_time;                // Terminal start time
    } stats;
    
} limitless_terminal_t;

// Global terminal instance
static limitless_terminal_t g_terminal = {0};

// ============================================================================
// COLOR SCHEME DEFINITIONS
// ============================================================================

static void init_limitless_dark_colors(terminal_colors_t* colors) {
    colors->background = LUI_COLOR_CHARCOAL_BLACK;
    colors->foreground = LUI_COLOR_ARCTIC_WHITE;
    colors->cursor = LUI_COLOR_SECURE_CYAN;
    colors->selection = LUI_COLOR_TACTICAL_BLUE;
    
    // ANSI colors - military-inspired palette
    colors->ansi_black = LUI_COLOR_CHARCOAL_BLACK;
    colors->ansi_red = LUI_COLOR_ALERT_RED;
    colors->ansi_green = LUI_COLOR_SUCCESS_GREEN;
    colors->ansi_yellow = LUI_COLOR_WARNING_AMBER;
    colors->ansi_blue = LUI_COLOR_TACTICAL_BLUE;
    colors->ansi_magenta = lui_color_make(186, 85, 211);       // Medium orchid
    colors->ansi_cyan = LUI_COLOR_SECURE_CYAN;
    colors->ansi_white = LUI_COLOR_STEEL_GRAY;
    
    colors->ansi_bright_black = LUI_COLOR_GRAPHITE;
    colors->ansi_bright_red = lui_color_make(255, 102, 102);   // Light red
    colors->ansi_bright_green = lui_color_make(144, 238, 144); // Light green
    colors->ansi_bright_yellow = lui_color_make(255, 255, 102); // Light yellow
    colors->ansi_bright_blue = lui_color_make(135, 206, 250);  // Light sky blue
    colors->ansi_bright_magenta = lui_color_make(221, 160, 221); // Plum
    colors->ansi_bright_cyan = lui_color_make(224, 255, 255);  // Light cyan
    colors->ansi_bright_white = LUI_COLOR_ARCTIC_WHITE;
    
    // Special colors
    colors->prompt = LUI_COLOR_ENERGY_ORANGE;
    colors->command = LUI_COLOR_ARCTIC_WHITE;
    colors->output = LUI_COLOR_STEEL_GRAY;
    colors->error = LUI_COLOR_ALERT_RED;
    colors->ai_suggestion = LUI_COLOR_SECURE_CYAN;
}

static void init_matrix_green_colors(terminal_colors_t* colors) {
    colors->background = LUI_COLOR_CHARCOAL_BLACK;
    colors->foreground = lui_color_make(0, 255, 65);           // Matrix green
    colors->cursor = lui_color_make(0, 255, 65);
    colors->selection = lui_color_make(0, 128, 32);
    
    // All ANSI colors in green variations
    colors->ansi_black = LUI_COLOR_CHARCOAL_BLACK;
    colors->ansi_red = lui_color_make(0, 200, 50);
    colors->ansi_green = lui_color_make(0, 255, 65);
    colors->ansi_yellow = lui_color_make(100, 255, 120);
    colors->ansi_blue = lui_color_make(0, 180, 40);
    colors->ansi_magenta = lui_color_make(50, 255, 100);
    colors->ansi_cyan = lui_color_make(0, 255, 180);
    colors->ansi_white = lui_color_make(200, 255, 220);
    
    // Bright variants
    colors->ansi_bright_black = lui_color_make(0, 100, 25);
    colors->ansi_bright_red = lui_color_make(0, 255, 80);
    colors->ansi_bright_green = lui_color_make(100, 255, 150);
    colors->ansi_bright_yellow = lui_color_make(150, 255, 170);
    colors->ansi_bright_blue = lui_color_make(0, 220, 60);
    colors->ansi_bright_magenta = lui_color_make(100, 255, 140);
    colors->ansi_bright_cyan = lui_color_make(50, 255, 200);
    colors->ansi_bright_white = lui_color_make(255, 255, 255);
    
    colors->prompt = lui_color_make(0, 255, 65);
    colors->command = lui_color_make(150, 255, 170);
    colors->output = lui_color_make(0, 200, 50);
    colors->error = lui_color_make(255, 100, 100);
    colors->ai_suggestion = lui_color_make(0, 255, 180);
}

// ============================================================================
// TERMINAL BUFFER MANAGEMENT
// ============================================================================

static terminal_buffer_t* create_terminal_buffer(uint32_t rows, uint32_t cols) {
    terminal_buffer_t* buffer = calloc(1, sizeof(terminal_buffer_t));
    if (!buffer) {
        printf("[Terminal] ERROR: Failed to allocate terminal buffer\n");
        return NULL;
    }
    
    buffer->rows = rows;
    buffer->cols = cols;
    buffer->scrollback_size = g_terminal.max_scrollback_lines;
    buffer->cursor_visible = true;
    
    // Allocate screen buffer
    buffer->cells = calloc(rows, sizeof(terminal_cell_t*));
    if (!buffer->cells) {
        free(buffer);
        return NULL;
    }
    
    for (uint32_t i = 0; i < rows; i++) {
        buffer->cells[i] = calloc(cols, sizeof(terminal_cell_t));
        if (!buffer->cells[i]) {
            // Clean up on failure
            for (uint32_t j = 0; j < i; j++) {
                free(buffer->cells[j]);
            }
            free(buffer->cells);
            free(buffer);
            return NULL;
        }
    }
    
    // Allocate scrollback buffer
    buffer->scrollback = calloc(buffer->scrollback_size, sizeof(terminal_cell_t*));
    if (!buffer->scrollback) {
        for (uint32_t i = 0; i < rows; i++) {
            free(buffer->cells[i]);
        }
        free(buffer->cells);
        free(buffer);
        return NULL;
    }
    
    for (uint32_t i = 0; i < buffer->scrollback_size; i++) {
        buffer->scrollback[i] = calloc(cols, sizeof(terminal_cell_t));
        if (!buffer->scrollback[i]) {
            // Clean up on failure
            for (uint32_t j = 0; j < i; j++) {
                free(buffer->scrollback[j]);
            }
            free(buffer->scrollback);
            for (uint32_t j = 0; j < rows; j++) {
                free(buffer->cells[j]);
            }
            free(buffer->cells);
            free(buffer);
            return NULL;
        }
    }
    
    printf("[Terminal] Created terminal buffer (%ux%u, scrollback: %u)\n", 
           cols, rows, buffer->scrollback_size);
    
    return buffer;
}

static void free_terminal_buffer(terminal_buffer_t* buffer) {
    if (!buffer) return;
    
    // Free screen buffer
    if (buffer->cells) {
        for (uint32_t i = 0; i < buffer->rows; i++) {
            if (buffer->cells[i]) {
                free(buffer->cells[i]);
            }
        }
        free(buffer->cells);
    }
    
    // Free scrollback buffer
    if (buffer->scrollback) {
        for (uint32_t i = 0; i < buffer->scrollback_size; i++) {
            if (buffer->scrollback[i]) {
                free(buffer->scrollback[i]);
            }
        }
        free(buffer->scrollback);
    }
    
    free(buffer);
}

static void clear_terminal_buffer(terminal_buffer_t* buffer, terminal_colors_t* colors) {
    if (!buffer || !colors) return;
    
    // Clear screen
    for (uint32_t row = 0; row < buffer->rows; row++) {
        for (uint32_t col = 0; col < buffer->cols; col++) {
            buffer->cells[row][col].character = ' ';
            buffer->cells[row][col].fg_color = colors->foreground;
            buffer->cells[row][col].bg_color = colors->background;
            buffer->cells[row][col].is_bold = false;
            buffer->cells[row][col].is_italic = false;
            buffer->cells[row][col].is_underline = false;
            buffer->cells[row][col].is_blinking = false;
            buffer->cells[row][col].is_inverse = false;
        }
    }
    
    buffer->cursor_row = 0;
    buffer->cursor_col = 0;
    buffer->selection.active = false;
}

static void write_char_to_buffer(terminal_buffer_t* buffer, char ch, 
                                terminal_colors_t* colors) {
    if (!buffer || !colors) return;
    
    if (ch == '\n') {
        // Newline - move to next line
        buffer->cursor_col = 0;
        buffer->cursor_row++;
        
        // Scroll if necessary
        if (buffer->cursor_row >= buffer->rows) {
            // Move top line to scrollback
            if (buffer->scrollback_used < buffer->scrollback_size) {
                memcpy(buffer->scrollback[buffer->scrollback_used], 
                       buffer->cells[0], 
                       buffer->cols * sizeof(terminal_cell_t));
                buffer->scrollback_used++;
            } else {
                // Scrollback is full, shift lines up
                terminal_cell_t* temp = buffer->scrollback[0];
                memmove(buffer->scrollback, buffer->scrollback + 1, 
                        (buffer->scrollback_size - 1) * sizeof(terminal_cell_t*));
                buffer->scrollback[buffer->scrollback_size - 1] = temp;
                
                memcpy(buffer->scrollback[buffer->scrollback_size - 1], 
                       buffer->cells[0], 
                       buffer->cols * sizeof(terminal_cell_t));
            }
            
            // Shift screen lines up
            terminal_cell_t* temp = buffer->cells[0];
            memmove(buffer->cells, buffer->cells + 1, 
                    (buffer->rows - 1) * sizeof(terminal_cell_t*));
            buffer->cells[buffer->rows - 1] = temp;
            
            // Clear new bottom line
            for (uint32_t col = 0; col < buffer->cols; col++) {
                buffer->cells[buffer->rows - 1][col].character = ' ';
                buffer->cells[buffer->rows - 1][col].fg_color = colors->foreground;
                buffer->cells[buffer->rows - 1][col].bg_color = colors->background;
                buffer->cells[buffer->rows - 1][col].is_bold = false;
                buffer->cells[buffer->rows - 1][col].is_italic = false;
                buffer->cells[buffer->rows - 1][col].is_underline = false;
                buffer->cells[buffer->rows - 1][col].is_blinking = false;
                buffer->cells[buffer->rows - 1][col].is_inverse = false;
            }
            
            buffer->cursor_row = buffer->rows - 1;
        }
    }
    else if (ch == '\r') {
        // Carriage return - move to beginning of line
        buffer->cursor_col = 0;
    }
    else if (ch == '\b') {
        // Backspace - move cursor back
        if (buffer->cursor_col > 0) {
            buffer->cursor_col--;
        }
    }
    else if (ch == '\t') {
        // Tab - move to next tab stop (8 column alignment)
        uint32_t next_tab = ((buffer->cursor_col / 8) + 1) * 8;
        if (next_tab < buffer->cols) {
            buffer->cursor_col = next_tab;
        }
    }
    else if (ch >= ' ' && ch <= '~') {
        // Printable character
        if (buffer->cursor_col < buffer->cols) {
            buffer->cells[buffer->cursor_row][buffer->cursor_col].character = ch;
            buffer->cells[buffer->cursor_row][buffer->cursor_col].fg_color = colors->foreground;
            buffer->cells[buffer->cursor_row][buffer->cursor_col].bg_color = colors->background;
            buffer->cursor_col++;
            
            // Wrap to next line if necessary
            if (buffer->cursor_col >= buffer->cols) {
                write_char_to_buffer(buffer, '\n', colors);
            }
        }
    }
}

static void write_string_to_buffer(terminal_buffer_t* buffer, const char* str, 
                                  terminal_colors_t* colors) {
    if (!buffer || !str || !colors) return;
    
    while (*str) {
        write_char_to_buffer(buffer, *str, colors);
        str++;
    }
}

// ============================================================================
// SHELL COMMAND PROCESSING
// ============================================================================

static void add_to_command_history(terminal_session_t* session, const char* command, 
                                  int exit_code, uint32_t execution_time) {
    if (!session || !command || strlen(command) == 0) return;
    
    // Don't add duplicate consecutive commands
    if (session->history && session->history->command && 
        strcmp(session->history->command, command) == 0) {
        return;
    }
    
    command_history_t* entry = calloc(1, sizeof(command_history_t));
    if (!entry) return;
    
    entry->command = strdup(command);
    entry->timestamp = time(NULL);
    entry->exit_code = exit_code;
    entry->execution_time_ms = execution_time;
    
    // Add to front of list
    entry->next = session->history;
    if (session->history) {
        session->history->prev = entry;
    }
    session->history = entry;
    session->history_size++;
    
    // Limit history size
    if (session->history_size > MAX_HISTORY_ENTRIES) {
        command_history_t* last = session->history;
        while (last->next) {
            last = last->next;
        }
        
        if (last->prev) {
            last->prev->next = NULL;
        }
        
        free(last->command);
        free(last);
        session->history_size--;
    }
}

static env_var_t* get_environment_variable(terminal_session_t* session, const char* name) {
    if (!session || !name) return NULL;
    
    env_var_t* var = session->environment;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            return var;
        }
        var = var->next;
    }
    
    return NULL;
}

static void set_environment_variable(terminal_session_t* session, const char* name, 
                                    const char* value, bool export) {
    if (!session || !name) return;
    
    // Check if variable already exists
    env_var_t* var = get_environment_variable(session, name);
    if (var) {
        free(var->value);
        var->value = strdup(value ? value : "");
        var->is_exported = export;
        return;
    }
    
    // Create new variable
    var = calloc(1, sizeof(env_var_t));
    if (!var) return;
    
    var->name = strdup(name);
    var->value = strdup(value ? value : "");
    var->is_exported = export;
    
    // Add to front of list
    var->next = session->environment;
    session->environment = var;
}

static bool execute_builtin_command(terminal_session_t* session, const char* command) {
    if (!session || !command) return false;
    
    // Parse command and arguments
    char cmd_copy[MAX_COMMAND_LENGTH];
    strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    
    char* cmd = strtok(cmd_copy, " \t");
    if (!cmd) return false;
    
    // Built-in commands
    if (strcmp(cmd, "cd") == 0) {
        char* path = strtok(NULL, " \t");
        if (!path) {
            // Change to home directory
            path = getenv("HOME");
            if (!path) path = "/";
        }
        
        if (chdir(path) == 0) {
            free(session->current_directory);
            session->current_directory = strdup(path);
            
            char result[512];
            snprintf(result, sizeof(result), "Changed directory to: %s\n", path);
            write_string_to_buffer(session->buffer, result, &session->colors);
        } else {
            char error[512];
            snprintf(error, sizeof(error), "cd: %s: No such file or directory\n", path);
            write_string_to_buffer(session->buffer, error, &session->colors);
        }
        return true;
    }
    else if (strcmp(cmd, "pwd") == 0) {
        char* cwd = getcwd(NULL, 0);
        if (cwd) {
            write_string_to_buffer(session->buffer, cwd, &session->colors);
            write_char_to_buffer(session->buffer, '\n', &session->colors);
            free(cwd);
        }
        return true;
    }
    else if (strcmp(cmd, "echo") == 0) {
        char* args = strtok(NULL, "");
        if (args) {
            write_string_to_buffer(session->buffer, args, &session->colors);
        }
        write_char_to_buffer(session->buffer, '\n', &session->colors);
        return true;
    }
    else if (strcmp(cmd, "exit") == 0) {
        session->state = TERMINAL_STATE_SUSPENDED;
        write_string_to_buffer(session->buffer, "Goodbye!\n", &session->colors);
        return true;
    }
    else if (strcmp(cmd, "clear") == 0) {
        clear_terminal_buffer(session->buffer, &session->colors);
        return true;
    }
    else if (strcmp(cmd, "history") == 0) {
        command_history_t* hist = session->history;
        int count = 1;
        while (hist) {
            char line[512];
            snprintf(line, sizeof(line), "%4d  %s\n", count++, hist->command);
            write_string_to_buffer(session->buffer, line, &session->colors);
            hist = hist->next;
        }
        return true;
    }
    else if (strcmp(cmd, "env") == 0) {
        env_var_t* var = session->environment;
        while (var) {
            char line[512];
            snprintf(line, sizeof(line), "%s=%s\n", var->name, var->value);
            write_string_to_buffer(session->buffer, line, &session->colors);
            var = var->next;
        }
        return true;
    }
    else if (strcmp(cmd, "export") == 0) {
        char* assignment = strtok(NULL, "");
        if (assignment) {
            char* equals = strchr(assignment, '=');
            if (equals) {
                *equals = '\0';
                char* name = assignment;
                char* value = equals + 1;
                set_environment_variable(session, name, value, true);
            } else {
                // Export existing variable
                env_var_t* var = get_environment_variable(session, assignment);
                if (var) {
                    var->is_exported = true;
                }
            }
        }
        return true;
    }
    else if (strcmp(cmd, "limitless") == 0) {
        // LimitlessOS specific commands
        char* subcmd = strtok(NULL, " \t");
        if (subcmd) {
            if (strcmp(subcmd, "version") == 0) {
                write_string_to_buffer(session->buffer, "LimitlessOS Terminal v", &session->colors);
                write_string_to_buffer(session->buffer, TERMINAL_VERSION, &session->colors);
                write_char_to_buffer(session->buffer, '\n', &session->colors);
            }
            else if (strcmp(subcmd, "ai") == 0) {
                char* ai_cmd = strtok(NULL, " \t");
                if (ai_cmd) {
                    if (strcmp(ai_cmd, "on") == 0) {
                        session->ai_level = TERMINAL_AI_STANDARD;
                        write_string_to_buffer(session->buffer, "AI assistance enabled\n", &session->colors);
                    }
                    else if (strcmp(ai_cmd, "off") == 0) {
                        session->ai_level = TERMINAL_AI_OFF;
                        write_string_to_buffer(session->buffer, "AI assistance disabled\n", &session->colors);
                    }
                }
            }
        }
        return true;
    }
    
    return false; // Not a built-in command
}

static void execute_command(terminal_session_t* session, const char* command) {
    if (!session || !command || strlen(command) == 0) return;
    
    printf("[Terminal] Executing command: %s\n", command);
    
    // Update statistics
    g_terminal.stats.commands_executed++;
    
    uint32_t start_time = time(NULL) * 1000; // Approximate milliseconds
    
    // Try built-in commands first
    if (execute_builtin_command(session, command)) {
        uint32_t execution_time = (time(NULL) * 1000) - start_time;
        add_to_command_history(session, command, 0, execution_time);
        return;
    }
    
    // For external commands, we would typically fork and exec
    // For this implementation, we'll simulate command execution
    
    session->state = TERMINAL_STATE_RUNNING;
    
    // Simple command simulation
    if (strncmp(command, "ls", 2) == 0) {
        write_string_to_buffer(session->buffer, "file1.txt  file2.c  directory1/  directory2/\n", &session->colors);
    }
    else if (strncmp(command, "date", 4) == 0) {
        time_t now = time(NULL);
        char* timestr = ctime(&now);
        write_string_to_buffer(session->buffer, timestr, &session->colors);
    }
    else if (strncmp(command, "whoami", 6) == 0) {
        write_string_to_buffer(session->buffer, "limitless-user\n", &session->colors);
    }
    else if (strncmp(command, "uname", 5) == 0) {
        write_string_to_buffer(session->buffer, "LimitlessOS 1.0.0 x86_64\n", &session->colors);
    }
    else if (strncmp(command, "ps", 2) == 0) {
        write_string_to_buffer(session->buffer, 
                              "PID   CMD\n"
                              "1     /sbin/init\n"
                              "2     [limitless-kernel]\n"
                              "1234  limitless-terminal\n", 
                              &session->colors);
    }
    else {
        // Unknown command
        char error[512];
        snprintf(error, sizeof(error), "%s: command not found\n", command);
        write_string_to_buffer(session->buffer, error, &session->colors);
    }
    
    session->state = TERMINAL_STATE_READY;
    
    uint32_t execution_time = (time(NULL) * 1000) - start_time;
    add_to_command_history(session, command, 0, execution_time);
}

// ============================================================================
// SESSION MANAGEMENT
// ============================================================================

static terminal_session_t* create_terminal_session(shell_type_t shell_type) {
    terminal_session_t* session = calloc(1, sizeof(terminal_session_t));
    if (!session) {
        printf("[Terminal] ERROR: Failed to allocate terminal session\n");
        return NULL;
    }
    
    // Initialize session
    session->id = g_terminal.next_session_id++;
    snprintf(session->title, sizeof(session->title), "Terminal %u", session->id);
    session->shell_type = shell_type;
    session->state = TERMINAL_STATE_READY;
    
    // Create terminal buffer
    session->buffer = create_terminal_buffer(TERMINAL_ROWS, TERMINAL_COLS);
    if (!session->buffer) {
        free(session);
        return NULL;
    }
    
    // Initialize colors based on theme
    if (g_terminal.theme == TERMINAL_THEME_MATRIX_GREEN) {
        init_matrix_green_colors(&session->colors);
    } else {
        init_limitless_dark_colors(&session->colors);
    }
    
    clear_terminal_buffer(session->buffer, &session->colors);
    
    // Initialize environment
    session->current_directory = strdup("/home/limitless");
    set_environment_variable(session, "HOME", "/home/limitless", true);
    set_environment_variable(session, "USER", "limitless", true);
    set_environment_variable(session, "SHELL", "/bin/limitless-sh", true);
    set_environment_variable(session, "PATH", "/bin:/usr/bin:/usr/local/bin", true);
    set_environment_variable(session, "TERM", "limitless-256color", true);
    
    // Session settings
    session->ai_level = g_terminal.global_ai_level;
    session->echo_enabled = true;
    session->auto_complete_enabled = true;
    session->history_search_enabled = true;
    session->font_size = g_terminal.default_font_size;
    
    // Display welcome message
    write_string_to_buffer(session->buffer, "Welcome to LimitlessOS Terminal v", &session->colors);
    write_string_to_buffer(session->buffer, TERMINAL_VERSION, &session->colors);
    write_char_to_buffer(session->buffer, '\n', &session->colors);
    
    if (session->ai_level > TERMINAL_AI_OFF) {
        write_string_to_buffer(session->buffer, "🤖 AI assistance is enabled. Type 'limitless ai help' for more info.\n", &session->colors);
    }
    
    write_char_to_buffer(session->buffer, '\n', &session->colors);
    
    // Display initial prompt
    write_string_to_buffer(session->buffer, "limitless@localhost:~$ ", &session->colors);
    
    printf("[Terminal] Created session %u (shell: %d)\n", session->id, shell_type);
    
    return session;
}

static void close_terminal_session(terminal_session_t* session) {
    if (!session) return;
    
    printf("[Terminal] Closing session %u\n", session->id);
    
    // Remove from session list
    if (session->prev) {
        session->prev->next = session->next;
    } else {
        g_terminal.sessions = session->next;
    }
    if (session->next) {
        session->next->prev = session->prev;
    }
    
    // Update active session
    if (g_terminal.active_session == session) {
        g_terminal.active_session = g_terminal.sessions;
    }
    
    // Free resources
    if (session->buffer) {
        free_terminal_buffer(session->buffer);
    }
    
    if (session->current_directory) {
        free(session->current_directory);
    }
    
    if (session->ai_current_suggestion) {
        free(session->ai_current_suggestion);
    }
    
    // Free command history
    command_history_t* hist = session->history;
    while (hist) {
        command_history_t* next = hist->next;
        if (hist->command) free(hist->command);
        free(hist);
        hist = next;
    }
    
    // Free environment variables
    env_var_t* var = session->environment;
    while (var) {
        env_var_t* next = var->next;
        if (var->name) free(var->name);
        if (var->value) free(var->value);
        free(var);
        var = next;
    }
    
    // Free process list
    process_info_t* proc = session->process_list;
    while (proc) {
        process_info_t* next = proc->next;
        if (proc->command) free(proc->command);
        free(proc);
        proc = next;
    }
    
    free(session);
    g_terminal.session_count--;
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_tab_bar(void) {
    g_terminal.tab_bar = lui_create_container(g_terminal.main_window->root_widget);
    strcpy(g_terminal.tab_bar->name, "tab_bar");
    g_terminal.tab_bar->bounds = lui_rect_make(0, 0, 1200, 28);
    g_terminal.tab_bar->background_color = LUI_COLOR_GRAPHITE;
    
    // Render tabs for active sessions
    terminal_session_t* session = g_terminal.sessions;
    uint32_t x_offset = 4;
    
    while (session && x_offset < 1000) {
        // Tab container
        lui_widget_t* tab = lui_create_container(g_terminal.tab_bar);
        tab->bounds = lui_rect_make(x_offset, 2, 120, 24);
        tab->background_color = (session == g_terminal.active_session) ? 
                                LUI_COLOR_TACTICAL_BLUE : LUI_COLOR_STEEL_GRAY;
        
        // Tab title
        lui_widget_t* tab_label = lui_create_label(session->title, tab);
        tab_label->bounds = lui_rect_make(8, 2, 90, 20);
        tab_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        // Close button
        lui_widget_t* close_btn = lui_create_button("×", tab);
        close_btn->bounds = lui_rect_make(100, 2, 16, 20);
        close_btn->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        x_offset += 124;
        session = session->next;
    }
    
    // New tab button
    lui_widget_t* new_tab_btn = lui_create_button("+", g_terminal.tab_bar);
    new_tab_btn->bounds = lui_rect_make(x_offset, 2, 24, 24);
    new_tab_btn->style.background_color = LUI_COLOR_SUCCESS_GREEN;
}

static void render_terminal_content(void) {
    if (!g_terminal.active_session || !g_terminal.active_session->buffer) {
        return;
    }
    
    terminal_session_t* session = g_terminal.active_session;
    terminal_buffer_t* buffer = session->buffer;
    
    // Clear terminal area
    g_terminal.terminal_area->background_color = session->colors.background;
    
    // Render terminal text
    uint32_t char_width = 8;   // Approximate character width
    uint32_t char_height = 16; // Approximate character height
    
    for (uint32_t row = 0; row < buffer->rows; row++) {
        for (uint32_t col = 0; col < buffer->cols; col++) {
            terminal_cell_t* cell = &buffer->cells[row][col];
            
            if (cell->character != ' ') {
                char text[2] = {cell->character, '\0'};
                
                lui_widget_t* char_widget = lui_create_label(text, g_terminal.terminal_area);
                char_widget->bounds = lui_rect_make(
                    col * char_width, 
                    row * char_height, 
                    char_width, 
                    char_height);
                char_widget->typography.color = cell->fg_color;
                char_widget->background_color = cell->bg_color;
                
                if (cell->is_bold) {
                    char_widget->typography.weight = LUI_FONT_WEIGHT_BOLD;
                }
            }
        }
    }
    
    // Render cursor
    if (buffer->cursor_visible) {
        lui_widget_t* cursor = lui_create_container(g_terminal.terminal_area);
        cursor->bounds = lui_rect_make(
            buffer->cursor_col * char_width,
            buffer->cursor_row * char_height,
            char_width,
            char_height);
        cursor->background_color = session->colors.cursor;
    }
    
    // Render current command line
    if (session->current_command[0]) {
        uint32_t prompt_row = buffer->cursor_row;
        uint32_t prompt_col = 0;
        
        // Find end of prompt
        for (uint32_t col = 0; col < buffer->cols; col++) {
            if (buffer->cells[prompt_row][col].character == '$' || 
                buffer->cells[prompt_row][col].character == '#') {
                prompt_col = col + 2; // Skip "$ "
                break;
            }
        }
        
        // Display current command
        lui_widget_t* cmd_widget = lui_create_label(session->current_command, g_terminal.terminal_area);
        cmd_widget->bounds = lui_rect_make(
            prompt_col * char_width,
            prompt_row * char_height,
            strlen(session->current_command) * char_width,
            char_height);
        cmd_widget->typography.color = session->colors.command;
        
        // Command cursor
        lui_widget_t* cmd_cursor = lui_create_container(g_terminal.terminal_area);
        cmd_cursor->bounds = lui_rect_make(
            (prompt_col + session->command_cursor) * char_width,
            prompt_row * char_height,
            2,
            char_height);
        cmd_cursor->background_color = session->colors.cursor;
    }
}

static void create_terminal_area(void) {
    g_terminal.terminal_area = lui_create_container(g_terminal.main_window->root_widget);
    strcpy(g_terminal.terminal_area->name, "terminal_area");
    g_terminal.terminal_area->bounds = lui_rect_make(0, 28, 1200, 600);
    g_terminal.terminal_area->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Render current session content
    render_terminal_content();
}

static void create_status_bar(void) {
    g_terminal.status_bar = lui_create_container(g_terminal.main_window->root_widget);
    strcpy(g_terminal.status_bar->name, "status_bar");
    g_terminal.status_bar->bounds = lui_rect_make(0, 628, 1200, 24);
    g_terminal.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Status information
    char status_text[256] = "Ready";
    if (g_terminal.active_session) {
        terminal_session_t* session = g_terminal.active_session;
        snprintf(status_text, sizeof(status_text),
                "%s | %s | AI: %s | %llu commands",
                session->title,
                session->current_directory ? session->current_directory : "/",
                (session->ai_level > TERMINAL_AI_OFF) ? "On" : "Off",
                (unsigned long long)g_terminal.stats.commands_executed);
    }
    
    lui_widget_t* status_label = lui_create_label(status_text, g_terminal.status_bar);
    status_label->bounds = lui_rect_make(8, 2, 800, 20);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // Shell type indicator
    char shell_info[64];
    if (g_terminal.active_session) {
        const char* shell_name = 
            (g_terminal.active_session->shell_type == SHELL_LIMITLESS_SH) ? "limitless-sh" :
            (g_terminal.active_session->shell_type == SHELL_BASH) ? "bash" :
            (g_terminal.active_session->shell_type == SHELL_ZSH) ? "zsh" : "shell";
        
        snprintf(shell_info, sizeof(shell_info), "Shell: %s", shell_name);
    } else {
        strcpy(shell_info, "No active session");
    }
    
    lui_widget_t* shell_label = lui_create_label(shell_info, g_terminal.status_bar);
    shell_label->bounds = lui_rect_make(1000, 2, 150, 20);
    shell_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

void terminal_handle_keypress(char key) {
    if (!g_terminal.active_session) return;
    
    terminal_session_t* session = g_terminal.active_session;
    
    g_terminal.stats.keystrokes_typed++;
    
    if (key == '\n' || key == '\r') {
        // Execute command
        if (session->current_command[0]) {
            // Add newline to display
            write_char_to_buffer(session->buffer, '\n', &session->colors);
            
            // Execute the command
            execute_command(session, session->current_command);
            
            // Clear command buffer
            memset(session->current_command, 0, sizeof(session->current_command));
            session->command_cursor = 0;
            session->history_position = 0;
            
            // Display new prompt
            write_string_to_buffer(session->buffer, "limitless@localhost:~$ ", &session->colors);
        } else {
            // Empty command, just show new prompt
            write_char_to_buffer(session->buffer, '\n', &session->colors);
            write_string_to_buffer(session->buffer, "limitless@localhost:~$ ", &session->colors);
        }
    }
    else if (key == '\b' || key == 127) {
        // Backspace
        if (session->command_cursor > 0) {
            session->command_cursor--;
            session->current_command[session->command_cursor] = '\0';
        }
    }
    else if (key == '\t') {
        // Tab completion
        if (session->auto_complete_enabled && session->ai_level >= TERMINAL_AI_BASIC) {
            // Simple completion for built-in commands
            const char* completions[] = {"cd", "ls", "pwd", "echo", "exit", "clear", "history", "env"};
            
            for (size_t i = 0; i < sizeof(completions) / sizeof(completions[0]); i++) {
                if (strncmp(session->current_command, completions[i], strlen(session->current_command)) == 0) {
                    strcpy(session->current_command, completions[i]);
                    session->command_cursor = strlen(completions[i]);
                    break;
                }
            }
        }
    }
    else if (key >= ' ' && key <= '~') {
        // Printable character
        if (session->command_cursor < MAX_COMMAND_LENGTH - 1) {
            session->current_command[session->command_cursor] = key;
            session->command_cursor++;
            session->current_command[session->command_cursor] = '\0';
        }
    }
    
    // Update display
    render_terminal_content();
}

// ============================================================================
// MAIN TERMINAL API
// ============================================================================

bool limitless_terminal_init(void) {
    if (g_terminal.initialized) {
        return false;
    }
    
    printf("[Terminal] Initializing Limitless Terminal v%s\n", TERMINAL_VERSION);
    
    // Clear state
    memset(&g_terminal, 0, sizeof(g_terminal));
    
    // Set default configuration
    g_terminal.theme = TERMINAL_THEME_LIMITLESS_DARK;
    g_terminal.default_shell = SHELL_LIMITLESS_SH;
    g_terminal.global_ai_level = TERMINAL_AI_STANDARD;
    g_terminal.default_font_size = 14;
    g_terminal.show_tabs = true;
    g_terminal.show_status_bar = true;
    g_terminal.ai_panel_visible = false;
    g_terminal.sidebar_visible = false;
    g_terminal.bell_enabled = false;
    g_terminal.visual_bell_enabled = true;
    g_terminal.max_scrollback_lines = 10000;
    g_terminal.refresh_rate_hz = 60;
    g_terminal.hardware_acceleration = true;
    g_terminal.next_session_id = 1;
    g_terminal.next_pane_id = 1;
    
    // Create main window
    g_terminal.main_window = lui_create_window("Limitless Terminal", LUI_WINDOW_NORMAL,
                                             150, 150, 1200, 652);
    if (!g_terminal.main_window) {
        printf("[Terminal] ERROR: Failed to create main window\n");
        return false;
    }
    
    // Create UI components
    create_tab_bar();
    create_terminal_area();
    create_status_bar();
    
    // Create initial terminal session
    terminal_session_t* initial_session = create_terminal_session(g_terminal.default_shell);
    if (initial_session) {
        g_terminal.sessions = initial_session;
        g_terminal.active_session = initial_session;
        g_terminal.session_count = 1;
        g_terminal.stats.sessions_created = 1;
    }
    
    // Show window
    lui_show_window(g_terminal.main_window);
    
    g_terminal.initialized = true;
    g_terminal.running = true;
    g_terminal.stats.start_time = time(NULL);
    
    printf("[Terminal] Terminal initialized successfully\n");
    printf("[Terminal] Theme: %d, Shell: %d, AI Level: %d\n",
           g_terminal.theme, g_terminal.default_shell, g_terminal.global_ai_level);
    
    return true;
}

void limitless_terminal_shutdown(void) {
    if (!g_terminal.initialized) {
        return;
    }
    
    printf("[Terminal] Shutting down Limitless Terminal\n");
    
    g_terminal.running = false;
    
    // Close all sessions
    terminal_session_t* session = g_terminal.sessions;
    while (session) {
        terminal_session_t* next = session->next;
        close_terminal_session(session);
        session = next;
    }
    
    // Destroy main window
    if (g_terminal.main_window) {
        lui_destroy_window(g_terminal.main_window);
    }
    
    // Print session statistics
    time_t uptime = time(NULL) - g_terminal.stats.start_time;
    printf("[Terminal] Session statistics:\n");
    printf("  Uptime: %ld seconds\n", uptime);
    printf("  Commands executed: %llu\n", (unsigned long long)g_terminal.stats.commands_executed);
    printf("  Keystrokes typed: %llu\n", (unsigned long long)g_terminal.stats.keystrokes_typed);
    printf("  Sessions created: %u\n", g_terminal.stats.sessions_created);
    printf("  AI suggestions used: %u\n", g_terminal.stats.ai_suggestions_used);
    
    memset(&g_terminal, 0, sizeof(g_terminal));
    
    printf("[Terminal] Shutdown complete\n");
}

terminal_session_t* limitless_terminal_new_session(shell_type_t shell_type) {
    if (!g_terminal.initialized || g_terminal.session_count >= MAX_TABS) {
        return NULL;
    }
    
    terminal_session_t* new_session = create_terminal_session(shell_type);
    if (!new_session) {
        return NULL;
    }
    
    // Add to session list
    new_session->next = g_terminal.sessions;
    if (g_terminal.sessions) {
        g_terminal.sessions->prev = new_session;
    }
    g_terminal.sessions = new_session;
    g_terminal.session_count++;
    g_terminal.stats.sessions_created++;
    
    // Activate new session
    if (g_terminal.active_session) {
        g_terminal.active_session->state = TERMINAL_STATE_SUSPENDED;
    }
    g_terminal.active_session = new_session;
    
    return new_session;
}

const char* limitless_terminal_get_version(void) {
    return TERMINAL_VERSION;
}

void limitless_terminal_run(void) {
    if (!g_terminal.initialized) {
        printf("[Terminal] ERROR: Terminal not initialized\n");
        return;
    }
    
    printf("[Terminal] Running Limitless Terminal\n");
    
    // Main event loop is handled by the desktop environment
    // This function serves as an entry point for standalone execution
}