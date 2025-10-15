/**
 * Limitless Browser - Next-Generation Web Browser with AI Integration
 * 
 * Revolutionary web browser that merges OS navigation, file access, and search
 * into one intelligent interface. Built from first principles for LimitlessOS
 * with native AI integration, system-wide privacy enforcement, and military-grade security.
 * 
 * Inspired by the speed and intelligence of modern browsers but redesigned
 * for the LimitlessOS ecosystem with unprecedented integration and capabilities.
 */

#include "../ui/limitlessui.h"
#include "../desktop/limitless_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>   // For HTTP requests
#include <json-c/json.h> // For JSON parsing
#include <ctype.h>

// ============================================================================
// BROWSER CONSTANTS AND ARCHITECTURE
// ============================================================================

#define BROWSER_VERSION             "1.0.0-Comet"
#define BROWSER_USER_AGENT         "LimitlessBrowser/1.0 (LimitlessOS; AI-Enhanced) WebKit/537.36"
#define MAX_URL_LENGTH             2048
#define MAX_TITLE_LENGTH           256
#define MAX_SEARCH_QUERY_LENGTH    512
#define MAX_TABS                   50
#define MAX_HISTORY_ENTRIES        10000
#define MAX_BOOKMARKS             1000
#define MAX_DOWNLOADS              100
#define RENDER_CACHE_SIZE_MB       512
#define AI_CONFIDENCE_THRESHOLD    0.8f

// Browser engine types for backend flexibility
typedef enum {
    ENGINE_LIMITLESS_NATIVE = 0,      // Native LimitlessOS renderer
    ENGINE_WEBKIT_HYBRID,             // WebKit integration layer
    ENGINE_BLINK_COMPATIBLE,          // Blink compatibility mode
    ENGINE_TEXT_FALLBACK              // Text-only fallback mode
} browser_engine_t;

// Security modes for different browsing contexts
typedef enum {
    SECURITY_MODE_STANDARD = 0,       // Standard web browsing
    SECURITY_MODE_PRIVATE,            // Private/incognito mode
    SECURITY_MODE_SECURE,             // Enhanced security mode
    SECURITY_MODE_ISOLATED,           // Maximum isolation sandbox
    SECURITY_MODE_TOR                 // Tor network routing
} security_mode_t;

// AI assistance modes
typedef enum {
    AI_ASSIST_DISABLED = 0,           // No AI assistance
    AI_ASSIST_MINIMAL,                // Basic suggestions only
    AI_ASSIST_STANDARD,               // Standard AI features
    AI_ASSIST_ENHANCED,               // Advanced AI integration
    AI_ASSIST_COPILOT                 // Full AI copilot mode
} ai_assistance_t;

// Page loading states
typedef enum {
    PAGE_STATE_IDLE = 0,
    PAGE_STATE_LOADING,
    PAGE_STATE_LOADED,
    PAGE_STATE_ERROR,
    PAGE_STATE_CACHED
} page_state_t;

// ============================================================================
// CORE DATA STRUCTURES
// ============================================================================

// HTTP response structure
typedef struct http_response {
    char* data;
    size_t size;
    long status_code;
    char content_type[128];
    char* headers;
    size_t header_size;
} http_response_t;

// Web page content
typedef struct web_page {
    char url[MAX_URL_LENGTH];
    char title[MAX_TITLE_LENGTH];
    char* html_content;
    size_t content_size;
    char* rendered_content;        // Processed/rendered content
    
    // Page metadata
    page_state_t state;
    time_t load_time;
    time_t last_accessed;
    uint32_t visit_count;
    
    // Security information
    bool is_secure;                // HTTPS connection
    bool certificate_valid;
    char certificate_info[256];
    
    // AI analysis
    float quality_score;           // AI-assessed page quality (0.0-1.0)
    float relevance_score;         // Relevance to user interests
    char ai_summary[512];          // AI-generated summary
    char topics[256];              // Extracted topics/tags
    
    // Performance metrics
    uint32_t load_time_ms;
    uint32_t render_time_ms;
    uint64_t memory_usage_bytes;
    
    struct web_page* next;
    struct web_page* prev;
} web_page_t;

