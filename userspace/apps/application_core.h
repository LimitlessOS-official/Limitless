/*
 * LimitlessOS Core Applications Suite
 * Essential desktop applications with modern features
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../compositor/compositor_core.h"

/* Application constants */
#define MAX_APPLICATIONS        256
#define MAX_DOCUMENTS           64
#define MAX_RECENT_FILES        32
#define MAX_BOOKMARKS           128
#define MAX_TABS                32

/* Application types */
typedef enum {
    APP_TYPE_UNKNOWN = 0,
    APP_TYPE_FILE_MANAGER,
    APP_TYPE_WEB_BROWSER,
    APP_TYPE_TEXT_EDITOR,
    APP_TYPE_TERMINAL,
    APP_TYPE_MEDIA_PLAYER,
    APP_TYPE_IMAGE_VIEWER,
    APP_TYPE_CALCULATOR,
    APP_TYPE_SETTINGS,
    APP_TYPE_APP_STORE,
    APP_TYPE_EMAIL_CLIENT,
    APP_TYPE_CALENDAR,
    APP_TYPE_NOTES,
    APP_TYPE_PDF_VIEWER,
    APP_TYPE_ARCHIVE_MANAGER,
    APP_TYPE_SYSTEM_MONITOR,
    APP_TYPE_GAME,
    APP_TYPE_OFFICE_SUITE,
    APP_TYPE_IDE,
    APP_TYPE_MAX
} app_type_t;

/* File Manager */
typedef struct file_manager {
    struct window *main_window;
    
    /* Navigation */
    struct {
        char current_path[1024];
        char history[32][1024];
        int history_count;
        int history_index;
    } navigation;
    
    /* View settings */
    struct {
        enum {
            VIEW_ICONS = 0,
            VIEW_LIST,
            VIEW_DETAILS,
            VIEW_THUMBNAILS
        } view_mode;
        
        bool show_hidden_files;
        bool show_file_extensions;
        enum {
            SORT_NAME = 0,
            SORT_SIZE,
            SORT_DATE,
            SORT_TYPE
        } sort_mode;
        bool sort_ascending;
    } view;
    
    /* File operations */
    struct {
        char clipboard_paths[64][1024];
        int clipboard_count;
        bool clipboard_cut; /* true for cut, false for copy */
        
        struct {
            char source[1024];
            char destination[1024];
            uint64_t total_size;
            uint64_t copied_size;
            bool active;
        } transfer;
    } operations;
    
    /* Bookmarks */
    struct {
        struct {
            char name[64];
            char path[1024];
        } bookmarks[MAX_BOOKMARKS];
        int count;
    } bookmarks;
    
    /* Sidebar */
    struct {
        bool visible;
        struct {
            char name[32];
            char path[1024];
            bool mounted;
        } devices[16];
        int device_count;
    } sidebar;
    
} file_manager_t;

/* Web Browser */
typedef struct web_browser {
    struct window *main_window;
    
    /* Tabs */
    struct {
        struct {
            char title[256];
            char url[2048];
            bool loading;
            bool secure;
            void *page_data;
        } tabs[MAX_TABS];
        int count;
        int active_tab;
    } tabs;
    
    /* Navigation */
    struct {
        char address_bar[2048];
        char search_engine[256];
        bool private_mode;
    } navigation;
    
    /* Bookmarks */
    struct {
        struct {
            char title[256];
            char url[2048];
            char folder[64];
        } bookmarks[MAX_BOOKMARKS];
        int count;
    } bookmarks;
    
    /* History */
    struct {
        struct {
            char title[256];
            char url[2048];
            uint64_t timestamp;
        } entries[1024];
        int count;
    } history;
    
    /* Downloads */
    struct {
        struct {
            char filename[256];
            char url[2048];
            uint64_t size;
            uint64_t downloaded;
            bool complete;
        } downloads[32];
        int count;
    } downloads;
    
    /* Settings */
    struct {
        bool javascript_enabled;
        bool cookies_enabled;
        bool popup_blocker;
        bool ad_blocker;
        int zoom_level;
        char user_agent[512];
    } settings;
    
} web_browser_t;

