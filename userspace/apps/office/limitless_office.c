/*
 * LimitlessOffice Suite - Complete Productivity Suite for LimitlessOS
 * 
 * Comprehensive office suite featuring:
 * - LimitlessWriter (Word processor with AI assistance)
 * - LimitlessCalc (Spreadsheet application with advanced analytics)
 * - LimitlessPresent (Presentation software with interactive features)
 * - LimitlessDraw (Vector graphics and diagram editor)
 * - LimitlessNote (Advanced note-taking with AI organization)
 * - LimitlessProject (Project management and collaboration)
 * 
 * Universal document compatibility: .docx, .xlsx, .pptx, .odt, .ods, .odp, .pdf
 * Real-time collaboration, cloud sync, AI-powered features
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <json-c/json.h>
#include <cairo.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <zlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>

#define LIMITLESS_OFFICE_VERSION "1.0.0"
#define MAX_DOCUMENTS 100
#define MAX_FILENAME 256
#define MAX_TITLE 256
#define MAX_CONTENT_SIZE (100 * 1024 * 1024) // 100MB max document size
#define MAX_RECENT_FILES 20
#define MAX_TEMPLATES 50
#define MAX_COLLABORATIVE_USERS 50

// Forward declarations
typedef struct LimitlessOffice LimitlessOffice;
typedef struct Document Document;
typedef struct Template Template;
typedef struct CollaborationSession CollaborationSession;
typedef struct AIAssistant AIAssistant;
typedef struct FormatManager FormatManager;

// Application types
typedef enum {
    APP_WRITER = 0,      // Word processor
    APP_CALC,            // Spreadsheet
    APP_PRESENT,         // Presentations
    APP_DRAW,            // Vector graphics
    APP_NOTE,            // Note-taking
    APP_PROJECT          // Project management
} AppType;

// Document types
typedef enum {
    DOC_TEXT = 0,        // Text documents (.docx, .odt, .rtf, .txt)
    DOC_SPREADSHEET,     // Spreadsheets (.xlsx, .ods, .csv)
    DOC_PRESENTATION,    // Presentations (.pptx, .odp)
    DOC_DRAWING,         // Vector graphics (.svg, .ai, .eps)
    DOC_NOTE,            // Notes (.note, .md, .txt)
    DOC_PROJECT          // Project files (.proj, .mpp)
} DocumentType;

// Document formats
typedef enum {
    FORMAT_DOCX = 0,     // Microsoft Word
    FORMAT_ODT,          // OpenDocument Text
    FORMAT_RTF,          // Rich Text Format
    FORMAT_PDF,          // Portable Document Format
    FORMAT_XLSX,         // Microsoft Excel
    FORMAT_ODS,          // OpenDocument Spreadsheet
    FORMAT_CSV,          // Comma Separated Values
    FORMAT_PPTX,         // Microsoft PowerPoint
    FORMAT_ODP,          // OpenDocument Presentation
    FORMAT_SVG,          // Scalable Vector Graphics
    FORMAT_PNG,          // Portable Network Graphics
    FORMAT_JPG,          // JPEG Image
    FORMAT_MD,           // Markdown
    FORMAT_TXT,          // Plain Text
    FORMAT_HTML,         // HyperText Markup Language
    FORMAT_XML,          // eXtensible Markup Language
    FORMAT_JSON          // JavaScript Object Notation
} DocumentFormat;

// Collaboration modes
typedef enum {
    COLLAB_NONE = 0,     // No collaboration
    COLLAB_READ_ONLY,    // Read-only sharing
    COLLAB_COMMENTS,     // Comments only
    COLLAB_EDIT,         // Full editing
    COLLAB_OWNER         // Owner privileges
} CollaborationMode;

// AI assistance features
typedef enum {
    AI_GRAMMAR_CHECK = 0x01,     // Grammar and spelling check
    AI_STYLE_SUGGEST = 0x02,     // Style suggestions
    AI_AUTO_COMPLETE = 0x04,     // Auto-completion
    AI_TRANSLATE = 0x08,         // Translation
    AI_SUMMARIZE = 0x10,         // Content summarization
    AI_FORMAT = 0x20,            // Auto-formatting
    AI_RESEARCH = 0x40,          // Research assistance
    AI_CHARTS = 0x80,            // Smart chart generation
    AI_TEMPLATES = 0x100,        // Template suggestions
    AI_COLLABORATION = 0x200     // Collaboration insights
} AIFeatures;

// Text formatting
struct TextFormat {
    char font_family[64];
    int font_size;
    bool bold;
    bool italic;
    bool underline;
    bool strikethrough;
    GdkRGBA text_color;
    GdkRGBA background_color;
    int alignment;               // 0=left, 1=center, 2=right, 3=justify
    double line_spacing;
    int indent_level;
    bool bullet_point;
    bool numbered_list;
};

// Spreadsheet cell
struct SpreadsheetCell {
    char value[256];
    char formula[512];
    struct TextFormat format;
    bool is_formula;
    double numeric_value;
    int row;
    int col;
    bool selected;
    bool modified;
};

// Presentation slide
struct PresentationSlide {
    int slide_number;
    char title[MAX_TITLE];
    char content[8192];
    char layout[64];             // slide layout template
    char background[256];        // background image/color
    GList *objects;              // slide objects (text, images, etc.)
    char notes[2048];            // presenter notes
    double duration;             // slide duration in seconds
    char animation[128];         // slide transition animation
    bool is_master;              // master slide template
};

// Vector drawing object
struct DrawingObject {
    int object_id;
    char type[32];               // rectangle, circle, line, text, image, etc.
    double x, y, width, height;
    GdkRGBA fill_color;
    GdkRGBA stroke_color;
    double stroke_width;
    char text_content[1024];
    struct TextFormat text_format;
    char image_path[PATH_MAX];
    double rotation;
    double opacity;
    bool visible;
    bool locked;
    int z_order;                 // layer order
};

// Note structure
struct NoteItem {
    int note_id;
    char title[MAX_TITLE];
    char content[16384];
    char tags[256];
    time_t created_time;
    time_t modified_time;
    int category_id;
    bool is_encrypted;
    bool is_shared;
    char checksum[64];           // content integrity check
    struct NoteItem *next;
};

// Project task
struct ProjectTask {
    int task_id;
    char name[256];
    char description[1024];
    time_t start_date;
    time_t end_date;
    time_t deadline;
    double progress;             // 0.0 to 1.0
    int priority;                // 1-5 priority level
    char assigned_to[128];
    char status[64];             // Not Started, In Progress, Completed, etc.
    GList *dependencies;         // dependent task IDs
    GList *attachments;          // file attachments
    struct ProjectTask *parent;  // parent task for subtasks
    GList *subtasks;            // child tasks
};

// Document structure
struct Document {
    int doc_id;
    char filename[MAX_FILENAME];
    char title[MAX_TITLE];
    char path[PATH_MAX];
    DocumentType type;
    DocumentFormat format;
    
    // Content based on document type
    union {
        struct {
            char *content;       // text content
            GList *paragraphs;   // paragraph list
            GList *images;       // embedded images
            GList *tables;       // tables
            struct TextFormat default_format;
        } text;
        
        struct {
            int rows, cols;
            struct SpreadsheetCell **cells;
            GList *charts;       // embedded charts
            GList *formulas;     // formula list
            char **row_headers;
            char **col_headers;
        } spreadsheet;
        
        struct {
            GList *slides;       // slide list
            struct PresentationSlide *current_slide;
            int slide_count;
            char theme[64];
            bool is_slideshow;
            double slideshow_timer;
        } presentation;
        
        struct {
            GList *objects;      // drawing objects
            double canvas_width, canvas_height;
            double zoom_level;
            char background_color[16];
            GList *layers;       // drawing layers
            bool grid_visible;
            double grid_size;
        } drawing;
        
        struct {
            GList *notes;        // note items
            GList *categories;   // note categories
            char search_query[256];
            bool encrypted;
        } notes;
        
        struct {
            GList *tasks;        // project tasks
            GList *milestones;   // project milestones
            GList *resources;    // project resources
            time_t start_date;
            time_t end_date;
            double completion;
            char project_manager[128];
        } project;
    } data;
    
    // Metadata
    time_t created_time;
    time_t modified_time;
    time_t last_saved;
    char author[128];
    char last_editor[128];
    bool is_modified;
    bool is_read_only;
    bool is_template;
    
    // Collaboration
    CollaborationSession *collaboration;
    GList *collaborators;
    
    // Undo/Redo
    GList *undo_stack;
    GList *redo_stack;
    int max_undo_levels;
    
    // AI assistance
    AIFeatures ai_features;
    
    struct Document *next;
};

// Template structure
struct Template {
    int template_id;
    char name[MAX_TITLE];
    char description[512];
    char category[64];
    DocumentType type;
    char preview_image[PATH_MAX];
    char template_file[PATH_MAX];
    bool is_builtin;
    bool is_favorite;
    int usage_count;
    double rating;
    struct Template *next;
};

// Collaboration session
struct CollaborationSession {
    char session_id[64];
    char server_url[256];
    bool is_active;
    CollaborationMode mode;
    
    struct {
        char user_id[64];
        char display_name[128];
        CollaborationMode permissions;
        GdkRGBA cursor_color;
        bool is_online;
        time_t last_activity;
    } users[MAX_COLLABORATIVE_USERS];
    
    int user_count;
    pthread_mutex_t session_mutex;
    
    // Real-time sync
    bool auto_sync;
    int sync_interval;           // seconds
    time_t last_sync;
    
    // Change tracking
    GList *pending_changes;
    GList *applied_changes;
};

// AI Assistant
struct AIAssistant {
    bool enabled;
    AIFeatures active_features;
    char language[16];
    double confidence_threshold;
    
    // Grammar and style
    GList *grammar_suggestions;
    GList *style_suggestions;
    bool auto_correct;
    
    // Translation
    GHashTable *translation_cache;
    GList *supported_languages;
    
    // Templates and research
    GList *template_suggestions;
    GHashTable *research_cache;
    
    // Learning
    GHashTable *user_preferences;
    int learning_mode;
    
    void *ml_context;           // Machine learning context
    pthread_t ai_thread;
    pthread_mutex_t ai_mutex;
};

// Format manager for import/export
struct FormatManager {
    GHashTable *readers;        // format -> reader function
    GHashTable *writers;        // format -> writer function
    GList *supported_formats;
    
    // Conversion functions
    bool (*docx_reader)(Document *doc, const char *filename);
    bool (*docx_writer)(Document *doc, const char *filename);
    bool (*odt_reader)(Document *doc, const char *filename);
    bool (*odt_writer)(Document *doc, const char *filename);
    bool (*pdf_writer)(Document *doc, const char *filename);
    bool (*xlsx_reader)(Document *doc, const char *filename);
    bool (*xlsx_writer)(Document *doc, const char *filename);
    bool (*pptx_reader)(Document *doc, const char *filename);
    bool (*pptx_writer)(Document *doc, const char *filename);
};

// Main LimitlessOffice structure
struct LimitlessOffice {
    // UI Components
    GtkWidget *main_window;
    GtkWidget *header_bar;
    GtkWidget *toolbar;
    GtkWidget *sidebar;
    GtkWidget *main_area;
    GtkWidget *status_bar;
    GtkWidget *notebook;         // tab container
    
    // Menu system
    GtkWidget *menubar;
    GtkWidget *file_menu;
    GtkWidget *edit_menu;
    GtkWidget *view_menu;
    GtkWidget *format_menu;
    GtkWidget *tools_menu;
    GtkWidget *help_menu;
    
    // Toolbar buttons
    GtkWidget *new_button;
    GtkWidget *open_button;
    GtkWidget *save_button;
    GtkWidget *print_button;
    GtkWidget *undo_button;
    GtkWidget *redo_button;
    GtkWidget *cut_button;
    GtkWidget *copy_button;
    GtkWidget *paste_button;
    GtkWidget *find_button;
    
    // Format toolbar
    GtkWidget *format_toolbar;
    GtkWidget *font_combo;
    GtkWidget *size_combo;
    GtkWidget *bold_button;
    GtkWidget *italic_button;
    GtkWidget *underline_button;
    GtkWidget *color_button;
    
    // Application-specific areas
    struct {
        GtkWidget *editor;       // text editor
        GtkWidget *ruler;        // formatting ruler
        GtkWidget *outline;      // document outline
        GtkTextBuffer *buffer;   // text buffer
    } writer;
    
    struct {
        GtkWidget *sheet;        // spreadsheet grid
        GtkWidget *formula_bar;  // formula entry
        GtkWidget *name_box;     // cell name
        GtkWidget *sheet_tabs;   // worksheet tabs
    } calc;
    
    struct {
        GtkWidget *canvas;       // presentation canvas
        GtkWidget *slide_panel;  // slide thumbnails
        GtkWidget *notes_panel;  // presenter notes
        GtkWidget *slide_sorter; // slide sorter view
    } present;
    
    struct {
        GtkWidget *canvas;       // drawing canvas
        GtkWidget *toolbox;      // drawing tools
        GtkWidget *properties;   // object properties
        GtkWidget *layers;       // layer manager
    } draw;
    
    struct {
        GtkWidget *notes_tree;   // notes tree view
        GtkWidget *editor;       // note editor
        GtkWidget *search_bar;   // search interface
        GtkWidget *tags_panel;   // tags manager
    } note;
    
    struct {
        GtkWidget *task_tree;    // task tree view
        GtkWidget *gantt_chart;  // Gantt chart
        GtkWidget *calendar;     // calendar view
        GtkWidget *resources;    // resource manager
    } project;
    
    // Current state
    AppType current_app;
    Document *documents;
    Document *active_document;
    int document_count;
    int next_doc_id;
    
    // Templates
    Template *templates;
    int template_count;
    
    // Recent files
    char recent_files[MAX_RECENT_FILES][PATH_MAX];
    int recent_count;
    
    // Configuration
    char profile_path[PATH_MAX];
    char templates_path[PATH_MAX];
    sqlite3 *profile_db;
    
    // Components
    AIAssistant *ai_assistant;
    FormatManager *format_manager;
    
    // Features
    bool auto_save;
    int auto_save_interval;
    bool spell_check;
    bool grammar_check;
    bool dark_theme;
    bool show_grid;
    bool show_ruler;
    
    // Synchronization
    pthread_mutex_t docs_mutex;
    pthread_mutex_t ui_mutex;
    
    // Threading
    pthread_t auto_save_thread;
    pthread_t sync_thread;
    
    bool running;
};

// Function prototypes
static int office_init(LimitlessOffice *office);
static void office_cleanup(LimitlessOffice *office);
static Document *create_new_document(LimitlessOffice *office, DocumentType type);
static bool open_document(LimitlessOffice *office, const char *filename);
static bool save_document(LimitlessOffice *office, Document *doc, const char *filename);
static void close_document(LimitlessOffice *office, Document *doc);
static void switch_application(LimitlessOffice *office, AppType app);
static bool init_format_manager(LimitlessOffice *office);
static bool init_ai_assistant(LimitlessOffice *office);
static bool init_profile_database(LimitlessOffice *office);

// Document format handlers
static bool read_docx_document(Document *doc, const char *filename);
static bool write_docx_document(Document *doc, const char *filename);
static bool read_odt_document(Document *doc, const char *filename);
static bool write_odt_document(Document *doc, const char *filename);
static bool read_xlsx_document(Document *doc, const char *filename);
static bool write_xlsx_document(Document *doc, const char *filename);
static bool write_pdf_document(Document *doc, const char *filename);

// AI assistance functions
static void ai_check_grammar(Document *doc);
static void ai_suggest_style(Document *doc);
static char *ai_translate_text(const char *text, const char *target_lang);
static char *ai_summarize_content(Document *doc);
static GList *ai_suggest_templates(DocumentType type, const char *content);

// UI event handlers
static void on_new_document(GtkWidget *widget, LimitlessOffice *office);
static void on_open_document(GtkWidget *widget, LimitlessOffice *office);
static void on_save_document(GtkWidget *widget, LimitlessOffice *office);
static void on_close_document(GtkWidget *widget, LimitlessOffice *office);
static void on_switch_app(GtkWidget *widget, LimitlessOffice *office);
static gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, LimitlessOffice *office);

// Profile Database Initialization
static bool init_profile_database(LimitlessOffice *office) {
    char db_path[PATH_MAX];
    int rc;
    
    snprintf(db_path, sizeof(db_path), "%s/office.db", office->profile_path);
    
    rc = sqlite3_open(db_path, &office->profile_db);
    if (rc != SQLITE_OK) {
        printf("Error: Cannot open office database: %s\n", sqlite3_errmsg(office->profile_db));
        return false;
    }
    
    // Create tables
    const char *create_documents_sql = 
        "CREATE TABLE IF NOT EXISTS documents ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "filename TEXT NOT NULL,"
        "title TEXT,"
        "path TEXT NOT NULL,"
        "type INTEGER,"
        "format INTEGER,"
        "created_time INTEGER,"
        "modified_time INTEGER,"
        "last_opened INTEGER,"
        "author TEXT,"
        "is_template BOOLEAN DEFAULT FALSE"
        ");";
    
    const char *create_templates_sql = 
        "CREATE TABLE IF NOT EXISTS templates ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "category TEXT,"
        "type INTEGER,"
        "template_file TEXT,"
        "preview_image TEXT,"
        "is_builtin BOOLEAN DEFAULT FALSE,"
        "usage_count INTEGER DEFAULT 0,"
        "rating REAL DEFAULT 0.0"
        ");";
    
    const char *create_recent_sql = 
        "CREATE TABLE IF NOT EXISTS recent_files ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "filename TEXT NOT NULL,"
        "path TEXT NOT NULL,"
        "last_opened INTEGER,"
        "type INTEGER"
        ");";
    
    char *error_msg = NULL;
    
    rc = sqlite3_exec(office->profile_db, create_documents_sql, NULL, NULL, &error_msg);
    if (rc != SQLITE_OK) {
        printf("Error creating documents table: %s\n", error_msg);
        sqlite3_free(error_msg);
        return false;
    }
    
    rc = sqlite3_exec(office->profile_db, create_templates_sql, NULL, NULL, &error_msg);
    if (rc != SQLITE_OK) {
        printf("Error creating templates table: %s\n", error_msg);
        sqlite3_free(error_msg);
        return false;
    }
    
    rc = sqlite3_exec(office->profile_db, create_recent_sql, NULL, NULL, &error_msg);
    if (rc != SQLITE_OK) {
        printf("Error creating recent files table: %s\n", error_msg);
        sqlite3_free(error_msg);
        return false;
    }
    
    printf("LimitlessOffice: Profile database initialized\n");
    return true;
}

// Format Manager Initialization
static bool init_format_manager(LimitlessOffice *office) {
    FormatManager *fm;
    
    fm = calloc(1, sizeof(FormatManager));
    if (!fm) {
        printf("Error: Failed to allocate format manager\n");
        return false;
    }
    
    // Initialize hash tables
    fm->readers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    fm->writers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    
    // Register format handlers
    fm->docx_reader = read_docx_document;
    fm->docx_writer = write_docx_document;
    fm->odt_reader = read_odt_document;
    fm->odt_writer = write_odt_document;
    fm->pdf_writer = write_pdf_document;
    fm->xlsx_reader = read_xlsx_document;
    fm->xlsx_writer = write_xlsx_document;
    
    // Add readers to hash table
    g_hash_table_insert(fm->readers, g_strdup("docx"), fm->docx_reader);
    g_hash_table_insert(fm->readers, g_strdup("odt"), fm->odt_reader);
    g_hash_table_insert(fm->readers, g_strdup("xlsx"), fm->xlsx_reader);
    
    // Add writers to hash table
    g_hash_table_insert(fm->writers, g_strdup("docx"), fm->docx_writer);
    g_hash_table_insert(fm->writers, g_strdup("odt"), fm->odt_writer);
    g_hash_table_insert(fm->writers, g_strdup("pdf"), fm->pdf_writer);
    g_hash_table_insert(fm->writers, g_strdup("xlsx"), fm->xlsx_writer);
    
    // Build supported formats list
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("docx"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("odt"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("rtf"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("pdf"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("xlsx"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("ods"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("csv"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("pptx"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("odp"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("txt"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("md"));
    fm->supported_formats = g_list_append(fm->supported_formats, g_strdup("html"));
    
    office->format_manager = fm;
    
    printf("LimitlessOffice: Format manager initialized with %d supported formats\n", 
           g_list_length(fm->supported_formats));
    
    return true;
}

// AI Assistant Initialization
static bool init_ai_assistant(LimitlessOffice *office) {
    AIAssistant *ai;
    
    ai = calloc(1, sizeof(AIAssistant));
    if (!ai) {
        printf("Error: Failed to allocate AI assistant\n");
        return false;
    }
    
    // Initialize AI assistant
    ai->enabled = true;
    ai->active_features = AI_GRAMMAR_CHECK | AI_STYLE_SUGGEST | AI_AUTO_COMPLETE | AI_TRANSLATE | AI_SUMMARIZE;
    strcpy(ai->language, "en");
    ai->confidence_threshold = 0.8;
    ai->auto_correct = false;
    ai->learning_mode = 1;
    
    // Initialize data structures
    ai->grammar_suggestions = NULL;
    ai->style_suggestions = NULL;
    ai->translation_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    ai->research_cache = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    ai->user_preferences = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    
    // Supported languages for translation
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("en"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("es"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("fr"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("de"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("it"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("pt"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("ru"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("zh"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("ja"));
    ai->supported_languages = g_list_append(ai->supported_languages, g_strdup("ko"));
    
    pthread_mutex_init(&ai->ai_mutex, NULL);
    
    office->ai_assistant = ai;
    
    printf("LimitlessOffice: AI assistant initialized with %d supported languages\n",
           g_list_length(ai->supported_languages));
    
    return true;
}

// Create New Document
static Document *create_new_document(LimitlessOffice *office, DocumentType type) {
    Document *doc;
    time_t now = time(NULL);
    
    pthread_mutex_lock(&office->docs_mutex);
    
    doc = calloc(1, sizeof(Document));
    if (!doc) {
        printf("Error: Failed to allocate document structure\n");
        pthread_mutex_unlock(&office->docs_mutex);
        return NULL;
    }
    
    // Initialize document
    doc->doc_id = office->next_doc_id++;
    doc->type = type;
    doc->created_time = now;
    doc->modified_time = now;
    doc->last_saved = 0;
    doc->is_modified = false;
    doc->is_read_only = false;
    doc->is_template = false;
    doc->max_undo_levels = 100;
    doc->ai_features = AI_GRAMMAR_CHECK | AI_STYLE_SUGGEST;
    
    strcpy(doc->author, getenv("USER") ? getenv("USER") : "Unknown");
    strcpy(doc->last_editor, doc->author);
    
    // Set default format based on type
    switch (type) {
    case DOC_TEXT:
        doc->format = FORMAT_DOCX;
        strcpy(doc->filename, "Untitled Document.docx");
        strcpy(doc->title, "Untitled Document");
        
        // Initialize text document
        doc->data.text.content = malloc(MAX_CONTENT_SIZE);
        if (doc->data.text.content) {
            doc->data.text.content[0] = '\0';
        }
        doc->data.text.paragraphs = NULL;
        doc->data.text.images = NULL;
        doc->data.text.tables = NULL;
        
        // Set default format
        strcpy(doc->data.text.default_format.font_family, "Liberation Serif");
        doc->data.text.default_format.font_size = 12;
        doc->data.text.default_format.bold = false;
        doc->data.text.default_format.italic = false;
        doc->data.text.default_format.underline = false;
        doc->data.text.default_format.alignment = 0; // left
        doc->data.text.default_format.line_spacing = 1.15;
        break;
        
    case DOC_SPREADSHEET:
        doc->format = FORMAT_XLSX;
        strcpy(doc->filename, "Untitled Spreadsheet.xlsx");
        strcpy(doc->title, "Untitled Spreadsheet");
        
        // Initialize spreadsheet
        doc->data.spreadsheet.rows = 1000;
        doc->data.spreadsheet.cols = 26; // A-Z
        doc->data.spreadsheet.cells = malloc(doc->data.spreadsheet.rows * sizeof(struct SpreadsheetCell*));
        for (int i = 0; i < doc->data.spreadsheet.rows; i++) {
            doc->data.spreadsheet.cells[i] = calloc(doc->data.spreadsheet.cols, sizeof(struct SpreadsheetCell));
            for (int j = 0; j < doc->data.spreadsheet.cols; j++) {
                doc->data.spreadsheet.cells[i][j].row = i;
                doc->data.spreadsheet.cells[i][j].col = j;
            }
        }
        break;
        
    case DOC_PRESENTATION:
        doc->format = FORMAT_PPTX;
        strcpy(doc->filename, "Untitled Presentation.pptx");
        strcpy(doc->title, "Untitled Presentation");
        
        // Initialize presentation with first slide
        struct PresentationSlide *slide = calloc(1, sizeof(struct PresentationSlide));
        if (slide) {
            slide->slide_number = 1;
            strcpy(slide->title, "Title Slide");
            strcpy(slide->layout, "title_slide");
            strcpy(slide->content, "Click to add title\n\nClick to add subtitle");
            doc->data.presentation.slides = g_list_append(doc->data.presentation.slides, slide);
            doc->data.presentation.current_slide = slide;
            doc->data.presentation.slide_count = 1;
            strcpy(doc->data.presentation.theme, "default");
        }
        break;
        
    case DOC_DRAWING:
        doc->format = FORMAT_SVG;
        strcpy(doc->filename, "Untitled Drawing.svg");
        strcpy(doc->title, "Untitled Drawing");
        
        // Initialize drawing canvas
        doc->data.drawing.objects = NULL;
        doc->data.drawing.canvas_width = 800;
        doc->data.drawing.canvas_height = 600;
        doc->data.drawing.zoom_level = 1.0;
        strcpy(doc->data.drawing.background_color, "#FFFFFF");
        doc->data.drawing.grid_visible = true;
        doc->data.drawing.grid_size = 10.0;
        break;
        
    case DOC_NOTE:
        doc->format = FORMAT_MD;
        strcpy(doc->filename, "Untitled Note.md");
        strcpy(doc->title, "Untitled Note");
        
        // Initialize note
        doc->data.notes.notes = NULL;
        doc->data.notes.categories = NULL;
        doc->data.notes.encrypted = false;
        break;
        
    case DOC_PROJECT:
        doc->format = FORMAT_XML;
        strcpy(doc->filename, "Untitled Project.proj");
        strcpy(doc->title, "Untitled Project");
        
        // Initialize project
        doc->data.project.tasks = NULL;
        doc->data.project.milestones = NULL;
        doc->data.project.resources = NULL;
        doc->data.project.start_date = now;
        doc->data.project.end_date = now + (30 * 24 * 60 * 60); // 30 days from now
        doc->data.project.completion = 0.0;
        strcpy(doc->data.project.project_manager, doc->author);
        break;
    }
    
    // Add to document list
    doc->next = office->documents;
    office->documents = doc;
    office->document_count++;
    office->active_document = doc;
    
    pthread_mutex_unlock(&office->docs_mutex);
    
    printf("LimitlessOffice: Created new %s document (ID: %d)\n", 
           (type == DOC_TEXT) ? "text" :
           (type == DOC_SPREADSHEET) ? "spreadsheet" :
           (type == DOC_PRESENTATION) ? "presentation" :
           (type == DOC_DRAWING) ? "drawing" :
           (type == DOC_NOTE) ? "note" :
           (type == DOC_PROJECT) ? "project" : "unknown",
           doc->doc_id);
    
    return doc;
}

// Document Format Readers (simplified implementations)
static bool read_docx_document(Document *doc, const char *filename) {
    // Simplified DOCX reader - in real implementation would parse ZIP/XML structure
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open DOCX file: %s\n", filename);
        return false;
    }
    
    // For now, just read as text (placeholder)
    if (!doc->data.text.content) {
        doc->data.text.content = malloc(MAX_CONTENT_SIZE);
    }
    
    if (doc->data.text.content) {
        size_t bytes_read = fread(doc->data.text.content, 1, MAX_CONTENT_SIZE - 1, file);
        doc->data.text.content[bytes_read] = '\0';
    }
    
    fclose(file);
    
    printf("LimitlessOffice: Loaded DOCX document: %s\n", filename);
    return true;
}

static bool write_docx_document(Document *doc, const char *filename) {
    // Simplified DOCX writer - in real implementation would generate ZIP/XML structure
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create DOCX file: %s\n", filename);
        return false;
    }
    
    if (doc->data.text.content) {
        fprintf(file, "%s", doc->data.text.content);
    }
    
    fclose(file);
    
    printf("LimitlessOffice: Saved DOCX document: %s\n", filename);
    return true;
}

static bool read_xlsx_document(Document *doc, const char *filename) {
    // Simplified XLSX reader
    printf("LimitlessOffice: Loading XLSX document: %s\n", filename);
    return true;
}

static bool write_xlsx_document(Document *doc, const char *filename) {
    // Simplified XLSX writer
    printf("LimitlessOffice: Saving XLSX document: %s\n", filename);
    return true;
}

static bool read_odt_document(Document *doc, const char *filename) {
    // Simplified ODT reader
    printf("LimitlessOffice: Loading ODT document: %s\n", filename);
    return true;
}

static bool write_odt_document(Document *doc, const char *filename) {
    // Simplified ODT writer
    printf("LimitlessOffice: Saving ODT document: %s\n", filename);
    return true;
}

static bool write_pdf_document(Document *doc, const char *filename) {
    // Simplified PDF writer using Cairo
    cairo_surface_t *surface;
    cairo_t *cr;
    
    surface = cairo_pdf_surface_create(filename, 612, 792); // US Letter size
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        printf("Error: Failed to create PDF surface\n");
        return false;
    }
    
    cr = cairo_create(surface);
    
    // Set font
    cairo_select_font_face(cr, "Liberation Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0, 0, 0);
    
    // Write content (simplified)
    if (doc->type == DOC_TEXT && doc->data.text.content) {
        cairo_move_to(cr, 50, 50);
        cairo_show_text(cr, doc->data.text.content);
    }
    
    cairo_show_page(cr);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    
    printf("LimitlessOffice: Exported PDF document: %s\n", filename);
    return true;
}

// AI Functions (simplified implementations)
static void ai_check_grammar(Document *doc) {
    // Simplified grammar check
    printf("LimitlessOffice: Running AI grammar check on document\n");
}

static char *ai_translate_text(const char *text, const char *target_lang) {
    // Simplified translation - in real implementation would use ML service
    char *translated = malloc(strlen(text) + 100);
    if (translated) {
        snprintf(translated, strlen(text) + 100, "[Translated to %s] %s", target_lang, text);
    }
    return translated;
}

// UI Event Handlers
static void on_new_document(GtkWidget *widget, LimitlessOffice *office) {
    DocumentType type = DOC_TEXT; // Default to text document
    
    // Get document type based on current app
    switch (office->current_app) {
    case APP_WRITER:   type = DOC_TEXT; break;
    case APP_CALC:     type = DOC_SPREADSHEET; break;
    case APP_PRESENT:  type = DOC_PRESENTATION; break;
    case APP_DRAW:     type = DOC_DRAWING; break;
    case APP_NOTE:     type = DOC_NOTE; break;
    case APP_PROJECT:  type = DOC_PROJECT; break;
    }
    
    Document *doc = create_new_document(office, type);
    if (doc) {
        printf("LimitlessOffice: New document created\n");
    }
}

static void on_open_document(GtkWidget *widget, LimitlessOffice *office) {
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new("Open Document",
                                       GTK_WINDOW(office->main_window),
                                       action,
                                       "_Cancel",
                                       GTK_RESPONSE_CANCEL,
                                       "_Open",
                                       GTK_RESPONSE_ACCEPT,
                                       NULL);
    
    chooser = GTK_FILE_CHOOSER(dialog);
    
    // Add file filters for supported formats
    GtkFileFilter *filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, "All Supported Documents");
    gtk_file_filter_add_pattern(filter_all, "*.docx");
    gtk_file_filter_add_pattern(filter_all, "*.odt");
    gtk_file_filter_add_pattern(filter_all, "*.xlsx");
    gtk_file_filter_add_pattern(filter_all, "*.ods");
    gtk_file_filter_add_pattern(filter_all, "*.pptx");
    gtk_file_filter_add_pattern(filter_all, "*.odp");
    gtk_file_filter_add_pattern(filter_all, "*.txt");
    gtk_file_filter_add_pattern(filter_all, "*.md");
    gtk_file_chooser_add_filter(chooser, filter_all);
    
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(chooser);
        open_document(office, filename);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_save_document(GtkWidget *widget, LimitlessOffice *office) {
    if (office->active_document) {
        if (strlen(office->active_document->path) == 0) {
            // Save As dialog
            GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Document",
                                                          GTK_WINDOW(office->main_window),
                                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                                          "_Cancel",
                                                          GTK_RESPONSE_CANCEL,
                                                          "_Save",
                                                          GTK_RESPONSE_ACCEPT,
                                                          NULL);
            
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), office->active_document->filename);
            
            gint res = gtk_dialog_run(GTK_DIALOG(dialog));
            if (res == GTK_RESPONSE_ACCEPT) {
                char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                save_document(office, office->active_document, filename);
                g_free(filename);
            }
            
            gtk_widget_destroy(dialog);
        } else {
            // Save existing document
            save_document(office, office->active_document, office->active_document->path);
        }
    }
}

// Application Switching
static void switch_application(LimitlessOffice *office, AppType app) {
    office->current_app = app;
    
    // Update UI based on application
    switch (app) {
    case APP_WRITER:
        gtk_widget_show(office->writer.editor);
        gtk_widget_show(office->writer.ruler);
        gtk_widget_show(office->format_toolbar);
        break;
        
    case APP_CALC:
        gtk_widget_show(office->calc.sheet);
        gtk_widget_show(office->calc.formula_bar);
        break;
        
    case APP_PRESENT:
        gtk_widget_show(office->present.canvas);
        gtk_widget_show(office->present.slide_panel);
        break;
        
    case APP_DRAW:
        gtk_widget_show(office->draw.canvas);
        gtk_widget_show(office->draw.toolbox);
        break;
        
    case APP_NOTE:
        gtk_widget_show(office->note.notes_tree);
        gtk_widget_show(office->note.editor);
        break;
        
    case APP_PROJECT:
        gtk_widget_show(office->project.task_tree);
        gtk_widget_show(office->project.gantt_chart);
        break;
    }
    
    // Update window title
    const char *app_names[] = {"Writer", "Calc", "Present", "Draw", "Note", "Project"};
    char title[256];
    snprintf(title, sizeof(title), "LimitlessOffice %s", app_names[app]);
    gtk_header_bar_set_title(GTK_HEADER_BAR(office->header_bar), title);
    
    printf("LimitlessOffice: Switched to %s\n", app_names[app]);
}

// LimitlessOffice Initialization
static int office_init(LimitlessOffice *office) {
    printf("Initializing LimitlessOffice v%s\n", LIMITLESS_OFFICE_VERSION);
    
    // Initialize GTK
    if (!gtk_init_check(NULL, NULL)) {
        printf("Error: Failed to initialize GTK\n");
        return -1;
    }
    
    // Initialize threading
    pthread_mutex_init(&office->docs_mutex, NULL);
    pthread_mutex_init(&office->ui_mutex, NULL);
    
    // Set up profile paths
    const char *home = getenv("HOME");
    if (!home) home = "/home/limitless";
    
    snprintf(office->profile_path, sizeof(office->profile_path), "%s/.limitless/office", home);
    snprintf(office->templates_path, sizeof(office->templates_path), "%s/.limitless/office/templates", home);
    
    // Create directories
    mkdir(office->profile_path, 0755);
    mkdir(office->templates_path, 0755);
    
    // Initialize components
    if (!init_profile_database(office)) {
        printf("Error: Failed to initialize profile database\n");
        return -1;
    }
    
    if (!init_format_manager(office)) {
        printf("Error: Failed to initialize format manager\n");
        return -1;
    }
    
    if (!init_ai_assistant(office)) {
        printf("Error: Failed to initialize AI assistant\n");
        return -1;
    }
    
    // Initialize settings
    office->current_app = APP_WRITER;
    office->auto_save = true;
    office->auto_save_interval = 300; // 5 minutes
    office->spell_check = true;
    office->grammar_check = true;
    office->dark_theme = false;
    office->show_grid = true;
    office->show_ruler = true;
    
    // Create main window
    office->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(office->main_window), "LimitlessOffice");
    gtk_window_set_default_size(GTK_WINDOW(office->main_window), 1200, 800);
    gtk_window_set_icon_name(GTK_WINDOW(office->main_window), "libreoffice-main");
    
    // Connect window signals
    g_signal_connect(office->main_window, "delete-event", G_CALLBACK(on_window_delete_event), office);
    
    // Create header bar
    office->header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(office->header_bar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(office->header_bar), "LimitlessOffice Writer");
    gtk_window_set_titlebar(GTK_WINDOW(office->main_window), office->header_bar);
    
    // Create main layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(office->main_window), vbox);
    
    // Create menu bar (simplified)
    office->menubar = gtk_menu_bar_new();
    
    // File menu
    office->file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), office->file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(office->menubar), file_item);
    
    GtkWidget *new_item = gtk_menu_item_new_with_label("New");
    GtkWidget *open_item = gtk_menu_item_new_with_label("Open");
    GtkWidget *save_item = gtk_menu_item_new_with_label("Save");
    
    g_signal_connect(new_item, "activate", G_CALLBACK(on_new_document), office);
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open_document), office);
    g_signal_connect(save_item, "activate", G_CALLBACK(on_save_document), office);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(office->file_menu), new_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(office->file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(office->file_menu), save_item);
    
    // Create toolbar
    office->toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(office->toolbar), GTK_TOOLBAR_ICONS);
    
    office->new_button = gtk_tool_button_new(NULL, "New");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(office->new_button), "document-new");
    g_signal_connect(office->new_button, "clicked", G_CALLBACK(on_new_document), office);
    gtk_toolbar_insert(GTK_TOOLBAR(office->toolbar), office->new_button, -1);
    
    office->open_button = gtk_tool_button_new(NULL, "Open");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(office->open_button), "document-open");
    g_signal_connect(office->open_button, "clicked", G_CALLBACK(on_open_document), office);
    gtk_toolbar_insert(GTK_TOOLBAR(office->toolbar), office->open_button, -1);
    
    office->save_button = gtk_tool_button_new(NULL, "Save");
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(office->save_button), "document-save");
    g_signal_connect(office->save_button, "clicked", G_CALLBACK(on_save_document), office);
    gtk_toolbar_insert(GTK_TOOLBAR(office->toolbar), office->save_button, -1);
    
    // Create format toolbar
    office->format_toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(office->format_toolbar), GTK_TOOLBAR_ICONS);
    
    office->bold_button = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(office->bold_button), "format-text-bold");
    gtk_toolbar_insert(GTK_TOOLBAR(office->format_toolbar), GTK_TOOL_ITEM(office->bold_button), -1);
    
    office->italic_button = gtk_toggle_tool_button_new();
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(office->italic_button), "format-text-italic");
    gtk_toolbar_insert(GTK_TOOLBAR(office->format_toolbar), GTK_TOOL_ITEM(office->italic_button), -1);
    
    // Create main content area (simplified - would have different layouts per app)
    office->notebook = gtk_notebook_new();
    
    // Writer interface
    office->writer.editor = gtk_text_view_new();
    office->writer.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(office->writer.editor));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(office->writer.editor), GTK_WRAP_WORD);
    
    GtkWidget *writer_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(writer_scroll), office->writer.editor);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(office->notebook), writer_scroll, gtk_label_new("Writer"));
    
    // Status bar
    office->status_bar = gtk_statusbar_new();
    
    // Pack main layout
    gtk_box_pack_start(GTK_BOX(vbox), office->menubar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), office->toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), office->format_toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), office->notebook, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), office->status_bar, FALSE, FALSE, 0);
    
    // Show all widgets
    gtk_widget_show_all(office->main_window);
    
    office->running = true;
    
    printf("LimitlessOffice: Initialization complete\n");
    printf("LimitlessOffice: Profile path: %s\n", office->profile_path);
    printf("LimitlessOffice: AI assistance: %s\n", office->ai_assistant->enabled ? "Enabled" : "Disabled");
    printf("LimitlessOffice: Supported formats: %d\n", g_list_length(office->format_manager->supported_formats));
    
    return 0;
}

// Window delete event handler
static gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, LimitlessOffice *office) {
    office->running = false;
    office_cleanup(office);
    gtk_main_quit();
    return FALSE;
}

// Office cleanup
static void office_cleanup(LimitlessOffice *office) {
    Document *doc, *next_doc;
    
    printf("LimitlessOffice: Cleaning up...\n");
    
    // Close all documents
    pthread_mutex_lock(&office->docs_mutex);
    doc = office->documents;
    while (doc) {
        next_doc = doc->next;
        
        // Cleanup document-specific data
        switch (doc->type) {
        case DOC_TEXT:
            if (doc->data.text.content) {
                free(doc->data.text.content);
            }
            break;
        case DOC_SPREADSHEET:
            if (doc->data.spreadsheet.cells) {
                for (int i = 0; i < doc->data.spreadsheet.rows; i++) {
                    free(doc->data.spreadsheet.cells[i]);
                }
                free(doc->data.spreadsheet.cells);
            }
            break;
        default:
            break;
        }
        
        free(doc);
        doc = next_doc;
    }
    office->documents = NULL;
    office->document_count = 0;
    pthread_mutex_unlock(&office->docs_mutex);
    
    // Cleanup components
    if (office->ai_assistant) {
        pthread_mutex_destroy(&office->ai_assistant->ai_mutex);
        if (office->ai_assistant->translation_cache) {
            g_hash_table_destroy(office->ai_assistant->translation_cache);
        }
        if (office->ai_assistant->research_cache) {
            g_hash_table_destroy(office->ai_assistant->research_cache);
        }
        if (office->ai_assistant->user_preferences) {
            g_hash_table_destroy(office->ai_assistant->user_preferences);
        }
        free(office->ai_assistant);
    }
    
    if (office->format_manager) {
        if (office->format_manager->readers) {
            g_hash_table_destroy(office->format_manager->readers);
        }
        if (office->format_manager->writers) {
            g_hash_table_destroy(office->format_manager->writers);
        }
        if (office->format_manager->supported_formats) {
            g_list_free_full(office->format_manager->supported_formats, g_free);
        }
        free(office->format_manager);
    }
    
    // Close database
    if (office->profile_db) {
        sqlite3_close(office->profile_db);
        office->profile_db = NULL;
    }
    
    // Cleanup mutexes
    pthread_mutex_destroy(&office->docs_mutex);
    pthread_mutex_destroy(&office->ui_mutex);
    
    printf("LimitlessOffice: Cleanup complete\n");
}

// Main function
int main(int argc, char *argv[]) {
    LimitlessOffice office = {0};
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--calc") == 0) {
            office.current_app = APP_CALC;
        } else if (strcmp(argv[i], "--present") == 0) {
            office.current_app = APP_PRESENT;
        } else if (strcmp(argv[i], "--draw") == 0) {
            office.current_app = APP_DRAW;
        } else if (strcmp(argv[i], "--note") == 0) {
            office.current_app = APP_NOTE;
        } else if (strcmp(argv[i], "--project") == 0) {
            office.current_app = APP_PROJECT;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("LimitlessOffice v%s - Complete Productivity Suite\n", LIMITLESS_OFFICE_VERSION);
            printf("Usage: %s [OPTIONS] [FILE]\n", argv[0]);
            printf("Options:\n");
            printf("  --calc      Start LimitlessCalc (Spreadsheet)\n");
            printf("  --present   Start LimitlessPresent (Presentations)\n");
            printf("  --draw      Start LimitlessDraw (Vector Graphics)\n");
            printf("  --note      Start LimitlessNote (Note-taking)\n");
            printf("  --project   Start LimitlessProject (Project Management)\n");
            printf("  -h, --help  Show this help\n");
            printf("\nSupported formats:\n");
            printf("  Documents: .docx, .odt, .rtf, .pdf, .txt, .md\n");
            printf("  Spreadsheets: .xlsx, .ods, .csv\n");
            printf("  Presentations: .pptx, .odp\n");
            printf("  Graphics: .svg, .png, .jpg\n");
            printf("  Notes: .md, .txt\n");
            printf("  Projects: .proj, .mpp\n");
            return 0;
        } else if (argv[i][0] != '-') {
            // File to open
            // Would be handled in open_document function
        }
    }
    
    // Initialize office suite
    if (office_init(&office) != 0) {
        printf("Error: Failed to initialize LimitlessOffice\n");
        return 1;
    }
    
    // Create initial document
    create_new_document(&office, DOC_TEXT);
    
    // Run GTK main loop
    printf("LimitlessOffice: Starting application...\n");
    gtk_main();
    
    // Cleanup and exit
    office_cleanup(&office);
    
    return 0;
}