// Browser tab
typedef struct browser_tab {
    uint32_t id;
    char title[MAX_TITLE_LENGTH];
    web_page_t* current_page;
    web_page_t* history;           // Navigation history for this tab
    uint32_t history_position;
    uint32_t history_count;
    
    // Tab state
    bool is_active;
    bool is_loading;
    bool is_muted;
    bool is_pinned;
    bool is_private;
    security_mode_t security_mode;
    
    // Visual representation
    uint32_t favicon_id;
    lui_color_t accent_color;
    float loading_progress;        // 0.0 - 1.0
    
    // AI features
    bool ai_reading_mode;
    bool ai_translate_enabled;
    char ai_detected_language[16];
    
    struct browser_tab* next;
    struct browser_tab* prev;
} browser_tab_t;

// Bookmark entry
typedef struct bookmark {
    char title[MAX_TITLE_LENGTH];
    char url[MAX_URL_LENGTH];
    char description[256];
    char tags[128];
    time_t created_time;
    time_t last_accessed;
    uint32_t visit_count;
    uint32_t folder_id;
    
    // AI metadata
    float relevance_score;
    char ai_category[32];
    
    struct bookmark* next;
} bookmark_t;

// Download item
typedef struct download_item {
    char filename[256];
    char url[MAX_URL_LENGTH];
    char local_path[512];
    uint64_t total_size;
    uint64_t downloaded_size;
    time_t start_time;
    time_t completion_time;
    
    enum {
        DOWNLOAD_PENDING = 0,
        DOWNLOAD_ACTIVE,
        DOWNLOAD_PAUSED,
        DOWNLOAD_COMPLETED,
        DOWNLOAD_FAILED,
        DOWNLOAD_CANCELLED
    } state;
    
    float progress;                // 0.0 - 1.0
    uint32_t speed_kbps;
    time_t estimated_completion;
    
    struct download_item* next;
} download_item_t;

// Main browser state
typedef struct limitless_browser {
    bool initialized;
    bool running;
    
    // Engine configuration
    browser_engine_t engine_type;
    bool javascript_enabled;
    bool webgl_enabled;
    bool notifications_enabled;
    bool location_services_enabled;
    
    // UI components
    lui_window_t* main_window;
    lui_widget_t* tab_bar;
    lui_widget_t* address_bar;
    lui_widget_t* search_bar;
    lui_widget_t* toolbar;
    lui_widget_t* content_area;
    lui_widget_t* sidebar;
    lui_widget_t* status_bar;
    lui_widget_t* ai_panel;
    
    // Tab management
    browser_tab_t* tabs;
    browser_tab_t* active_tab;
    uint32_t tab_count;
    uint32_t next_tab_id;
    
    // Navigation and history
    char current_url[MAX_URL_LENGTH];
    web_page_t* global_history;
    uint32_t history_count;
    
    // Bookmarks and favorites
    bookmark_t* bookmarks;
    uint32_t bookmark_count;
    
    // Downloads
    download_item_t* downloads;
    uint32_t download_count;
    
    // Search and AI
    char search_engine_url[MAX_URL_LENGTH];
    ai_assistance_t ai_mode;
    bool ai_ad_blocking;
    bool ai_privacy_protection;
    bool ai_malware_detection;
    char ai_current_summary[1024];
    
    // Security and privacy
    security_mode_t default_security_mode;
    bool tracking_protection_enabled;
    bool cookie_blocking_enabled;
    bool fingerprinting_protection;
    bool dns_over_https;
    
    // Performance settings
    bool hardware_acceleration;
    bool lazy_loading;
    uint32_t cache_size_mb;
    bool preload_enabled;
    
    // Statistics
    struct {
        uint64_t pages_loaded;
        uint64_t bytes_transferred;
        uint32_t tabs_opened;
        uint32_t bookmarks_added;
        uint32_t ai_queries_processed;
        uint64_t malware_blocked;
        uint64_t ads_blocked;
        uint64_t trackers_blocked;
    } stats;
    
} limitless_browser_t;

// Global browser instance
static limitless_browser_t g_browser = {0};

// ============================================================================
// HTTP CLIENT AND NETWORKING
// ============================================================================

static size_t write_callback(void* contents, size_t size, size_t nmemb, http_response_t* response) {
    size_t realsize = size * nmemb;
    char* ptr = realloc(response->data, response->size + realsize + 1);
    
    if (!ptr) {
        printf("[Browser] ERROR: Out of memory in HTTP callback\n");
        return 0;
    }
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = '\0';
    
    return realsize;
}

