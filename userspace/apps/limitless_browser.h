/**
 * Limitless Browser - Header File
 * 
 * Next-generation web browser with AI integration for LimitlessOS
 * Unified interface for web content, local files, and system search
 */

#ifndef LIMITLESS_BROWSER_H
#define LIMITLESS_BROWSER_H

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
    BROWSER_ENGINE_NATIVE = 0,
    BROWSER_ENGINE_WEBKIT,
    BROWSER_ENGINE_BLINK,
    BROWSER_ENGINE_TEXT
} browser_engine_type_t;

typedef enum {
    BROWSER_SECURITY_STANDARD = 0,
    BROWSER_SECURITY_PRIVATE,
    BROWSER_SECURITY_SECURE,
    BROWSER_SECURITY_ISOLATED,
    BROWSER_SECURITY_TOR
} browser_security_level_t;

typedef enum {
    BROWSER_AI_DISABLED = 0,
    BROWSER_AI_MINIMAL,
    BROWSER_AI_STANDARD,
    BROWSER_AI_ENHANCED,
    BROWSER_AI_COPILOT
} browser_ai_level_t;

typedef struct browser_config {
    browser_engine_type_t engine;
    browser_security_level_t security;
    browser_ai_level_t ai_level;
    bool javascript_enabled;
    bool webgl_enabled;
    bool hardware_acceleration;
    bool ad_blocking;
    bool tracking_protection;
    bool malware_detection;
    char search_engine[256];
    char user_agent[256];
} browser_config_t;

// ============================================================================
// CORE BROWSER API
// ============================================================================

/**
 * Initialize the Limitless Browser system
 * Returns: true if successful, false otherwise
 */
bool limitless_browser_init(void);

/**
 * Shutdown the browser and cleanup resources
 */
void limitless_browser_shutdown(void);

/**
 * Run the browser main loop
 */
void limitless_browser_run(void);

/**
 * Get browser version string
 * Returns: Version string (static)
 */
const char* limitless_browser_get_version(void);

// ============================================================================
// TAB AND NAVIGATION API
// ============================================================================

/**
 * Create a new browser tab
 * url: Initial URL to load (can be NULL)
 * Returns: Pointer to new tab structure (opaque)
 */
struct browser_tab* limitless_browser_new_tab(const char* url);

/**
 * Close a browser tab by ID
 * tab_id: ID of tab to close
 */
void limitless_browser_close_tab(uint32_t tab_id);

/**
 * Navigate active tab to URL
 * url: Target URL to navigate to
 * Returns: true if navigation started successfully
 */
bool browser_navigate_to(const char* url);

/**
 * Navigate back in history
 * Returns: true if navigation possible
 */
bool browser_navigate_back(void);

/**
 * Navigate forward in history
 * Returns: true if navigation possible
 */
bool browser_navigate_forward(void);

/**
 * Refresh current page
 * force_reload: true to bypass cache
 * Returns: true if refresh started
 */
bool browser_refresh_page(bool force_reload);

// ============================================================================
// SEARCH AND AI API
// ============================================================================

/**
 * Perform intelligent search
 * query: Search query string
 * Returns: true if search initiated
 */
bool browser_search(const char* query);

/**
 * Get AI summary of current page
 * buffer: Output buffer for summary
 * buffer_size: Size of output buffer
 * Returns: true if summary available
 */
bool browser_get_ai_summary(char* buffer, size_t buffer_size);

/**
 * Enable/disable AI reading mode
 * enabled: true to enable reading mode
 */
void browser_set_reading_mode(bool enabled);

/**
 * Translate current page
 * target_language: ISO language code (e.g., "es", "fr")
 * Returns: true if translation started
 */
bool browser_translate_page(const char* target_language);

/**
 * Analyze page content with AI
 * Returns: Quality score (0.0-1.0)
 */
float browser_analyze_page_quality(void);

// ============================================================================
// BOOKMARKS AND HISTORY API
// ============================================================================

/**
 * Add current page to bookmarks
 * title: Custom title (NULL for page title)
 * folder: Bookmark folder (NULL for default)
 * Returns: true if bookmark added
 */
bool browser_add_bookmark(const char* title, const char* folder);