/* Text Editor */
typedef struct text_editor {
    struct window *main_window;
    
    /* Documents */
    struct {
        struct {
            char filename[256];
            char *content;
            size_t content_size;
            size_t content_capacity;
            bool modified;
            bool read_only;
            
            /* Cursor and selection */
            struct {
                int line;
                int column;
                int selection_start_line;
                int selection_start_column;
                int selection_end_line;
                int selection_end_column;
                bool has_selection;
            } cursor;
            
            /* View state */
            struct {
                int scroll_line;
                int scroll_column;
                int zoom_level;
            } view;
            
        } documents[MAX_DOCUMENTS];
        int count;
        int active_document;
    } documents;
    
    /* Editor settings */
    struct {
        bool line_numbers;
        bool syntax_highlighting;
        bool auto_indent;
        bool word_wrap;
        bool show_whitespace;
        int tab_size;
        bool use_spaces;
        char font_family[64];
        int font_size;
    } settings;
    
    /* Search and replace */
    struct {
        char search_text[256];
        char replace_text[256];
        bool case_sensitive;
        bool whole_word;
        bool use_regex;
        bool find_backwards;
    } search;
    
    /* Recent files */
    struct {
        char files[MAX_RECENT_FILES][256];
        int count;
    } recent;
    
} text_editor_t;

/* Terminal Emulator */
typedef struct terminal {
    struct window *main_window;
    
    /* Terminal state */
    struct {
        char *buffer;
        int rows;
        int cols;
        int cursor_row;
        int cursor_col;
        bool cursor_visible;
        
        /* Colors */
        uint32_t foreground_color;
        uint32_t background_color;
        uint32_t cursor_color;
        
        /* Attributes */
        bool bold;
        bool italic;
        bool underline;
        bool reverse;
        
    } terminal;
    
    /* Shell process */
    struct {
        struct process *shell_process;
        int master_fd;
        int slave_fd;
    } shell;
    
    /* Settings */
    struct {
        char font_family[64];
        int font_size;
        int scrollback_lines;
        bool bell_enabled;
        char shell_command[256];
        
        /* Color scheme */
        uint32_t color_palette[16];
        
    } settings;
    
    /* History */
    struct {
        char commands[256][512];
        int count;
        int index;
    } history;
    
    /* Tabs */
    struct {
        struct {
            char title[64];
            void *terminal_data;
        } tabs[16];
        int count;
        int active_tab;
    } tabs;
    
} terminal_t;

/* Media Player */
typedef struct media_player {
    struct window *main_window;
    
    /* Current media */
    struct {
        char filename[512];
        enum {
            MEDIA_TYPE_AUDIO = 0,
            MEDIA_TYPE_VIDEO,
            MEDIA_TYPE_PLAYLIST
        } type;
        
        uint64_t duration_ms;
        uint64_t position_ms;
        bool playing;
        bool paused;
        float volume;
        bool muted;
        
        /* Metadata */
        char title[256];
        char artist[256];
        char album[256];
        char genre[64];
        int year;
        
    } media;
    
    /* Playlist */
    struct {
        struct {
            char filename[512];
            char title[256];
            char artist[256];
            uint64_t duration_ms;
        } items[512];
        int count;
        int current_index;
        bool shuffle;
        enum {
            REPEAT_NONE = 0,
            REPEAT_ONE,
            REPEAT_ALL
        } repeat_mode;
    } playlist;
    
    /* Video display */
    struct {
        int video_width;
        int video_height;
        float aspect_ratio;
        bool fullscreen;
        int brightness;
        int contrast;
        int saturation;
    } video;
    
    /* Audio settings */
    struct {
        bool equalizer_enabled;
        float equalizer_bands[10];
        bool bass_boost;
        bool surround_sound;
    } audio;
    
} media_player_t;