static size_t header_callback(void* contents, size_t size, size_t nmemb, http_response_t* response) {
    size_t realsize = size * nmemb;
    char* ptr = realloc(response->headers, response->header_size + realsize + 1);
    
    if (!ptr) return 0;
    
    response->headers = ptr;
    memcpy(&(response->headers[response->header_size]), contents, realsize);
    response->header_size += realsize;
    response->headers[response->header_size] = '\0';
    
    return realsize;
}

static bool http_get(const char* url, http_response_t* response) {
    if (!url || !response) return false;
    
    memset(response, 0, sizeof(http_response_t));
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        printf("[Browser] ERROR: Failed to initialize CURL\n");
        return false;
    }
    
    // Configure request
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, BROWSER_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Security headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.9");
    headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate, br");
    headers = curl_slist_append(headers, "DNT: 1"); // Do Not Track
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->status_code);
        
        // Extract content type
        char* content_type;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
        if (content_type) {
            strncpy(response->content_type, content_type, sizeof(response->content_type) - 1);
        }
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        printf("[Browser] HTTP request failed: %s\n", curl_easy_strerror(res));
        if (response->data) {
            free(response->data);
            response->data = NULL;
        }
        if (response->headers) {
            free(response->headers);
            response->headers = NULL;
        }
        return false;
    }
    
    return true;
}

static void free_http_response(http_response_t* response) {
    if (response) {
        if (response->data) {
            free(response->data);
            response->data = NULL;
        }
        if (response->headers) {
            free(response->headers);
            response->headers = NULL;
        }
        memset(response, 0, sizeof(http_response_t));
    }
}

// ============================================================================
// AI-POWERED CONTENT PROCESSING
// ============================================================================

static float analyze_page_quality(const web_page_t* page) {
    if (!page || !page->html_content) return 0.0f;
    
    float score = 0.5f; // Base score
    
    // Check for proper HTML structure
    if (strstr(page->html_content, "<!DOCTYPE") != NULL) score += 0.1f;
    if (strstr(page->html_content, "<title>") != NULL) score += 0.1f;
    if (strstr(page->html_content, "<meta") != NULL) score += 0.1f;
    
    // Check for accessibility features
    if (strstr(page->html_content, "alt=") != NULL) score += 0.1f;
    if (strstr(page->html_content, "aria-") != NULL) score += 0.1f;
    
    // Check for security headers
    if (page->is_secure) score += 0.2f;
    
    // Penalize for suspicious content
    if (strstr(page->html_content, "onclick=") != NULL) score -= 0.1f;
    if (strstr(page->html_content, "eval(") != NULL) score -= 0.2f;
    
    // Content length assessment
    size_t content_length = strlen(page->html_content);
    if (content_length > 1000 && content_length < 100000) {
        score += 0.1f; // Good content length
    }
    
    return fmaxf(0.0f, fminf(1.0f, score));
}

static void generate_ai_summary(web_page_t* page) {
    if (!page || !page->html_content) return;
    
    // Simplified AI summary generation
    // Real implementation would use ML models
    
    // Extract title
    char* title_start = strstr(page->html_content, "<title>");
    char* title_end = strstr(page->html_content, "</title>");
    
    if (title_start && title_end && title_end > title_start) {
        title_start += 7; // Skip "<title>"
        size_t title_len = title_end - title_start;
        if (title_len < sizeof(page->title) - 1) {
            strncpy(page->title, title_start, title_len);
            page->title[title_len] = '\0';
        }
    }
    
    // Generate basic summary
    snprintf(page->ai_summary, sizeof(page->ai_summary), 
             "Web page: %s. Content size: %zu bytes. Quality score: %.2f", 
             page->title[0] ? page->title : "Untitled",
             page->content_size, 
             page->quality_score);
}