/**
 * Remove bookmark by URL
 * url: URL to remove from bookmarks
 * Returns: true if bookmark removed
 */
bool browser_remove_bookmark(const char* url);

/**
 * Search bookmarks
 * query: Search query
 * results: Array to store results (max 50)
 * Returns: Number of results found
 */
int browser_search_bookmarks(const char* query, char results[][256]);

/**
 * Clear browsing history
 * days: Number of days to keep (0 = clear all)
 */
void browser_clear_history(int days);

// ============================================================================
// SECURITY AND PRIVACY API
// ============================================================================

/**
 * Set security mode for current tab
 * mode: Security mode to apply
 */
void browser_set_security_mode(browser_security_level_t mode);

/**
 * Get security information for current page
 * info: Buffer to store security info
 * info_size: Size of info buffer
 * Returns: true if security info available
 */
bool browser_get_security_info(char* info, size_t info_size);

/**
 * Enable/disable private browsing
 * enabled: true for private mode
 */
void browser_set_private_mode(bool enabled);

/**
 * Block specific domain
 * domain: Domain to block
 */
void browser_block_domain(const char* domain);

/**
 * Get privacy statistics
 * ads_blocked: Pointer to store ads blocked count
 * trackers_blocked: Pointer to store trackers blocked count
 * malware_blocked: Pointer to store malware blocked count
 */
void browser_get_privacy_stats(uint64_t* ads_blocked, uint64_t* trackers_blocked, 
                              uint64_t* malware_blocked);

// ============================================================================
// DOWNLOADS API
// ============================================================================

/**
 * Start download from URL
 * url: Download URL
 * local_path: Local save path (NULL for default)
 * Returns: Download ID (0 on failure)
 */
uint32_t browser_start_download(const char* url, const char* local_path);

/**
 * Get download progress
 * download_id: Download ID
 * progress: Pointer to store progress (0.0-1.0)
 * speed_kbps: Pointer to store speed
 * Returns: true if download exists
 */
bool browser_get_download_progress(uint32_t download_id, float* progress, uint32_t* speed_kbps);

/**
 * Cancel download
 * download_id: Download ID to cancel
 */
void browser_cancel_download(uint32_t download_id);

// ============================================================================
// CONFIGURATION API
// ============================================================================

/**
 * Apply browser configuration
 * config: Configuration structure
 * Returns: true if config applied successfully
 */
bool browser_apply_config(const browser_config_t* config);

/**
 * Get current browser configuration
 * config: Buffer to store current config
 */
void browser_get_config(browser_config_t* config);

/**
 * Set user agent string
 * user_agent: Custom user agent string
 */
void browser_set_user_agent(const char* user_agent);

/**
 * Set default search engine
 * search_url: Search engine URL with %s placeholder
 */
void browser_set_search_engine(const char* search_url);

// ============================================================================
// STATISTICS AND MONITORING API
// ============================================================================

typedef struct browser_stats {
    uint64_t pages_loaded;
    uint64_t bytes_transferred;
    uint32_t tabs_opened;
    uint32_t bookmarks_count;
    uint32_t history_entries;
    uint64_t ai_queries_processed;
    uint64_t security_threats_blocked;
    uint32_t average_load_time_ms;
    float average_page_quality;
} browser_stats_t;

/**
 * Get browser usage statistics
 * stats: Buffer to store statistics
 */
void browser_get_stats(browser_stats_t* stats);

/**
 * Reset browser statistics
 */
void browser_reset_stats(void);

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

typedef void (*browser_page_load_callback_t)(const char* url, bool success);
typedef void (*browser_security_alert_callback_t)(const char* threat_type, const char* details);
typedef void (*browser_ai_insight_callback_t)(const char* insight_type, const char* content);

/**
 * Set page load event callback
 * callback: Function to call on page load events
 */
void browser_set_page_load_callback(browser_page_load_callback_t callback);

/**
 * Set security alert callback
 * callback: Function to call on security events
 */
void browser_set_security_alert_callback(browser_security_alert_callback_t callback);

/**
 * Set AI insight callback
 * callback: Function to call on AI insights
 */
void browser_set_ai_insight_callback(browser_ai_insight_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // LIMITLESS_BROWSER_H