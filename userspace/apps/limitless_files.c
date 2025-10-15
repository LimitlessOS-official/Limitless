/**
 * Limitless Files - Next-Generation File Manager
 * 
 * Intelligent file management with AI-powered organization, predictive search,
 * and seamless integration with the LimitlessOS ecosystem. Military-grade precision
 * meets intuitive design in the most advanced file manager ever created.
 */

#include "../ui/limitlessui.h"
#include "../desktop/limitless_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// ============================================================================
// FILE MANAGER CONSTANTS AND TYPES
// ============================================================================

#define FILES_VERSION           "1.0.0-Military"
#define MAX_PATH_LENGTH         4096
#define MAX_FILENAME_LENGTH     256
#define MAX_SEARCH_RESULTS      1000
#define MAX_BOOKMARKS           50
#define MAX_RECENT_FILES        100
#define THUMBNAIL_SIZE          128
#define PREVIEW_PANEL_WIDTH     300

// File type classifications for intelligent handling
typedef enum {
    FILE_TYPE_DIRECTORY = 0,
    FILE_TYPE_DOCUMENT,           // Text, PDF, Office docs
    FILE_TYPE_IMAGE,              // Photos, graphics
    FILE_TYPE_VIDEO,              // Video files
    FILE_TYPE_AUDIO,              // Music, audio
    FILE_TYPE_CODE,               // Source code, scripts
    FILE_TYPE_ARCHIVE,            // ZIP, TAR, etc.
    FILE_TYPE_EXECUTABLE,         // Binary executables
    FILE_TYPE_SYSTEM,             // System files
    FILE_TYPE_UNKNOWN
} file_type_t;

// View modes for different use cases
typedef enum {
    VIEW_MODE_ICONS = 0,          // Large icons with names
    VIEW_MODE_LIST,               // Detailed list view
    VIEW_MODE_COLUMNS,            // Multi-column browser
    VIEW_MODE_TILES,              // Medium tiles with details
    VIEW_MODE_TIMELINE,           // Chronological timeline view
    VIEW_MODE_AI_CLUSTERS         // AI-organized smart clusters
} view_mode_t;

// Sort criteria for file organization
typedef enum {
    SORT_BY_NAME = 0,
    SORT_BY_SIZE,
    SORT_BY_DATE_MODIFIED,
    SORT_BY_DATE_CREATED,
    SORT_BY_TYPE,
    SORT_BY_RELEVANCE,            // AI-powered relevance scoring
    SORT_BY_USAGE_FREQUENCY       // How often files are accessed
} sort_criteria_t;

// File metadata structure
typedef struct file_entry {
    char name[MAX_FILENAME_LENGTH];
    char full_path[MAX_PATH_LENGTH];
    char display_name[MAX_FILENAME_LENGTH];
    
    file_type_t type;
    uint64_t size;
    time_t created_time;
    time_t modified_time;
    time_t accessed_time;
    
    // Permissions and ownership
    mode_t permissions;
    uid_t owner_uid;
    gid_t group_gid;
    
    // AI-enhanced metadata
    float relevance_score;        // AI-calculated relevance (0.0-1.0)
    uint32_t access_count;        // How many times opened
    float user_rating;            // User-assigned rating (0.0-1.0)
    char tags[256];               // User or AI-assigned tags
    
    // Visual representation
    uint32_t icon_id;
    uint32_t thumbnail_id;
    lui_color_t type_color;
    
    // Linking and relationships
    bool is_symlink;
    char symlink_target[MAX_PATH_LENGTH];
    bool is_favorite;
    bool is_recent;
    
    struct file_entry* next;
    struct file_entry* prev;
} file_entry_t;

// Bookmark for quick navigation
typedef struct bookmark {
    char name[64];
    char path[MAX_PATH_LENGTH];
    char description[128];
    uint32_t icon_id;
    lui_color_t color;
    uint32_t usage_count;
    time_t created_time;
    time_t last_used_time;
} bookmark_t;

// Search context for intelligent search
typedef struct search_context {
    char query[256];
    char current_directory[MAX_PATH_LENGTH];
    file_type_t filter_type;
    bool include_subdirectories;
    bool case_sensitive;
    bool use_ai_search;           // Enable AI-powered semantic search
    time_t date_range_start;
    time_t date_range_end;
    uint64_t size_min;
    uint64_t size_max;
} search_context_t;

