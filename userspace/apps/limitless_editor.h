/**
 * Limitless Editor - Header File
 * 
 * Advanced text and code editor with AI integration for LimitlessOS
 */

#ifndef LIMITLESS_EDITOR_H
#define LIMITLESS_EDITOR_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// PUBLIC API TYPES
// ============================================================================

typedef enum {
    EDITOR_LANGUAGE_PLAIN_TEXT = 0,
    EDITOR_LANGUAGE_C,
    EDITOR_LANGUAGE_CPP,
    EDITOR_LANGUAGE_PYTHON,
    EDITOR_LANGUAGE_JAVASCRIPT,
    EDITOR_LANGUAGE_TYPESCRIPT,
    EDITOR_LANGUAGE_RUST,
    EDITOR_LANGUAGE_GO,
    EDITOR_LANGUAGE_JAVA,
    EDITOR_LANGUAGE_CSHARP,
    EDITOR_LANGUAGE_HTML,
    EDITOR_LANGUAGE_CSS,
    EDITOR_LANGUAGE_JSON,
    EDITOR_LANGUAGE_XML,
    EDITOR_LANGUAGE_YAML,
    EDITOR_LANGUAGE_MARKDOWN,
    EDITOR_LANGUAGE_SHELL,
    EDITOR_LANGUAGE_SQL,
    EDITOR_LANGUAGE_ASSEMBLY
} editor_language_t;

typedef enum {
    EDITOR_THEME_DARK = 0,
    EDITOR_THEME_LIGHT,
    EDITOR_THEME_TERMINAL,
    EDITOR_THEME_ARCTIC,
    EDITOR_THEME_TACTICAL,
    EDITOR_THEME_CUSTOM
} editor_theme_t;

typedef enum {
    EDITOR_AI_OFF = 0,
    EDITOR_AI_BASIC,
    EDITOR_AI_STANDARD,
    EDITOR_AI_ENHANCED,
    EDITOR_AI_COPILOT
} editor_ai_level_t;

typedef struct editor_position {
    uint32_t line;
    uint32_t column;
} editor_position_t;

typedef struct editor_selection {
    editor_position_t start;
    editor_position_t end;
    bool is_active;
    bool is_rectangular;
} editor_selection_t;

typedef struct editor_config {
    editor_theme_t theme;
    uint32_t font_size;
    bool show_line_numbers;
    bool show_whitespace;
    bool word_wrap;
    bool minimap_enabled;
    bool ai_panel_visible;
    editor_ai_level_t ai_level;
    uint32_t tab_size;
    bool use_spaces_for_tabs;
    bool auto_indent;
    bool auto_complete_brackets;
} editor_config_t;

// ============================================================================
// CORE EDITOR API
// ============================================================================

/**
 * Initialize the Limitless Editor system
 * Returns: true if successful, false otherwise
 */
bool limitless_editor_init(void);

/**
 * Shutdown the editor and cleanup resources
 */
void limitless_editor_shutdown(void);

/**
 * Run the editor main loop
 */
void limitless_editor_run(void);

/**
 * Get editor version string
 * Returns: Version string (static)
 */
const char* limitless_editor_get_version(void);

// ============================================================================
// FILE MANAGEMENT API
// ============================================================================

/**
 * Open a file for editing
 * file_path: Path to file to open
 * Returns: true if file opened successfully
 */
bool limitless_editor_open_file(const char* file_path);

/**
 * Create a new file buffer
 * Returns: true if new file created
 */
bool limitless_editor_new_file(void);

/**
 * Save the current file
 * Returns: true if saved successfully
 */
bool limitless_editor_save_file(void);

/**
 * Save the current file with new name
 * file_path: New file path
 * Returns: true if saved successfully
 */
bool limitless_editor_save_file_as(const char* file_path);

/**
 * Close the current file
 * Returns: true if closed (false if user cancels due to unsaved changes)
 */
bool limitless_editor_close_file(void);

/**
 * Close file by index
 * file_index: Index of file to close
 * Returns: true if closed
 */
bool limitless_editor_close_file_by_index(uint32_t file_index);

/**
 * Switch to file by index
 * file_index: Index of file to activate
 * Returns: true if switched successfully
 */
bool limitless_editor_switch_to_file(uint32_t file_index);

/**
 * Get number of open files
 * Returns: Number of open files
 */