static void detect_page_topics(web_page_t* page) {
    if (!page || !page->html_content) return;
    
    // Simple topic detection based on common keywords
    // Real implementation would use NLP models
    
    const char* tech_keywords[] = {"programming", "software", "code", "developer", "API", "GitHub"};
    const char* news_keywords[] = {"news", "breaking", "report", "update", "politics", "economy"};
    const char* social_keywords[] = {"social", "network", "community", "forum", "discussion", "chat"};
    
    bool is_tech = false, is_news = false, is_social = false;
    
    char* lowercase_content = strdup(page->html_content);
    if (lowercase_content) {
        // Convert to lowercase for matching
        for (size_t i = 0; lowercase_content[i]; i++) {
            lowercase_content[i] = tolower(lowercase_content[i]);
        }
        
        // Check for tech keywords
        for (size_t i = 0; i < sizeof(tech_keywords) / sizeof(tech_keywords[0]); i++) {
            if (strstr(lowercase_content, tech_keywords[i])) {
                is_tech = true;
                break;
            }
        }
        
        // Check for news keywords
        for (size_t i = 0; i < sizeof(news_keywords) / sizeof(news_keywords[0]); i++) {
            if (strstr(lowercase_content, news_keywords[i])) {
                is_news = true;
                break;
            }
        }
        
        // Check for social keywords
        for (size_t i = 0; i < sizeof(social_keywords) / sizeof(social_keywords[0]); i++) {
            if (strstr(lowercase_content, social_keywords[i])) {
                is_social = true;
                break;
            }
        }
        
        free(lowercase_content);
    }
    
    // Build topics string
    page->topics[0] = '\0';
    if (is_tech) strcat(page->topics, "technology ");
    if (is_news) strcat(page->topics, "news ");
    if (is_social) strcat(page->topics, "social ");
    
    if (page->topics[0] == '\0') {
        strcpy(page->topics, "general");
    }
}

// ============================================================================
// TAB MANAGEMENT
// ============================================================================

static browser_tab_t* create_tab(const char* url, bool activate) {
    if (g_browser.tab_count >= MAX_TABS) {
        printf("[Browser] Maximum tab limit reached\n");
        return NULL;
    }
    
    browser_tab_t* tab = calloc(1, sizeof(browser_tab_t));
    if (!tab) {
        printf("[Browser] ERROR: Failed to allocate tab\n");
        return NULL;
    }
    
    // Initialize tab
    tab->id = g_browser.next_tab_id++;
    strcpy(tab->title, "New Tab");
    tab->is_active = activate;
    tab->security_mode = g_browser.default_security_mode;
    tab->accent_color = LUI_COLOR_TACTICAL_BLUE;
    
    // Add to tab list
    tab->next = g_browser.tabs;
    if (g_browser.tabs) {
        g_browser.tabs->prev = tab;
    }
    g_browser.tabs = tab;
    g_browser.tab_count++;
    
    // Activate if requested
    if (activate) {
        if (g_browser.active_tab) {
            g_browser.active_tab->is_active = false;
        }
        g_browser.active_tab = tab;
    }
    
    printf("[Browser] Created tab %u: %s\n", tab->id, url ? url : "blank");
    
    // Load initial URL if provided
    if (url && strlen(url) > 0) {
        browser_navigate_to(url);
    }
    
    return tab;
}

static void close_tab(browser_tab_t* tab) {
    if (!tab) return;
    
    printf("[Browser] Closing tab %u\n", tab->id);
    
    // Remove from tab list
    if (tab->prev) {
        tab->prev->next = tab->next;
    } else {
        g_browser.tabs = tab->next;
    }
    if (tab->next) {
        tab->next->prev = tab->prev;
    }
    
    // Update active tab if needed
    if (g_browser.active_tab == tab) {
        g_browser.active_tab = g_browser.tabs; // Switch to first available tab
        if (g_browser.active_tab) {
            g_browser.active_tab->is_active = true;
        }
    }
    
    // Free tab resources
    if (tab->current_page && tab->current_page->html_content) {
        free(tab->current_page->html_content);
    }
    if (tab->current_page && tab->current_page->rendered_content) {
        free(tab->current_page->rendered_content);
    }
    if (tab->current_page) {
        free(tab->current_page);
    }
    
    // Free history
    web_page_t* page = tab->history;
    while (page) {
        web_page_t* next = page->next;
        if (page->html_content) free(page->html_content);
        if (page->rendered_content) free(page->rendered_content);
        free(page);
        page = next;
    }
    
    free(tab);
    g_browser.tab_count--;
}

// ============================================================================
// NAVIGATION AND PAGE LOADING
// ============================================================================