// File operations context
typedef struct file_operation {
    enum {
        OP_COPY = 0,
        OP_MOVE,
        OP_DELETE,
        OP_COMPRESS,
        OP_EXTRACT,
        OP_ENCRYPT,
        OP_DECRYPT
    } type;
    
    char source_paths[100][MAX_PATH_LENGTH];  // Multiple source files
    char destination_path[MAX_PATH_LENGTH];
    uint32_t source_count;
    
    // Progress tracking
    uint64_t total_bytes;
    uint64_t processed_bytes;
    uint32_t total_files;
    uint32_t processed_files;
    bool completed;
    bool cancelled;
    char error_message[256];
    
    time_t start_time;
    time_t estimated_completion;
} file_operation_t;

// Main file manager state
typedef struct limitless_files {
    bool initialized;
    
    // UI components
    lui_window_t* main_window;
    lui_widget_t* toolbar;
    lui_widget_t* address_bar;
    lui_widget_t* search_bar;
    lui_widget_t* sidebar;
    lui_widget_t* file_view;
    lui_widget_t* preview_panel;
    lui_widget_t* status_bar;
    
    // Navigation state
    char current_path[MAX_PATH_LENGTH];
    char* path_history[100];
    int history_position;
    int history_count;
    
    // File management
    file_entry_t* files;
    uint32_t file_count;
    file_entry_t* selected_files;
    uint32_t selected_count;
    
    // View configuration
    view_mode_t view_mode;
    sort_criteria_t sort_criteria;
    bool sort_ascending;
    bool show_hidden_files;
    bool show_file_extensions;
    float icon_size;
    
    // Bookmarks and favorites
    bookmark_t bookmarks[MAX_BOOKMARKS];
    uint32_t bookmark_count;
    file_entry_t* recent_files[MAX_RECENT_FILES];
    uint32_t recent_file_count;
    
    // Search functionality
    search_context_t current_search;
    file_entry_t* search_results;
    uint32_t search_result_count;
    bool search_in_progress;
    
    // File operations
    file_operation_t* active_operations;
    uint32_t operation_count;
    
    // AI features
    bool ai_suggestions_enabled;
    bool ai_auto_organize;
    bool ai_smart_preview;
    float ai_confidence_threshold;
    
    // Performance optimization
    bool thumbnail_cache_enabled;
    bool lazy_loading_enabled;
    uint32_t cache_size_mb;
    
    // Security and privacy
    bool secure_delete_enabled;
    bool encryption_available;
    uint8_t security_level;
    
} limitless_files_t;

// Global file manager instance
static limitless_files_t g_files = {0};

// ============================================================================
// FILE TYPE DETECTION AND CLASSIFICATION
// ============================================================================