uint32_t limitless_editor_get_file_count(void);

/**
 * Check if current file has unsaved changes
 * Returns: true if file is modified
 */
bool limitless_editor_is_file_modified(void);

// ============================================================================
// TEXT EDITING API
// ============================================================================

/**
 * Insert text at current cursor position
 * text: Text to insert
 * Returns: true if inserted successfully
 */
bool limitless_editor_insert_text(const char* text);

/**
 * Delete text in current selection or character at cursor
 * Returns: true if text deleted
 */
bool limitless_editor_delete_text(void);

/**
 * Get selected text
 * buffer: Buffer to store selected text
 * buffer_size: Size of buffer
 * Returns: true if selection exists and copied
 */
bool limitless_editor_get_selected_text(char* buffer, size_t buffer_size);

/**
 * Copy selection to clipboard
 * Returns: true if copied successfully
 */
bool limitless_editor_copy(void);

/**
 * Cut selection to clipboard
 * Returns: true if cut successfully
 */
bool limitless_editor_cut(void);

/**
 * Paste from clipboard
 * Returns: true if pasted successfully
 */
bool limitless_editor_paste(void);

/**
 * Select all text
 */
void limitless_editor_select_all(void);

/**
 * Undo last operation
 * Returns: true if undo performed
 */
bool limitless_editor_undo(void);

/**
 * Redo last undone operation
 * Returns: true if redo performed
 */
bool limitless_editor_redo(void);

// ============================================================================
// CURSOR AND SELECTION API
// ============================================================================

/**
 * Get current cursor position
 * position: Pointer to store position
 */
void limitless_editor_get_cursor_position(editor_position_t* position);

/**
 * Set cursor position
 * position: New cursor position
 * Returns: true if position valid and set
 */
bool limitless_editor_set_cursor_position(const editor_position_t* position);

/**
 * Move cursor by offset
 * line_offset: Lines to move (positive = down, negative = up)
 * column_offset: Columns to move (positive = right, negative = left)
 * extend_selection: true to extend selection
 * Returns: true if cursor moved
 */
bool limitless_editor_move_cursor(int32_t line_offset, int32_t column_offset, bool extend_selection);

/**
 * Get current selection
 * selection: Pointer to store selection
 * Returns: true if selection is active
 */
bool limitless_editor_get_selection(editor_selection_t* selection);

/**
 * Set text selection
 * selection: Selection to set
 * Returns: true if selection set successfully
 */
bool limitless_editor_set_selection(const editor_selection_t* selection);

/**
 * Clear current selection
 */
void limitless_editor_clear_selection(void);

// ============================================================================
// SEARCH AND REPLACE API
// ============================================================================

/**
 * Find text in current file
 * query: Search query
 * case_sensitive: true for case-sensitive search
 * whole_word: true to match whole words only
 * use_regex: true to use regular expressions
 * Returns: true if text found
 */
bool limitless_editor_find(const char* query, bool case_sensitive, bool whole_word, bool use_regex);

/**
 * Find next occurrence
 * Returns: true if next occurrence found
 */
bool limitless_editor_find_next(void);

/**
 * Find previous occurrence
 * Returns: true if previous occurrence found
 */
bool limitless_editor_find_previous(void);

/**
 * Replace current selection or next occurrence
 * replacement: Replacement text
 * Returns: true if replaced
 */
bool limitless_editor_replace(const char* replacement);

/**
 * Replace all occurrences
 * query: Search query
 * replacement: Replacement text
 * case_sensitive: true for case-sensitive search
 * whole_word: true to match whole words only
 * use_regex: true to use regular expressions
 * Returns: Number of replacements made
 */
uint32_t limitless_editor_replace_all(const char* query, const char* replacement,
                                     bool case_sensitive, bool whole_word, bool use_regex);

// ============================================================================
// AI ASSISTANCE API
// ============================================================================

/**
 * Set AI assistance level
 * level: AI assistance level to set
 */
void limitless_editor_set_ai_level(editor_ai_level_t level);

/**
 * Get AI assistance level
 * Returns: Current AI assistance level
 */
editor_ai_level_t limitless_editor_get_ai_level(void);

/**
 * Trigger AI code completion
 * Returns: true if completion suggestions available
 */
bool limitless_editor_ai_complete(void);