bool browser_navigate_to(const char* url) {
    if (!url || !g_browser.initialized || !g_browser.active_tab) {
        return false;
    }
    
    printf("[Browser] Navigating to: %s\n", url);
    
    // Update tab state
    g_browser.active_tab->is_loading = true;
    g_browser.active_tab->loading_progress = 0.0f;
    
    // Create new page
    web_page_t* page = calloc(1, sizeof(web_page_t));
    if (!page) {
        printf("[Browser] ERROR: Failed to allocate page\n");
        return false;
    }
    
    strncpy(page->url, url, sizeof(page->url) - 1);
    page->state = PAGE_STATE_LOADING;
    page->load_time = time(NULL);
    
    // Check if URL is secure
    page->is_secure = (strncmp(url, "https://", 8) == 0);
    
    // Fetch page content
    http_response_t response;
    uint32_t start_time = time(NULL) * 1000; // Approximate milliseconds
    
    if (http_get(url, &response)) {
        if (response.status_code == 200) {
            page->html_content = response.data;
            page->content_size = response.size;
            page->state = PAGE_STATE_LOADED;
            
            // AI analysis
            page->quality_score = analyze_page_quality(page);
            generate_ai_summary(page);
            detect_page_topics(page);
            
            printf("[Browser] Page loaded successfully (%zu bytes, quality: %.2f)\n", 
                   page->content_size, page->quality_score);
        } else {
            printf("[Browser] HTTP error: %ld\n", response.status_code);
            page->state = PAGE_STATE_ERROR;
            free_http_response(&response);
        }
    } else {
        printf("[Browser] Failed to fetch page\n");
        page->state = PAGE_STATE_ERROR;
    }
    
    page->load_time_ms = (time(NULL) * 1000) - start_time;
    
    // Update tab
    if (g_browser.active_tab->current_page) {
        // Add current page to history
        web_page_t* old_page = g_browser.active_tab->current_page;
        old_page->next = g_browser.active_tab->history;
        if (g_browser.active_tab->history) {
            g_browser.active_tab->history->prev = old_page;
        }
        g_browser.active_tab->history = old_page;
        g_browser.active_tab->history_count++;
    }
    
    g_browser.active_tab->current_page = page;
    g_browser.active_tab->is_loading = false;
    g_browser.active_tab->loading_progress = 1.0f;
    
    // Update tab title
    if (page->title[0]) {
        strncpy(g_browser.active_tab->title, page->title, sizeof(g_browser.active_tab->title) - 1);
    } else {
        strcpy(g_browser.active_tab->title, "Untitled");
    }
    
    // Update current URL
    strncpy(g_browser.current_url, url, sizeof(g_browser.current_url) - 1);
    
    // Update statistics
    g_browser.stats.pages_loaded++;
    if (page->content_size > 0) {
        g_browser.stats.bytes_transferred += page->content_size;
    }
    
    // Update AI summary for display
    if (g_browser.ai_mode >= AI_ASSIST_STANDARD && page->ai_summary[0]) {
        strncpy(g_browser.ai_current_summary, page->ai_summary, 
                sizeof(g_browser.ai_current_summary) - 1);
    }
    
    return (page->state == PAGE_STATE_LOADED);
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_tab_bar(void) {
    g_browser.tab_bar = lui_create_container(g_browser.main_window->root_widget);
    strcpy(g_browser.tab_bar->name, "tab_bar");
    g_browser.tab_bar->bounds = lui_rect_make(0, 0, 1400, 32);
    g_browser.tab_bar->background_color = LUI_COLOR_GRAPHITE;
    
    // TODO: Render individual tabs based on g_browser.tabs
}

static void create_toolbar(void) {
    g_browser.toolbar = lui_create_container(g_browser.main_window->root_widget);
    strcpy(g_browser.toolbar->name, "toolbar");
    g_browser.toolbar->bounds = lui_rect_make(0, 32, 1400, 40);
    g_browser.toolbar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Navigation buttons
    lui_widget_t* back_btn = lui_create_button("◀", g_browser.toolbar);
    back_btn->bounds = lui_rect_make(8, 4, 32, 32);
    
    lui_widget_t* forward_btn = lui_create_button("▶", g_browser.toolbar);
    forward_btn->bounds = lui_rect_make(48, 4, 32, 32);
    
    lui_widget_t* refresh_btn = lui_create_button("↻", g_browser.toolbar);
    refresh_btn->bounds = lui_rect_make(88, 4, 32, 32);
    
    lui_widget_t* home_btn = lui_create_button("🏠", g_browser.toolbar);
    home_btn->bounds = lui_rect_make(128, 4, 32, 32);
}

static void create_address_bar(void) {
    g_browser.address_bar = lui_create_text_input("Enter URL or search...", g_browser.main_window->root_widget);
    strcpy(g_browser.address_bar->name, "address_bar");
    g_browser.address_bar->bounds = lui_rect_make(168, 36, 1000, 32);
    
    // Security indicator
    lui_widget_t* security_indicator = lui_create_label("🔒", g_browser.main_window->root_widget);
    security_indicator->bounds = lui_rect_make(1175, 36, 24, 32);
}

static void create_content_area(void) {
    g_browser.content_area = lui_create_container(g_browser.main_window->root_widget);
    strcpy(g_browser.content_area->name, "content_area");
    g_browser.content_area->bounds = lui_rect_make(0, 72, 1100, 600);
    g_browser.content_area->background_color = LUI_COLOR_ARCTIC_WHITE;
    
    // Simple text display for now (real implementation would render HTML)
    if (g_browser.active_tab && g_browser.active_tab->current_page) {
        web_page_t* page = g_browser.active_tab->current_page;
        
        // Display basic page info
        char display_text[512];
        snprintf(display_text, sizeof(display_text),
                "URL: %s\nTitle: %s\nSize: %zu bytes\nQuality: %.2f\nTopics: %s\n\nSummary: %s",
                page->url,
                page->title[0] ? page->title : "Untitled",
                page->content_size,
                page->quality_score,
                page->topics,
                page->ai_summary);
        
        lui_widget_t* content_label = lui_create_label(display_text, g_browser.content_area);
        content_label->bounds = lui_rect_make(16, 16, 1068, 568);
        content_label->typography = LUI_TYPOGRAPHY_BODY_MEDIUM;
    }
}

static void create_ai_panel(void) {
    if (g_browser.ai_mode == AI_ASSIST_DISABLED) return;
    
    g_browser.ai_panel = lui_create_container(g_browser.main_window->root_widget);
    strcpy(g_browser.ai_panel->name, "ai_panel");
    g_browser.ai_panel->bounds = lui_rect_make(1100, 72, 300, 600);
    g_browser.ai_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // AI assistant header
    lui_widget_t* ai_header = lui_create_label("🤖 AI Assistant", g_browser.ai_panel);
    ai_header->bounds = lui_rect_make(8, 8, 284, 24);
    ai_header->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
    ai_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // Current page summary
    if (g_browser.ai_current_summary[0]) {
        lui_widget_t* summary_label = lui_create_label(g_browser.ai_current_summary, g_browser.ai_panel);
        summary_label->bounds = lui_rect_make(8, 40, 284, 200);
        summary_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
    }
    
    // AI features
    lui_widget_t* translate_btn = lui_create_button("Translate", g_browser.ai_panel);
    translate_btn->bounds = lui_rect_make(8, 250, 80, 28);
    
    lui_widget_t* summarize_btn = lui_create_button("Summarize", g_browser.ai_panel);
    summarize_btn->bounds = lui_rect_make(96, 250, 80, 28);
    
    lui_widget_t* analyze_btn = lui_create_button("Analyze", g_browser.ai_panel);
    analyze_btn->bounds = lui_rect_make(184, 250, 80, 28);
}

static void create_status_bar(void) {
    g_browser.status_bar = lui_create_container(g_browser.main_window->root_widget);
    strcpy(g_browser.status_bar->name, "status_bar");
    g_browser.status_bar->bounds = lui_rect_make(0, 672, 1400, 24);
    g_browser.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Status information
    char status_text[256];
    snprintf(status_text, sizeof(status_text), 
             "Ready | %u tabs | %llu pages loaded | AI: %s",
             g_browser.tab_count,
             (unsigned long long)g_browser.stats.pages_loaded,
             (g_browser.ai_mode > AI_ASSIST_DISABLED) ? "Active" : "Disabled");
    
    lui_widget_t* status_label = lui_create_label(status_text, g_browser.status_bar);
    status_label->bounds = lui_rect_make(8, 2, 600, 20);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
}

// ============================================================================
// MAIN BROWSER API
// ============================================================================

bool limitless_browser_init(void) {
    if (g_browser.initialized) {
        return false;
    }
    
    printf("[Browser] Initializing Limitless Browser v%s\n", BROWSER_VERSION);
    
    // Initialize CURL
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        printf("[Browser] ERROR: Failed to initialize CURL\n");
        return false;
    }
    
    // Clear state
    memset(&g_browser, 0, sizeof(g_browser));
    
    // Set default configuration
    g_browser.engine_type = ENGINE_LIMITLESS_NATIVE;
    g_browser.javascript_enabled = true;
    g_browser.webgl_enabled = true;
    g_browser.notifications_enabled = true;
    g_browser.default_security_mode = SECURITY_MODE_STANDARD;
    g_browser.ai_mode = AI_ASSIST_STANDARD;
    g_browser.ai_ad_blocking = true;
    g_browser.ai_privacy_protection = true;
    g_browser.ai_malware_detection = true;
    g_browser.tracking_protection_enabled = true;
    g_browser.hardware_acceleration = true;
    g_browser.lazy_loading = true;
    g_browser.cache_size_mb = RENDER_CACHE_SIZE_MB;
    g_browser.next_tab_id = 1;
    
    strcpy(g_browser.search_engine_url, "https://www.google.com/search?q=");
    
    // Create main window
    g_browser.main_window = lui_create_window("Limitless Browser", LUI_WINDOW_NORMAL,
                                            50, 50, 1400, 696);
    if (!g_browser.main_window) {
        printf("[Browser] ERROR: Failed to create main window\n");
        curl_global_cleanup();
        return false;
    }
    
    // Create UI components
    create_tab_bar();
    create_toolbar();
    create_address_bar();
    create_content_area();
    create_ai_panel();
    create_status_bar();
    
    // Create initial tab
    create_tab("https://www.limitlessos.org", true);
    
    // Show window
    lui_show_window(g_browser.main_window);
    
    g_browser.initialized = true;
    g_browser.running = true;
    
    printf("[Browser] Browser initialized successfully\n");
    printf("[Browser] Engine: %s, AI Mode: %d, Security: %d\n",
           (g_browser.engine_type == ENGINE_LIMITLESS_NATIVE) ? "Native" : "Other",
           g_browser.ai_mode, g_browser.default_security_mode);
    
    return true;
}