static file_type_t detect_file_type(const char* filename, mode_t mode) {
    if (S_ISDIR(mode)) {
        return FILE_TYPE_DIRECTORY;
    }
    
    if (!filename) return FILE_TYPE_UNKNOWN;
    
    // Find file extension
    const char* ext = strrchr(filename, '.');
    if (!ext) return FILE_TYPE_UNKNOWN;
    ext++; // Skip the dot
    
    // Document types
    if (strcasecmp(ext, "txt") == 0 || strcasecmp(ext, "md") == 0 || 
        strcasecmp(ext, "doc") == 0 || strcasecmp(ext, "docx") == 0 ||
        strcasecmp(ext, "pdf") == 0 || strcasecmp(ext, "rtf") == 0 ||
        strcasecmp(ext, "odt") == 0) {
        return FILE_TYPE_DOCUMENT;
    }
    
    // Image types
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0 ||
        strcasecmp(ext, "png") == 0 || strcasecmp(ext, "gif") == 0 ||
        strcasecmp(ext, "bmp") == 0 || strcasecmp(ext, "svg") == 0 ||
        strcasecmp(ext, "tiff") == 0 || strcasecmp(ext, "webp") == 0) {
        return FILE_TYPE_IMAGE;
    }
    
    // Video types
    if (strcasecmp(ext, "mp4") == 0 || strcasecmp(ext, "avi") == 0 ||
        strcasecmp(ext, "mkv") == 0 || strcasecmp(ext, "mov") == 0 ||
        strcasecmp(ext, "wmv") == 0 || strcasecmp(ext, "webm") == 0) {
        return FILE_TYPE_VIDEO;
    }
    
    // Audio types
    if (strcasecmp(ext, "mp3") == 0 || strcasecmp(ext, "wav") == 0 ||
        strcasecmp(ext, "flac") == 0 || strcasecmp(ext, "ogg") == 0 ||
        strcasecmp(ext, "m4a") == 0 || strcasecmp(ext, "aac") == 0) {
        return FILE_TYPE_AUDIO;
    }
    
    // Code types
    if (strcasecmp(ext, "c") == 0 || strcasecmp(ext, "cpp") == 0 ||
        strcasecmp(ext, "h") == 0 || strcasecmp(ext, "hpp") == 0 ||
        strcasecmp(ext, "py") == 0 || strcasecmp(ext, "js") == 0 ||
        strcasecmp(ext, "html") == 0 || strcasecmp(ext, "css") == 0 ||
        strcasecmp(ext, "java") == 0 || strcasecmp(ext, "rs") == 0 ||
        strcasecmp(ext, "go") == 0 || strcasecmp(ext, "sh") == 0) {
        return FILE_TYPE_CODE;
    }
    
    // Archive types
    if (strcasecmp(ext, "zip") == 0 || strcasecmp(ext, "tar") == 0 ||
        strcasecmp(ext, "gz") == 0 || strcasecmp(ext, "7z") == 0 ||
        strcasecmp(ext, "rar") == 0 || strcasecmp(ext, "bz2") == 0) {
        return FILE_TYPE_ARCHIVE;
    }
    
    // Executable types
    if (strcasecmp(ext, "exe") == 0 || strcasecmp(ext, "app") == 0 ||
        strcasecmp(ext, "deb") == 0 || strcasecmp(ext, "rpm") == 0 ||
        strcasecmp(ext, "dmg") == 0 || (S_ISREG(mode) && (mode & S_IXUSR))) {
        return FILE_TYPE_EXECUTABLE;
    }
    
    return FILE_TYPE_UNKNOWN;
}

static lui_color_t get_file_type_color(file_type_t type) {
    switch (type) {
        case FILE_TYPE_DIRECTORY:   return LUI_COLOR_TACTICAL_BLUE;
        case FILE_TYPE_DOCUMENT:    return LUI_COLOR_PLATINUM;
        case FILE_TYPE_IMAGE:       return LUI_COLOR_MISSION_GREEN;
        case FILE_TYPE_VIDEO:       return LUI_COLOR_CRITICAL_RED;
        case FILE_TYPE_AUDIO:       return LUI_COLOR_INTEL_PURPLE;
        case FILE_TYPE_CODE:        return LUI_COLOR_SECURE_CYAN;
        case FILE_TYPE_ARCHIVE:     return LUI_COLOR_ALERT_AMBER;
        case FILE_TYPE_EXECUTABLE:  return LUI_COLOR_MISSION_GREEN;
        case FILE_TYPE_SYSTEM:      return LUI_COLOR_STEEL_GRAY;
        default:                    return LUI_COLOR_TITANIUM;
    }
}

static const char* get_file_type_icon(file_type_t type) {
    switch (type) {
        case FILE_TYPE_DIRECTORY:   return "📁";
        case FILE_TYPE_DOCUMENT:    return "📄";
        case FILE_TYPE_IMAGE:       return "🖼️";
        case FILE_TYPE_VIDEO:       return "🎬";
        case FILE_TYPE_AUDIO:       return "🎵";
        case FILE_TYPE_CODE:        return "💻";
        case FILE_TYPE_ARCHIVE:     return "📦";
        case FILE_TYPE_EXECUTABLE:  return "⚡";
        case FILE_TYPE_SYSTEM:      return "⚙️";
        default:                    return "📄";
    }
}

// ============================================================================
// FILE SYSTEM OPERATIONS
// ============================================================================