/**
 * Accept current AI suggestion
 * Returns: true if suggestion accepted
 */
bool limitless_editor_ai_accept_suggestion(void);

/**
 * Reject current AI suggestion
 */
void limitless_editor_ai_reject_suggestion(void);

/**
 * Request AI refactoring suggestions
 * Returns: true if suggestions generated
 */
bool limitless_editor_ai_refactor(void);

/**
 * Generate documentation for current function/class
 * Returns: true if documentation generated
 */
bool limitless_editor_ai_generate_docs(void);

/**
 * Explain selected code with AI
 * explanation: Buffer to store explanation
 * buffer_size: Size of explanation buffer
 * Returns: true if explanation generated
 */
bool limitless_editor_ai_explain_code(char* explanation, size_t buffer_size);

/**
 * Analyze code for potential issues
 * Returns: Number of issues found
 */
uint32_t limitless_editor_ai_analyze_code(void);

// ============================================================================
// SYNTAX AND FORMATTING API
// ============================================================================

/**
 * Set syntax highlighting language
 * language: Programming language for syntax highlighting
 */
void limitless_editor_set_language(editor_language_t language);

/**
 * Get current syntax highlighting language
 * Returns: Current language
 */
editor_language_t limitless_editor_get_language(void);

/**
 * Auto-format current selection or entire file
 * Returns: true if formatting applied
 */
bool limitless_editor_format_code(void);

/**
 * Auto-indent current line or selection
 */
void limitless_editor_auto_indent(void);

/**
 * Comment/uncomment current line or selection
 */
void limitless_editor_toggle_comment(void);

/**
 * Convert tabs to spaces or vice versa
 * to_spaces: true to convert tabs to spaces, false for spaces to tabs
 */
void limitless_editor_convert_indentation(bool to_spaces);

// ============================================================================
// CONFIGURATION API
// ============================================================================

/**
 * Apply editor configuration
 * config: Configuration structure
 * Returns: true if config applied successfully
 */
bool limitless_editor_apply_config(const editor_config_t* config);

/**
 * Get current editor configuration
 * config: Buffer to store current config
 */
void limitless_editor_get_config(editor_config_t* config);

/**
 * Set editor theme
 * theme: Theme to apply
 */
void limitless_editor_set_theme(editor_theme_t theme);

/**
 * Set font size
 * size: Font size in points
 */
void limitless_editor_set_font_size(uint32_t size);

/**
 * Toggle line numbers visibility
 * show: true to show line numbers
 */
void limitless_editor_show_line_numbers(bool show);

/**
 * Toggle whitespace visibility
 * show: true to show whitespace characters
 */
void limitless_editor_show_whitespace(bool show);

/**
 * Toggle word wrap
 * enable: true to enable word wrap
 */
void limitless_editor_set_word_wrap(bool enable);

/**
 * Toggle minimap visibility
 * show: true to show minimap
 */
void limitless_editor_show_minimap(bool show);

/**
 * Toggle AI panel visibility
 * show: true to show AI panel
 */
void limitless_editor_show_ai_panel(bool show);

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

typedef void (*editor_text_changed_callback_t)(void);
typedef void (*editor_cursor_moved_callback_t)(uint32_t line, uint32_t column);
typedef void (*editor_file_opened_callback_t)(const char* file_path);
typedef void (*editor_file_saved_callback_t)(const char* file_path);
typedef void (*editor_ai_suggestion_callback_t)(const char* suggestion);

/**
 * Set text changed event callback
 * callback: Function to call when text is modified
 */
void limitless_editor_set_text_changed_callback(editor_text_changed_callback_t callback);

/**
 * Set cursor moved event callback
 * callback: Function to call when cursor position changes
 */
void limitless_editor_set_cursor_moved_callback(editor_cursor_moved_callback_t callback);

/**
 * Set file opened event callback
 * callback: Function to call when file is opened
 */
void limitless_editor_set_file_opened_callback(editor_file_opened_callback_t callback);

/**
 * Set file saved event callback
 * callback: Function to call when file is saved
 */
void limitless_editor_set_file_saved_callback(editor_file_saved_callback_t callback);

/**
 * Set AI suggestion event callback
 * callback: Function to call when AI suggestion is available
 */
void limitless_editor_set_ai_suggestion_callback(editor_ai_suggestion_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // LIMITLESS_EDITOR_H