/* Image Viewer */
typedef struct image_viewer {
    struct window *main_window;
    
    /* Current image */
    struct {
        char filename[512];
        void *image_data;
        int width;
        int height;
        int channels;
        int bit_depth;
        
        /* View state */
        float zoom_factor;
        int pan_x;
        int pan_y;
        int rotation; /* 0, 90, 180, 270 degrees */
        
        /* EXIF data */
        struct {
            char camera_make[64];
            char camera_model[64];
            char date_taken[32];
            float focal_length;
            float aperture;
            float shutter_speed;
            int iso;
        } exif;
        
    } image;
    
    /* Navigation */
    struct {
        char directory[1024];
        char image_files[256][256];
        int file_count;
        int current_index;
        bool slideshow_mode;
        int slideshow_interval_s;
    } navigation;
    
    /* Tools */
    struct {
        bool crop_mode;
        bool rotate_mode;
        bool color_adjust_mode;
        
        struct {
            int x;
            int y;
            int width;
            int height;
        } crop_selection;
        
        struct {
            float brightness;
            float contrast;
            float saturation;
            float gamma;
        } color_adjustment;
        
    } tools;
    
    /* Settings */
    struct {
        bool fit_to_window;
        bool smooth_scaling;
        uint32_t background_color;
        bool show_toolbar;
        bool show_status_bar;
    } settings;
    
} image_viewer_t;

/* Calculator */
typedef struct calculator {
    struct window *main_window;
    
    /* Display */
    struct {
        char display_text[64];
        double current_value;
        double stored_value;
        double memory_value;
        
        enum {
            CALC_MODE_BASIC = 0,
            CALC_MODE_SCIENTIFIC,
            CALC_MODE_PROGRAMMER
        } mode;
        
    } display;
    
    /* Operation state */
    struct {
        enum {
            OP_NONE = 0,
            OP_ADD,
            OP_SUBTRACT,
            OP_MULTIPLY,
            OP_DIVIDE,
            OP_POWER,
            OP_ROOT,
            OP_PERCENT,
            OP_FACTORIAL
        } pending_operation;
        
        bool waiting_for_operand;
        bool clear_on_next_digit;
        
    } operation;
    
    /* History */
    struct {
        struct {
            char expression[128];
            double result;
        } entries[32];
        int count;
    } history;
    
    /* Settings */
    struct {
        int decimal_places;
        bool use_thousands_separator;
        enum {
            ANGLE_DEGREES = 0,
            ANGLE_RADIANS,
            ANGLE_GRADIANS
        } angle_mode;
        
        enum {
            BASE_DECIMAL = 0,
            BASE_BINARY,
            BASE_OCTAL,
            BASE_HEXADECIMAL
        } number_base;
        
    } settings;
    
} calculator_t;

/* Settings Application */
typedef struct settings_app {
    struct window *main_window;
    
    /* Categories */
    enum {
        SETTINGS_SYSTEM = 0,
        SETTINGS_DISPLAY,
        SETTINGS_SOUND,
        SETTINGS_NETWORK,
        SETTINGS_BLUETOOTH,
        SETTINGS_POWER,
        SETTINGS_PRIVACY,
        SETTINGS_ACCESSIBILITY,
        SETTINGS_ACCOUNTS,
        SETTINGS_APPLICATIONS,
        SETTINGS_KEYBOARD,
        SETTINGS_MOUSE,
        SETTINGS_PRINTERS,
        SETTINGS_REGION,
        SETTINGS_UPDATE,
        SETTINGS_ABOUT
    } current_category;
    
    /* System settings */
    struct {
        char hostname[64];
        char timezone[64];
        bool automatic_login;
        bool guest_account;
        int session_timeout_min;
    } system;
    
    /* Display settings */
    struct {
        struct display_config displays[16];
        int display_count;
        int primary_display;
        struct theme current_theme;
        bool night_mode;
        int night_mode_temperature;
    } display;
    
    /* Network settings */
    struct {
        struct {
            char ssid[64];
            char security[32];
            bool connected;
            int signal_strength;
        } wifi_networks[32];
        int wifi_count;
        
        bool wifi_enabled;
        bool ethernet_enabled;
        bool airplane_mode;
        
        struct {
            char address[64];
            char netmask[64];
            char gateway[64];
            char dns_primary[64];
            char dns_secondary[64];
            bool dhcp_enabled;
        } network_config;
        
    } network;
    
    /* Privacy settings */
    struct {
        bool location_enabled;
        bool camera_enabled;
        bool microphone_enabled;
        bool analytics_enabled;
        bool crash_reports_enabled;
        
        struct {
            char app_names[64][64];
            bool camera_permission[64];
            bool microphone_permission[64];
            bool location_permission[64];
            bool file_permission[64];
            int app_count;
        } app_permissions;
        
    } privacy;
    
} settings_app_t;