static void format_file_size(uint64_t bytes, char* buffer, size_t buffer_size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = (double)bytes;
    int unit_index = 0;
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%llu B", (unsigned long long)bytes);
    } else {
        snprintf(buffer, buffer_size, "%.1f %s", size, units[unit_index]);
    }
}

static void format_file_date(time_t timestamp, char* buffer, size_t buffer_size) {
    struct tm* tm_info = localtime(&timestamp);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M", tm_info);
}

static float calculate_file_relevance(const file_entry_t* file, const char* context) {
    // AI-powered relevance calculation
    // This is a simplified version - real implementation would use ML models
    
    float score = 0.5f; // Base score
    
    // Recent files get higher score
    time_t now = time(NULL);
    double days_since_access = difftime(now, file->accessed_time) / (24.0 * 3600.0);
    if (days_since_access < 1.0) score += 0.3f;
    else if (days_since_access < 7.0) score += 0.2f;
    else if (days_since_access < 30.0) score += 0.1f;
    
    // Frequently accessed files get higher score
    if (file->access_count > 10) score += 0.2f;
    else if (file->access_count > 5) score += 0.1f;
    
    // User-rated files get priority
    if (file->user_rating > 0.7f) score += 0.2f;
    else if (file->user_rating > 0.5f) score += 0.1f;
    
    // Favorites always get high score
    if (file->is_favorite) score += 0.3f;
    
    return fminf(1.0f, score);
}

static bool load_directory(const char* path) {
    if (!path) return false;
    
    printf("[Files] Loading directory: %s\n", path);
    
    DIR* dir = opendir(path);
    if (!dir) {
        printf("[Files] ERROR: Failed to open directory: %s\n", path);
        return false;
    }
    
    // Clear existing files
    file_entry_t* file = g_files.files;
    while (file) {
        file_entry_t* next = file->next;
        free(file);
        file = next;
    }
    g_files.files = NULL;
    g_files.file_count = 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Skip hidden files if not showing them
        if (!g_files.show_hidden_files && entry->d_name[0] == '.') {
            continue;
        }
        
        // Create file entry
        file_entry_t* file_entry = calloc(1, sizeof(file_entry_t));
        if (!file_entry) continue;
        
        strncpy(file_entry->name, entry->d_name, sizeof(file_entry->name) - 1);
        snprintf(file_entry->full_path, sizeof(file_entry->full_path), 
                "%s/%s", path, entry->d_name);
        
        // Get file stats
        struct stat file_stat;
        if (stat(file_entry->full_path, &file_stat) == 0) {
            file_entry->size = file_stat.st_size;
            file_entry->created_time = file_stat.st_ctime;
            file_entry->modified_time = file_stat.st_mtime;
            file_entry->accessed_time = file_stat.st_atime;
            file_entry->permissions = file_stat.st_mode;
            file_entry->owner_uid = file_stat.st_uid;
            file_entry->group_gid = file_stat.st_gid;
            
            // Check if it's a symlink
            if (S_ISLNK(file_stat.st_mode)) {
                file_entry->is_symlink = true;
                readlink(file_entry->full_path, file_entry->symlink_target, 
                        sizeof(file_entry->symlink_target) - 1);
            }
        }
        
        // Detect file type and set properties
        file_entry->type = detect_file_type(entry->d_name, file_entry->permissions);
        file_entry->type_color = get_file_type_color(file_entry->type);
        
        // Calculate AI relevance
        file_entry->relevance_score = calculate_file_relevance(file_entry, path);
        
        // Set display name
        if (g_files.show_file_extensions) {
            strcpy(file_entry->display_name, file_entry->name);
        } else {
            // Remove extension for display
            char* dot = strrchr(file_entry->name, '.');
            if (dot && file_entry->type != FILE_TYPE_DIRECTORY) {
                size_t name_len = dot - file_entry->name;
                strncpy(file_entry->display_name, file_entry->name, name_len);
                file_entry->display_name[name_len] = '\0';
            } else {
                strcpy(file_entry->display_name, file_entry->name);
            }
        }
        
        // Add to file list
        file_entry->next = g_files.files;
        if (g_files.files) {
            g_files.files->prev = file_entry;
        }
        g_files.files = file_entry;
        g_files.file_count++;
    }
    
    closedir(dir);
    
    // Update current path
    strncpy(g_files.current_path, path, sizeof(g_files.current_path) - 1);
    
    printf("[Files] Loaded %u files from %s\n", g_files.file_count, path);
    return true;
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_toolbar(void) {
    g_files.toolbar = lui_create_container(g_files.main_window->root_widget);
    strcpy(g_files.toolbar->name, "toolbar");
    
    // Configure toolbar appearance
    g_files.toolbar->bounds = lui_rect_make(0, 0, 1200, 48);
    g_files.toolbar->background_color = LUI_COLOR_GRAPHITE;
    
    // Navigation buttons
    lui_widget_t* back_button = lui_create_button("◀", g_files.toolbar);
    back_button->bounds = lui_rect_make(8, 8, 32, 32);
    
    lui_widget_t* forward_button = lui_create_button("▶", g_files.toolbar);
    forward_button->bounds = lui_rect_make(48, 8, 32, 32);
    
    lui_widget_t* up_button = lui_create_button("▲", g_files.toolbar);
    up_button->bounds = lui_rect_make(88, 8, 32, 32);
    
    lui_widget_t* home_button = lui_create_button("🏠", g_files.toolbar);
    home_button->bounds = lui_rect_make(128, 8, 32, 32);
    
    // View mode buttons
    lui_widget_t* view_icons = lui_create_button("🔷", g_files.toolbar);
    view_icons->bounds = lui_rect_make(200, 8, 32, 32);
    
    lui_widget_t* view_list = lui_create_button("☰", g_files.toolbar);
    view_list->bounds = lui_rect_make(240, 8, 32, 32);
    
    lui_widget_t* view_columns = lui_create_button("▦", g_files.toolbar);
    view_columns->bounds = lui_rect_make(280, 8, 32, 32);
}