void limitless_browser_shutdown(void) {
    if (!g_browser.initialized) {
        return;
    }
    
    printf("[Browser] Shutting down Limitless Browser\n");
    
    g_browser.running = false;
    
    // Close all tabs
    browser_tab_t* tab = g_browser.tabs;
    while (tab) {
        browser_tab_t* next = tab->next;
        close_tab(tab);
        tab = next;
    }
    
    // Free bookmarks
    bookmark_t* bookmark = g_browser.bookmarks;
    while (bookmark) {
        bookmark_t* next = bookmark->next;
        free(bookmark);
        bookmark = next;
    }
    
    // Free downloads
    download_item_t* download = g_browser.downloads;
    while (download) {
        download_item_t* next = download->next;
        free(download);
        download = next;
    }
    
    // Destroy main window
    if (g_browser.main_window) {
        lui_destroy_window(g_browser.main_window);
    }
    
    // Cleanup CURL
    curl_global_cleanup();
    
    memset(&g_browser, 0, sizeof(g_browser));
    
    printf("[Browser] Shutdown complete\n");
}

browser_tab_t* limitless_browser_new_tab(const char* url) {
    return create_tab(url, true);
}

void limitless_browser_close_tab(uint32_t tab_id) {
    browser_tab_t* tab = g_browser.tabs;
    while (tab) {
        if (tab->id == tab_id) {
            close_tab(tab);
            break;
        }
        tab = tab->next;
    }
}

const char* limitless_browser_get_version(void) {
    return BROWSER_VERSION;
}

void limitless_browser_run(void) {
    if (!g_browser.initialized) {
        printf("[Browser] ERROR: Browser not initialized\n");
        return;
    }
    
    printf("[Browser] Running Limitless Browser\n");
    
    // Main event loop is handled by the desktop environment
    // This function serves as an entry point for standalone execution
}