/* App Store */
typedef struct app_store {
    struct window *main_window;
    
    /* Categories */
    enum {
        STORE_FEATURED = 0,
        STORE_CATEGORIES,
        STORE_SEARCH,
        STORE_INSTALLED,
        STORE_UPDATES
    } current_view;
    
    /* Applications */
    struct {
        struct {
            char name[128];
            char description[512];
            char developer[128];
            char version[32];
            char category[64];
            float rating;
            int download_count;
            uint64_t size_bytes;
            bool free;
            float price;
            char icon_url[256];
            char screenshot_urls[8][256];
            int screenshot_count;
            bool installed;
            bool update_available;
        } apps[1024];
        int count;
        int selected_app;
    } apps;
    
    /* Search */
    struct {
        char query[256];
        char category_filter[64];
        float min_rating;
        bool free_only;
        enum {
            SORT_RELEVANCE = 0,
            SORT_RATING,
            SORT_DOWNLOADS,
            SORT_NAME,
            SORT_DATE
        } sort_mode;
    } search;
    
    /* Downloads */
    struct {
        struct {
            char app_name[128];
            uint64_t size_bytes;
            uint64_t downloaded_bytes;
            bool installing;
            bool complete;
            bool error;
        } downloads[16];
        int count;
    } downloads;
    
    /* User account */
    struct {
        char username[64];
        char email[128];
        bool logged_in;
        struct {
            char app_names[128][128];
            char purchase_dates[128][32];
            int count;
        } purchased_apps;
    } account;
    
} app_store_t;

/* Application registry */
typedef struct application_registry {
    struct {
        char name[64];
        char executable[256];
        char icon[256];
        char description[256];
        app_type_t type;
        bool system_app;
        bool autostart;
        struct process *process;
    } applications[MAX_APPLICATIONS];
    int count;
    
    /* MIME type associations */
    struct {
        char mime_type[64];
        char application[64];
    } mime_associations[256];
    int mime_count;
    
} application_registry_t;

/* External application registry */
extern application_registry_t app_registry;

/* Core application functions */
int applications_init(void);
void applications_exit(void);

/* Application management */
int app_register(const char *name, const char *executable, const char *icon, 
                const char *description, app_type_t type);
int app_launch(const char *name, const char *args[]);
int app_terminate(const char *name);
struct process *app_get_process(const char *name);
bool app_is_running(const char *name);

/* MIME type handling */
int app_register_mime_type(const char *mime_type, const char *application);
const char *app_get_default_for_mime(const char *mime_type);
int app_open_file(const char *filename);
int app_open_url(const char *url);

/* File Manager */
file_manager_t *file_manager_create(void);
void file_manager_destroy(file_manager_t *fm);
int file_manager_navigate_to(file_manager_t *fm, const char *path);
int file_manager_go_back(file_manager_t *fm);
int file_manager_go_forward(file_manager_t *fm);
int file_manager_refresh(file_manager_t *fm);
int file_manager_copy_files(file_manager_t *fm, const char *paths[], int count);
int file_manager_move_files(file_manager_t *fm, const char *paths[], int count, const char *destination);
int file_manager_delete_files(file_manager_t *fm, const char *paths[], int count);
int file_manager_add_bookmark(file_manager_t *fm, const char *name, const char *path);

/* Web Browser */
web_browser_t *web_browser_create(void);
void web_browser_destroy(web_browser_t *browser);
int web_browser_navigate(web_browser_t *browser, const char *url);
int web_browser_new_tab(web_browser_t *browser, const char *url);
int web_browser_close_tab(web_browser_t *browser, int tab_index);
int web_browser_switch_tab(web_browser_t *browser, int tab_index);
int web_browser_add_bookmark(web_browser_t *browser, const char *title, const char *url);
int web_browser_download_file(web_browser_t *browser, const char *url, const char *filename);