static void create_address_bar(void) {
    g_files.address_bar = lui_create_text_input("Enter path...", g_files.main_window->root_widget);
    strcpy(g_files.address_bar->name, "address_bar");
    g_files.address_bar->bounds = lui_rect_make(0, 48, 800, 32);
    lui_text_input_set_text(g_files.address_bar, g_files.current_path);
}

static void create_search_bar(void) {
    g_files.search_bar = lui_create_text_input("Search files...", g_files.main_window->root_widget);
    strcpy(g_files.search_bar->name, "search_bar");
    g_files.search_bar->bounds = lui_rect_make(800, 48, 400, 32);
}

static void create_sidebar(void) {
    g_files.sidebar = lui_create_container(g_files.main_window->root_widget);
    strcpy(g_files.sidebar->name, "sidebar");
    g_files.sidebar->bounds = lui_rect_make(0, 80, 200, 520);
    g_files.sidebar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Create bookmark entries
    float y_offset = 8;
    
    // Quick access bookmarks
    const char* quick_bookmarks[] = {
        "🏠 Home", "📁 Documents", "📷 Pictures", "🎵 Music", 
        "🎬 Videos", "💾 Downloads", "🗑️ Trash"
    };
    
    for (int i = 0; i < 7; i++) {
        lui_widget_t* bookmark = lui_create_label(quick_bookmarks[i], g_files.sidebar);
        bookmark->bounds = lui_rect_make(8, y_offset, 184, 24);
        bookmark->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
        y_offset += 32;
    }
}

static void create_file_view(void) {
    g_files.file_view = lui_create_container(g_files.main_window->root_widget);
    strcpy(g_files.file_view->name, "file_view");
    g_files.file_view->bounds = lui_rect_make(200, 80, 700, 520);
    g_files.file_view->background_color = LUI_COLOR_TACTICAL_BLACK;
    
    // TODO: Implement dynamic file list rendering based on view mode
}

static void create_preview_panel(void) {
    g_files.preview_panel = lui_create_container(g_files.main_window->root_widget);
    strcpy(g_files.preview_panel->name, "preview_panel");
    g_files.preview_panel->bounds = lui_rect_make(900, 80, PREVIEW_PANEL_WIDTH, 520);
    g_files.preview_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // Preview title
    lui_widget_t* preview_title = lui_create_label("Preview", g_files.preview_panel);
    preview_title->bounds = lui_rect_make(8, 8, 284, 24);
    preview_title->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
}

static void create_status_bar(void) {
    g_files.status_bar = lui_create_container(g_files.main_window->root_widget);
    strcpy(g_files.status_bar->name, "status_bar");
    g_files.status_bar->bounds = lui_rect_make(0, 600, 1200, 24);
    g_files.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Status info
    char status_text[256];
    snprintf(status_text, sizeof(status_text), "%u items in %s", 
             g_files.file_count, g_files.current_path);
    
    lui_widget_t* status_label = lui_create_label(status_text, g_files.status_bar);
    status_label->bounds = lui_rect_make(8, 2, 400, 20);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
}

// ============================================================================
// MAIN FILE MANAGER API
// ============================================================================

bool limitless_files_init(void) {
    if (g_files.initialized) {
        return false;
    }
    
    printf("[Files] Initializing Limitless Files v%s\n", FILES_VERSION);
    
    // Clear state
    memset(&g_files, 0, sizeof(g_files));
    
    // Set default configuration
    g_files.view_mode = VIEW_MODE_ICONS;
    g_files.sort_criteria = SORT_BY_NAME;
    g_files.sort_ascending = true;
    g_files.show_hidden_files = false;
    g_files.show_file_extensions = true;
    g_files.icon_size = 1.0f;
    g_files.ai_suggestions_enabled = true;
    g_files.ai_auto_organize = false;
    g_files.ai_smart_preview = true;
    g_files.ai_confidence_threshold = 0.7f;
    g_files.thumbnail_cache_enabled = true;
    g_files.lazy_loading_enabled = true;
    g_files.cache_size_mb = 256;
    g_files.secure_delete_enabled = true;
    g_files.security_level = SECURITY_LEVEL_OPEN;
    
    // Create main window
    g_files.main_window = lui_create_window("Limitless Files", LUI_WINDOW_NORMAL, 
                                          100, 100, 1200, 624);
    if (!g_files.main_window) {
        printf("[Files] ERROR: Failed to create main window\n");
        return false;
    }
    
    // Create UI components
    create_toolbar();
    create_address_bar();
    create_search_bar();
    create_sidebar();
    create_file_view();
    create_preview_panel();
    create_status_bar();
    
    // Load home directory initially
    const char* home_dir = getenv("HOME");
    if (!home_dir) home_dir = "/home";
    
    load_directory(home_dir);
    
    // Show window
    lui_show_window(g_files.main_window);
    
    g_files.initialized = true;
    
    printf("[Files] File manager initialized successfully\n");
    printf("[Files] Current directory: %s (%u files)\n", 
           g_files.current_path, g_files.file_count);
    
    return true;
}

void limitless_files_shutdown(void) {
    if (!g_files.initialized) {
        return;
    }
    
    printf("[Files] Shutting down Limitless Files\n");
    
    // Clear file list
    file_entry_t* file = g_files.files;
    while (file) {
        file_entry_t* next = file->next;
        free(file);
        file = next;
    }
    
    // Clear search results
    file = g_files.search_results;
    while (file) {
        file_entry_t* next = file->next;
        free(file);
        file = next;
    }
    
    // Clear path history
    for (int i = 0; i < g_files.history_count; i++) {
        if (g_files.path_history[i]) {
            free(g_files.path_history[i]);
        }
    }
    
    // Destroy main window
    if (g_files.main_window) {
        lui_destroy_window(g_files.main_window);
    }
    
    memset(&g_files, 0, sizeof(g_files));
    
    printf("[Files] Shutdown complete\n");
}

bool limitless_files_navigate_to(const char* path) {
    if (!g_files.initialized || !path) {
        return false;
    }
    
    return load_directory(path);
}

const char* limitless_files_get_current_path(void) {
    return g_files.initialized ? g_files.current_path : NULL;
}

void limitless_files_run(void) {
    if (!g_files.initialized) {
        printf("[Files] ERROR: File manager not initialized\n");
        return;
    }
    
    printf("[Files] Running Limitless Files\n");
    
    // Main event loop is handled by the desktop environment
    // This function serves as an entry point for standalone execution
}