/* Text Editor */
text_editor_t *text_editor_create(void);
void text_editor_destroy(text_editor_t *editor);
int text_editor_open_file(text_editor_t *editor, const char *filename);
int text_editor_save_file(text_editor_t *editor);
int text_editor_save_as(text_editor_t *editor, const char *filename);
int text_editor_new_document(text_editor_t *editor);
int text_editor_close_document(text_editor_t *editor, int document_index);
int text_editor_find_text(text_editor_t *editor, const char *text, bool case_sensitive);
int text_editor_replace_text(text_editor_t *editor, const char *find_text, const char *replace_text);

/* Terminal */
terminal_t *terminal_create(void);
void terminal_destroy(terminal_t *terminal);
int terminal_execute_command(terminal_t *terminal, const char *command);
int terminal_send_input(terminal_t *terminal, const char *input);
int terminal_new_tab(terminal_t *terminal);
int terminal_close_tab(terminal_t *terminal, int tab_index);
int terminal_clear_screen(terminal_t *terminal);
int terminal_copy_selection(terminal_t *terminal);
int terminal_paste_clipboard(terminal_t *terminal);

/* Media Player */
media_player_t *media_player_create(void);
void media_player_destroy(media_player_t *player);
int media_player_open_file(media_player_t *player, const char *filename);
int media_player_play(media_player_t *player);
int media_player_pause(media_player_t *player);
int media_player_stop(media_player_t *player);
int media_player_seek(media_player_t *player, uint64_t position_ms);
int media_player_set_volume(media_player_t *player, float volume);
int media_player_add_to_playlist(media_player_t *player, const char *filename);
int media_player_next_track(media_player_t *player);
int media_player_previous_track(media_player_t *player);

/* Image Viewer */
image_viewer_t *image_viewer_create(void);
void image_viewer_destroy(image_viewer_t *viewer);
int image_viewer_open_file(image_viewer_t *viewer, const char *filename);
int image_viewer_zoom_in(image_viewer_t *viewer);
int image_viewer_zoom_out(image_viewer_t *viewer);
int image_viewer_fit_to_window(image_viewer_t *viewer);
int image_viewer_rotate(image_viewer_t *viewer, int degrees);
int image_viewer_next_image(image_viewer_t *viewer);
int image_viewer_previous_image(image_viewer_t *viewer);
int image_viewer_start_slideshow(image_viewer_t *viewer, int interval_s);

/* Calculator */
calculator_t *calculator_create(void);
void calculator_destroy(calculator_t *calc);
int calculator_input_digit(calculator_t *calc, int digit);
int calculator_input_operation(calculator_t *calc, int operation);
int calculator_calculate(calculator_t *calc);
int calculator_clear(calculator_t *calc);
int calculator_clear_all(calculator_t *calc);
int calculator_memory_store(calculator_t *calc);
int calculator_memory_recall(calculator_t *calc);
int calculator_memory_clear(calculator_t *calc);

/* Settings */
settings_app_t *settings_create(void);
void settings_destroy(settings_app_t *settings);
int settings_get_value(const char *category, const char *key, char *value, size_t size);
int settings_set_value(const char *category, const char *key, const char *value);
int settings_apply_changes(void);
int settings_reset_to_defaults(const char *category);

/* App Store */
app_store_t *app_store_create(void);
void app_store_destroy(app_store_t *store);
int app_store_search(app_store_t *store, const char *query);
int app_store_install_app(app_store_t *store, const char *app_name);
int app_store_uninstall_app(app_store_t *store, const char *app_name);
int app_store_update_app(app_store_t *store, const char *app_name);
int app_store_check_updates(app_store_t *store);

/* Utility functions */
const char *app_type_name(app_type_t type);
int app_get_icon(const char *app_name, void **icon_data, size_t *icon_size);
int app_create_desktop_entry(const char *name, const char *executable, const char *icon);
int app_remove_desktop_entry(const char *name);

/* Application integration */
int app_send_notification(const char *title, const char *message, const char *icon);
int app_open_file_dialog(const char *title, const char *filter, char *selected_file, size_t size);
int app_open_save_dialog(const char *title, const char *default_name, char *selected_file, size_t size);
int app_show_message_box(const char *title, const char *message, int type);

/* Recent files */
int app_add_recent_file(const char *filename);
int app_get_recent_files(char files[][256], int max_files);
int app_clear_recent_files(void);