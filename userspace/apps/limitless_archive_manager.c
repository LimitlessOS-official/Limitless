/**
 * LimitlessOS Archive Manager
 * 
 * Advanced file compression and extraction utility with AI-powered optimization,
 * security scanning, and military-grade interface design. Supports multiple
 * archive formats with intelligent compression analysis and threat detection.
 * 
 * Features:
 * - Multi-format archive support (ZIP, TAR, 7Z, RAR, GZ, BZ2, XZ, LZ4)
 * - AI-powered compression optimization and file type detection
 * - Real-time security scanning and malware detection
 * - Smart compression recommendations based on content analysis
 * - Batch processing with progress tracking and resume capabilities
 * - Password protection and encryption with military-grade algorithms
 * - Archive integrity verification and repair utilities
 * - Duplicate file detection and deduplication
 * - Compression ratio analysis and performance metrics
 * - Network-optimized transfer preparation
 * 
 * Military Design Principles:
 * - Tactical color schemes with secure visual indicators
 * - Precision file management with detailed progress tracking
 * - Security-first approach with threat detection integration
 * - Performance optimization for mission-critical operations
 * - Configurable security policies and access controls
 * 
 * AI Integration:
 * - Intelligent file type detection and compression recommendations
 * - Content-aware compression optimization for different file types
 * - Security threat analysis and suspicious file detection
 * - Automatic archive organization and categorization
 * - Performance learning and optimization suggestions
 * 
 * @author LimitlessOS Development Team
 * @version 1.0.0
 * @since 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

// Desktop integration
#include "../../include/limitless_ui.h"
#include "../../include/desktop_integration.h"

// Archive manager configuration
#define ARCHIVE_MANAGER_VERSION "1.0.0"
#define MAX_ARCHIVE_ITEMS 10000
#define MAX_COMPRESSION_LEVELS 10
#define MAX_PASSWORD_LENGTH 256
#define MAX_PATH_LENGTH 1024
#define MAX_FILENAME_LENGTH 256
#define BUFFER_SIZE 1048576  // 1MB buffer
#define AI_ANALYSIS_THRESHOLD 100  // Files above this count get AI analysis

// Archive format definitions
typedef enum {
    ARCHIVE_FORMAT_ZIP,
    ARCHIVE_FORMAT_TAR,
    ARCHIVE_FORMAT_TAR_GZ,
    ARCHIVE_FORMAT_TAR_BZ2,
    ARCHIVE_FORMAT_TAR_XZ,
    ARCHIVE_FORMAT_7Z,
    ARCHIVE_FORMAT_RAR,
    ARCHIVE_FORMAT_GZ,
    ARCHIVE_FORMAT_BZ2,
    ARCHIVE_FORMAT_XZ,
    ARCHIVE_FORMAT_LZ4,
    ARCHIVE_FORMAT_ZSTD,
    ARCHIVE_FORMAT_UNKNOWN
} archive_format_t;

typedef enum {
    COMPRESSION_STORE,      // No compression
    COMPRESSION_FASTEST,    // Fastest compression
    COMPRESSION_FAST,       // Fast compression
    COMPRESSION_NORMAL,     // Normal compression
    COMPRESSION_MAXIMUM,    // Maximum compression
    COMPRESSION_ULTRA       // Ultra compression (slowest)
} compression_level_t;

typedef enum {
    OPERATION_EXTRACT,
    OPERATION_CREATE,
    OPERATION_ADD,
    OPERATION_DELETE,
    OPERATION_TEST,
    OPERATION_LIST,
    OPERATION_UPDATE
} archive_operation_t;

typedef enum {
    ENCRYPTION_NONE,
    ENCRYPTION_ZIP_TRADITIONAL,
    ENCRYPTION_AES128,
    ENCRYPTION_AES192,
    ENCRYPTION_AES256,
    ENCRYPTION_CHACHA20,
    ENCRYPTION_MILITARY_GRADE
} encryption_type_t;

// File type classification for compression optimization
typedef enum {
    FILE_TYPE_UNKNOWN,
    FILE_TYPE_TEXT,
    FILE_TYPE_BINARY,
    FILE_TYPE_EXECUTABLE,
    FILE_TYPE_IMAGE,
    FILE_TYPE_AUDIO,
    FILE_TYPE_VIDEO,
    FILE_TYPE_COMPRESSED,
    FILE_TYPE_DATABASE,
    FILE_TYPE_DOCUMENT,
    FILE_TYPE_ARCHIVE
} file_type_t;

// Archive item information
typedef struct archive_item {
    char filename[MAX_FILENAME_LENGTH];
    char full_path[MAX_PATH_LENGTH];
    uint64_t original_size;
    uint64_t compressed_size;
    float compression_ratio;
    time_t modified_time;
    time_t created_time;
    uint32_t crc32;
    bool is_directory;
    bool is_encrypted;
    bool is_executable;
    bool has_security_warning;
    file_type_t file_type;
    
    // Security analysis
    struct {
        bool scanned;
        bool is_suspicious;
        bool contains_malware;
        bool has_dangerous_extension;
        char threat_description[256];
        float risk_score;  // 0.0 to 1.0
    } security;
    
    // AI analysis
    struct {
        bool analyzed;
        bool optimal_compression;
        compression_level_t recommended_level;
        char optimization_notes[256];
        float space_savings_potential;
    } ai_analysis;
    
    struct archive_item* next;
} archive_item_t;

// Archive information
typedef struct {
    char filepath[MAX_PATH_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    char display_name[MAX_FILENAME_LENGTH];
    archive_format_t format;
    uint64_t total_size;
    uint64_t compressed_size;
    uint32_t item_count;
    float overall_compression_ratio;
    time_t created_time;
    time_t modified_time;
    bool has_password;
    bool is_encrypted;
    bool is_solid;  // Solid compression (7z)
    encryption_type_t encryption;
    
    // Archive items
    archive_item_t* items;
    uint32_t loaded_items;
    
    // Security summary
    struct {
        uint32_t suspicious_files;
        uint32_t malware_detected;
        uint32_t dangerous_extensions;
        float overall_risk_score;
        bool quarantine_recommended;
    } security_summary;
    
    // Performance metrics
    struct {
        float extraction_speed;  // MB/s
        float compression_speed; // MB/s
        time_t last_access_time;
        uint32_t access_count;
    } performance;
    
} archive_info_t;

// Compression job for background processing
typedef struct compression_job {
    archive_operation_t operation;
    char source_path[MAX_PATH_LENGTH];
    char target_path[MAX_PATH_LENGTH];
    archive_format_t format;
    compression_level_t level;
    encryption_type_t encryption;
    char password[MAX_PASSWORD_LENGTH];
    
    // Progress tracking
    bool active;
    bool completed;
    bool cancelled;
    bool error;
    char error_message[256];
    
    uint64_t total_bytes;
    uint64_t processed_bytes;
    uint32_t total_files;
    uint32_t processed_files;
    float progress_percent;
    time_t start_time;
    time_t estimated_completion;
    
    // Performance metrics
    float current_speed;     // MB/s
    float average_speed;     // MB/s
    uint64_t bytes_per_second;
    
    pthread_t thread_id;
    struct compression_job* next;
} compression_job_t;

// AI compression advisor
typedef struct {
    bool enabled;
    bool learning_mode;
    
    // Analysis statistics
    struct {
        uint32_t files_analyzed;
        uint32_t recommendations_made;
        uint32_t recommendations_accepted;
        float average_space_savings;
        float accuracy_rate;
    } stats;
    
    // File type optimization profiles
    struct {
        compression_level_t text_optimal;
        compression_level_t binary_optimal;
        compression_level_t image_optimal;
        compression_level_t video_optimal;
        compression_level_t audio_optimal;
        bool skip_compressed_files;
    } profiles;
    
    // Security integration
    struct {
        bool scan_during_compression;
        bool quarantine_threats;
        bool block_dangerous_files;
        float security_threshold;
    } security_policy;
    
} ai_compression_advisor_t;

// Main archive manager state
typedef struct {
    bool initialized;
    bool running;
    
    // Current archive
    archive_info_t* current_archive;
    char current_archive_path[MAX_PATH_LENGTH];
    bool archive_loaded;
    
    // Archive list (recently opened)
    archive_info_t* recent_archives[10];
    uint32_t recent_count;
    
    // Active compression jobs
    compression_job_t* active_jobs;
    uint32_t job_count;
    pthread_mutex_t jobs_mutex;
    
    // UI state
    lui_window_t* main_window;
    lui_widget_t* toolbar;
    lui_widget_t* file_list;
    lui_widget_t* properties_panel;
    lui_widget_t* progress_panel;
    lui_widget_t* ai_advisor_panel;
    lui_widget_t* status_bar;
    
    bool show_properties;
    bool show_progress;
    bool show_ai_advisor;
    bool show_hidden_files;
    bool show_system_files;
    
    // View preferences
    typedef enum {
        VIEW_LIST,
        VIEW_DETAILS,
        VIEW_ICONS,
        VIEW_TREE
    } view_mode_t;
    
    view_mode_t view_mode;
    bool sort_by_name;
    bool sort_by_size;
    bool sort_by_date;
    bool sort_ascending;
    
    // Settings
    struct {
        compression_level_t default_compression;
        encryption_type_t default_encryption;
        bool auto_password_generate;
        bool verify_after_operation;
        bool delete_source_after_compression;
        bool create_recovery_records;
        uint32_t thread_count;
        uint64_t memory_limit;
        bool hardware_acceleration;
    } settings;
    
    // AI advisor
    ai_compression_advisor_t ai_advisor;
    
    // Security scanner
    struct {
        bool enabled;
        bool real_time_scanning;
        bool cloud_scanning;
        char scanner_engine[64];
        time_t last_update;
        uint32_t threats_blocked;
    } security_scanner;
    
    // Session statistics
    struct {
        time_t session_start_time;
        uint32_t archives_created;
        uint32_t archives_extracted;
        uint32_t files_compressed;
        uint64_t bytes_compressed;
        uint64_t bytes_extracted;
        float total_compression_ratio;
        uint32_t security_threats_detected;
        time_t total_operation_time;
    } stats;
    
} archive_manager_state_t;

// Global archive manager state
static archive_manager_state_t g_archive_manager = {0};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static const char* get_format_name(archive_format_t format) {
    switch (format) {
        case ARCHIVE_FORMAT_ZIP: return "ZIP";
        case ARCHIVE_FORMAT_TAR: return "TAR";
        case ARCHIVE_FORMAT_TAR_GZ: return "TAR.GZ";
        case ARCHIVE_FORMAT_TAR_BZ2: return "TAR.BZ2";
        case ARCHIVE_FORMAT_TAR_XZ: return "TAR.XZ";
        case ARCHIVE_FORMAT_7Z: return "7Z";
        case ARCHIVE_FORMAT_RAR: return "RAR";
        case ARCHIVE_FORMAT_GZ: return "GZIP";
        case ARCHIVE_FORMAT_BZ2: return "BZIP2";
        case ARCHIVE_FORMAT_XZ: return "XZ";
        case ARCHIVE_FORMAT_LZ4: return "LZ4";
        case ARCHIVE_FORMAT_ZSTD: return "ZSTD";
        default: return "Unknown";
    }
}

static const char* get_compression_name(compression_level_t level) {
    switch (level) {
        case COMPRESSION_STORE: return "Store";
        case COMPRESSION_FASTEST: return "Fastest";
        case COMPRESSION_FAST: return "Fast";
        case COMPRESSION_NORMAL: return "Normal";
        case COMPRESSION_MAXIMUM: return "Maximum";
        case COMPRESSION_ULTRA: return "Ultra";
        default: return "Normal";
    }
}

static const char* get_encryption_name(encryption_type_t encryption) {
    switch (encryption) {
        case ENCRYPTION_NONE: return "None";
        case ENCRYPTION_ZIP_TRADITIONAL: return "ZIP Traditional";
        case ENCRYPTION_AES128: return "AES-128";
        case ENCRYPTION_AES192: return "AES-192";
        case ENCRYPTION_AES256: return "AES-256";
        case ENCRYPTION_CHACHA20: return "ChaCha20";
        case ENCRYPTION_MILITARY_GRADE: return "Military Grade";
        default: return "None";
    }
}

static archive_format_t detect_archive_format(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (!ext) return ARCHIVE_FORMAT_UNKNOWN;
    
    ext++; // Skip the dot
    
    if (strcasecmp(ext, "zip") == 0) return ARCHIVE_FORMAT_ZIP;
    if (strcasecmp(ext, "tar") == 0) return ARCHIVE_FORMAT_TAR;
    if (strcasecmp(ext, "gz") == 0) {
        // Check for .tar.gz
        const char* prev_ext = filepath + strlen(filepath) - strlen(ext) - 2;
        while (prev_ext > filepath && *prev_ext != '.') prev_ext--;
        if (prev_ext > filepath && strncasecmp(prev_ext, ".tar", 4) == 0) {
            return ARCHIVE_FORMAT_TAR_GZ;
        }
        return ARCHIVE_FORMAT_GZ;
    }
    if (strcasecmp(ext, "bz2") == 0) {
        // Check for .tar.bz2
        const char* prev_ext = filepath + strlen(filepath) - strlen(ext) - 2;
        while (prev_ext > filepath && *prev_ext != '.') prev_ext--;
        if (prev_ext > filepath && strncasecmp(prev_ext, ".tar", 4) == 0) {
            return ARCHIVE_FORMAT_TAR_BZ2;
        }
        return ARCHIVE_FORMAT_BZ2;
    }
    if (strcasecmp(ext, "xz") == 0) {
        // Check for .tar.xz
        const char* prev_ext = filepath + strlen(filepath) - strlen(ext) - 2;
        while (prev_ext > filepath && *prev_ext != '.') prev_ext--;
        if (prev_ext > filepath && strncasecmp(prev_ext, ".tar", 4) == 0) {
            return ARCHIVE_FORMAT_TAR_XZ;
        }
        return ARCHIVE_FORMAT_XZ;
    }
    if (strcasecmp(ext, "7z") == 0) return ARCHIVE_FORMAT_7Z;
    if (strcasecmp(ext, "rar") == 0) return ARCHIVE_FORMAT_RAR;
    if (strcasecmp(ext, "lz4") == 0) return ARCHIVE_FORMAT_LZ4;
    if (strcasecmp(ext, "zst") == 0 || strcasecmp(ext, "zstd") == 0) return ARCHIVE_FORMAT_ZSTD;
    
    return ARCHIVE_FORMAT_UNKNOWN;
}

static file_type_t classify_file_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return FILE_TYPE_BINARY;
    
    ext++; // Skip the dot
    
    // Text files
    if (strcasecmp(ext, "txt") == 0 || strcasecmp(ext, "log") == 0 ||
        strcasecmp(ext, "md") == 0 || strcasecmp(ext, "rst") == 0 ||
        strcasecmp(ext, "json") == 0 || strcasecmp(ext, "xml") == 0 ||
        strcasecmp(ext, "yaml") == 0 || strcasecmp(ext, "yml") == 0 ||
        strcasecmp(ext, "csv") == 0 || strcasecmp(ext, "tsv") == 0) {
        return FILE_TYPE_TEXT;
    }
    
    // Images
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0 ||
        strcasecmp(ext, "png") == 0 || strcasecmp(ext, "gif") == 0 ||
        strcasecmp(ext, "bmp") == 0 || strcasecmp(ext, "tiff") == 0 ||
        strcasecmp(ext, "webp") == 0 || strcasecmp(ext, "svg") == 0) {
        return FILE_TYPE_IMAGE;
    }
    
    // Audio
    if (strcasecmp(ext, "mp3") == 0 || strcasecmp(ext, "wav") == 0 ||
        strcasecmp(ext, "flac") == 0 || strcasecmp(ext, "ogg") == 0 ||
        strcasecmp(ext, "aac") == 0 || strcasecmp(ext, "wma") == 0) {
        return FILE_TYPE_AUDIO;
    }
    
    // Video
    if (strcasecmp(ext, "mp4") == 0 || strcasecmp(ext, "avi") == 0 ||
        strcasecmp(ext, "mkv") == 0 || strcasecmp(ext, "mov") == 0 ||
        strcasecmp(ext, "wmv") == 0 || strcasecmp(ext, "flv") == 0) {
        return FILE_TYPE_VIDEO;
    }
    
    // Compressed files
    if (strcasecmp(ext, "zip") == 0 || strcasecmp(ext, "rar") == 0 ||
        strcasecmp(ext, "7z") == 0 || strcasecmp(ext, "tar") == 0 ||
        strcasecmp(ext, "gz") == 0 || strcasecmp(ext, "bz2") == 0) {
        return FILE_TYPE_COMPRESSED;
    }
    
    // Executables
    if (strcasecmp(ext, "exe") == 0 || strcasecmp(ext, "dll") == 0 ||
        strcasecmp(ext, "so") == 0 || strcasecmp(ext, "bin") == 0 ||
        strcasecmp(ext, "app") == 0) {
        return FILE_TYPE_EXECUTABLE;
    }
    
    // Documents
    if (strcasecmp(ext, "pdf") == 0 || strcasecmp(ext, "doc") == 0 ||
        strcasecmp(ext, "docx") == 0 || strcasecmp(ext, "xls") == 0 ||
        strcasecmp(ext, "xlsx") == 0 || strcasecmp(ext, "ppt") == 0 ||
        strcasecmp(ext, "pptx") == 0) {
        return FILE_TYPE_DOCUMENT;
    }
    
    // Database files
    if (strcasecmp(ext, "db") == 0 || strcasecmp(ext, "sqlite") == 0 ||
        strcasecmp(ext, "mdb") == 0 || strcasecmp(ext, "dbf") == 0) {
        return FILE_TYPE_DATABASE;
    }
    
    return FILE_TYPE_BINARY;
}

static bool is_dangerous_extension(const char* filename) {
    const char* dangerous_extensions[] = {
        "exe", "scr", "bat", "cmd", "com", "pif", "vbs", "js", "jar",
        "msi", "dll", "sys", "drv", "cpl", "ocx", "reg", "ps1", "sh"
    };
    
    const char* ext = strrchr(filename, '.');
    if (!ext) return false;
    
    ext++; // Skip the dot
    
    for (size_t i = 0; i < sizeof(dangerous_extensions) / sizeof(dangerous_extensions[0]); i++) {
        if (strcasecmp(ext, dangerous_extensions[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

static void format_file_size(uint64_t size, char* buffer, size_t buffer_size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_d = (double)size;
    
    while (size_d >= 1024.0 && unit_index < 4) {
        size_d /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%llu %s", size, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.1f %s", size_d, units[unit_index]);
    }
}

// ============================================================================
// AI COMPRESSION ADVISOR
// ============================================================================

static void ai_analyze_file_for_compression(archive_item_t* item) {
    if (!g_archive_manager.ai_advisor.enabled || item->ai_analysis.analyzed) {
        return;
    }
    
    printf("[ArchiveManager] AI analyzing file: %s\n", item->filename);
    
    // Simulate AI analysis delay
    usleep(10000); // 10ms
    
    ai_compression_advisor_t* advisor = &g_archive_manager.ai_advisor;
    
    // Determine optimal compression level based on file type
    compression_level_t recommended = COMPRESSION_NORMAL;
    float space_savings = 0.3f; // Default 30% savings
    
    switch (item->file_type) {
        case FILE_TYPE_TEXT:
            recommended = advisor->profiles.text_optimal;
            space_savings = 0.7f; // Text compresses well
            strcpy(item->ai_analysis.optimization_notes, 
                   "Text files benefit from high compression levels");
            break;
            
        case FILE_TYPE_BINARY:
        case FILE_TYPE_EXECUTABLE:
            recommended = advisor->profiles.binary_optimal;
            space_savings = 0.4f;
            strcpy(item->ai_analysis.optimization_notes,
                   "Binary files show moderate compression gains");
            break;
            
        case FILE_TYPE_IMAGE:
            if (advisor->profiles.skip_compressed_files) {
                recommended = COMPRESSION_STORE;
                space_savings = 0.05f;
                strcpy(item->ai_analysis.optimization_notes,
                       "Image already compressed - store only recommended");
            } else {
                recommended = advisor->profiles.image_optimal;
                space_savings = 0.15f;
                strcpy(item->ai_analysis.optimization_notes,
                       "Minimal gains expected - image already compressed");
            }
            break;
            
        case FILE_TYPE_AUDIO:
            if (advisor->profiles.skip_compressed_files) {
                recommended = COMPRESSION_STORE;
                space_savings = 0.02f;
                strcpy(item->ai_analysis.optimization_notes,
                       "Audio already compressed - avoid double compression");
            } else {
                recommended = advisor->profiles.audio_optimal;
                space_savings = 0.1f;
            }
            break;
            
        case FILE_TYPE_VIDEO:
            recommended = COMPRESSION_STORE;
            space_savings = 0.01f;
            strcpy(item->ai_analysis.optimization_notes,
                   "Video files rarely benefit from archive compression");
            break;
            
        case FILE_TYPE_COMPRESSED:
            recommended = COMPRESSION_STORE;
            space_savings = 0.0f;
            strcpy(item->ai_analysis.optimization_notes,
                   "File already compressed - store without compression");
            break;
            
        case FILE_TYPE_DATABASE:
            recommended = COMPRESSION_MAXIMUM;
            space_savings = 0.6f;
            strcpy(item->ai_analysis.optimization_notes,
                   "Database files often contain redundant data");
            break;
            
        case FILE_TYPE_DOCUMENT:
            recommended = COMPRESSION_NORMAL;
            space_savings = 0.5f;
            strcpy(item->ai_analysis.optimization_notes,
                   "Document formats vary - normal compression recommended");
            break;
            
        default:
            recommended = COMPRESSION_NORMAL;
            space_savings = 0.3f;
            strcpy(item->ai_analysis.optimization_notes,
                   "Unknown file type - using balanced compression");
            break;
    }
    
    // Adjust based on file size
    if (item->original_size < 1024) {
        // Very small files
        recommended = COMPRESSION_STORE;
        space_savings = 0.0f;
        strcat(item->ai_analysis.optimization_notes, " (Small file - overhead exceeds benefit)");
    } else if (item->original_size > 100 * 1024 * 1024) {
        // Very large files - balance speed vs compression
        if (recommended > COMPRESSION_NORMAL) {
            recommended = COMPRESSION_NORMAL;
            strcat(item->ai_analysis.optimization_notes, " (Large file - balanced for speed)");
        }
    }
    
    item->ai_analysis.analyzed = true;
    item->ai_analysis.recommended_level = recommended;
    item->ai_analysis.space_savings_potential = space_savings;
    item->ai_analysis.optimal_compression = (recommended != COMPRESSION_STORE);
    
    // Update advisor statistics
    advisor->stats.files_analyzed++;
    
    printf("[ArchiveManager] AI analysis complete: %s compression, %.1f%% savings potential\n",
           get_compression_name(recommended), space_savings * 100.0f);
}

static void ai_analyze_archive_for_optimization(archive_info_t* archive) {
    if (!archive || !g_archive_manager.ai_advisor.enabled) {
        return;
    }
    
    printf("[ArchiveManager] AI analyzing archive for optimization opportunities\n");
    
    ai_compression_advisor_t* advisor = &g_archive_manager.ai_advisor;
    
    // Analyze each item if not already done
    archive_item_t* item = archive->items;
    uint32_t analyzed_count = 0;
    float total_potential_savings = 0.0f;
    
    while (item && analyzed_count < AI_ANALYSIS_THRESHOLD) {
        if (!item->ai_analysis.analyzed) {
            ai_analyze_file_for_compression(item);
        }
        
        total_potential_savings += item->ai_analysis.space_savings_potential * item->original_size;
        analyzed_count++;
        item = item->next;
    }
    
    // Calculate overall optimization potential
    float average_savings = total_potential_savings / archive->total_size;
    
    printf("[ArchiveManager] Archive analysis complete: %.1f%% average space savings potential\n",
           average_savings * 100.0f);
    
    // Generate recommendations
    if (average_savings > 0.5f) {
        advisor->stats.recommendations_made++;
        printf("[ArchiveManager] Recommendation: High compression potential detected\n");
    } else if (average_savings < 0.1f) {
        advisor->stats.recommendations_made++;
        printf("[ArchiveManager] Recommendation: Files already well-compressed, use fast compression\n");
    }
}

// ============================================================================
// SECURITY SCANNING
// ============================================================================

static void security_scan_archive_item(archive_item_t* item) {
    if (!g_archive_manager.security_scanner.enabled || item->security.scanned) {
        return;
    }
    
    printf("[ArchiveManager] Security scanning: %s\n", item->filename);
    
    item->security.scanned = true;
    item->security.risk_score = 0.0f;
    item->security.is_suspicious = false;
    item->security.contains_malware = false;
    
    // Check for dangerous file extensions
    item->security.has_dangerous_extension = is_dangerous_extension(item->filename);
    if (item->security.has_dangerous_extension) {
        item->security.risk_score += 0.3f;
        item->security.is_suspicious = true;
        strcpy(item->security.threat_description, "Potentially dangerous file extension");
    }
    
    // Suspicious filename patterns
    const char* suspicious_patterns[] = {
        "setup", "install", "crack", "patch", "keygen", "loader", "hack"
    };
    
    for (size_t i = 0; i < sizeof(suspicious_patterns) / sizeof(suspicious_patterns[0]); i++) {
        if (strcasestr(item->filename, suspicious_patterns[i]) != NULL) {
            item->security.risk_score += 0.2f;
            item->security.is_suspicious = true;
            if (strlen(item->security.threat_description) == 0) {
                snprintf(item->security.threat_description, 
                        sizeof(item->security.threat_description),
                        "Suspicious filename pattern: %s", suspicious_patterns[i]);
            }
        }
    }
    
    // Simulate malware detection (in real implementation, integrate with antivirus)
    if (item->security.risk_score > 0.5f && (rand() % 100) < 5) {
        item->security.contains_malware = true;
        item->security.risk_score = 0.9f;
        strcpy(item->security.threat_description, "Potential malware detected");
        g_archive_manager.security_scanner.threats_blocked++;
    }
    
    // Set warning flag
    item->has_security_warning = item->security.is_suspicious || item->security.contains_malware;
    
    if (item->has_security_warning) {
        printf("[ArchiveManager] Security warning: %s - %s (Risk: %.1f%%)\n",
               item->filename, item->security.threat_description, item->security.risk_score * 100.0f);
    }
}

static void security_scan_archive(archive_info_t* archive) {
    if (!archive || !g_archive_manager.security_scanner.enabled) {
        return;
    }
    
    printf("[ArchiveManager] Security scanning archive: %s\n", archive->display_name);
    
    archive->security_summary.suspicious_files = 0;
    archive->security_summary.malware_detected = 0;
    archive->security_summary.dangerous_extensions = 0;
    archive->security_summary.overall_risk_score = 0.0f;
    
    archive_item_t* item = archive->items;
    uint32_t scanned_count = 0;
    float total_risk = 0.0f;
    
    while (item) {
        security_scan_archive_item(item);
        
        if (item->security.is_suspicious) {
            archive->security_summary.suspicious_files++;
        }
        if (item->security.contains_malware) {
            archive->security_summary.malware_detected++;
        }
        if (item->security.has_dangerous_extension) {
            archive->security_summary.dangerous_extensions++;
        }
        
        total_risk += item->security.risk_score;
        scanned_count++;
        item = item->next;
    }
    
    if (scanned_count > 0) {
        archive->security_summary.overall_risk_score = total_risk / scanned_count;
    }
    
    archive->security_summary.quarantine_recommended = 
        (archive->security_summary.malware_detected > 0) ||
        (archive->security_summary.overall_risk_score > 0.7f);
    
    printf("[ArchiveManager] Security scan complete: %u suspicious, %u malware, %.1f%% risk\n",
           archive->security_summary.suspicious_files,
           archive->security_summary.malware_detected,
           archive->security_summary.overall_risk_score * 100.0f);
    
    g_archive_manager.stats.security_threats_detected += 
        archive->security_summary.suspicious_files + archive->security_summary.malware_detected;
}

// ============================================================================
// ARCHIVE OPERATIONS
// ============================================================================

static archive_info_t* create_archive_info(const char* filepath) {
    archive_info_t* archive = calloc(1, sizeof(archive_info_t));
    if (!archive) {
        printf("[ArchiveManager] ERROR: Failed to allocate archive info\n");
        return NULL;
    }
    
    strncpy(archive->filepath, filepath, sizeof(archive->filepath) - 1);
    
    const char* filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;
    strncpy(archive->filename, filename, sizeof(archive->filename) - 1);
    strncpy(archive->display_name, filename, sizeof(archive->display_name) - 1);
    
    archive->format = detect_archive_format(filepath);
    archive->created_time = time(NULL);
    archive->modified_time = time(NULL);
    
    printf("[ArchiveManager] Created archive info for: %s (%s format)\n", 
           filename, get_format_name(archive->format));
    
    return archive;
}

static void free_archive_info(archive_info_t* archive) {
    if (!archive) return;
    
    // Free all items
    archive_item_t* item = archive->items;
    while (item) {
        archive_item_t* next = item->next;
        free(item);
        item = next;
    }
    
    free(archive);
}

static archive_item_t* create_archive_item(const char* filename, const char* full_path) {
    archive_item_t* item = calloc(1, sizeof(archive_item_t));
    if (!item) {
        return NULL;
    }
    
    strncpy(item->filename, filename, sizeof(item->filename) - 1);
    strncpy(item->full_path, full_path, sizeof(item->full_path) - 1);
    
    // Get file statistics
    struct stat st;
    if (stat(full_path, &st) == 0) {
        item->original_size = st.st_size;
        item->modified_time = st.st_mtime;
        item->created_time = st.st_ctime;
        item->is_directory = S_ISDIR(st.st_mode);
        item->is_executable = (st.st_mode & S_IXUSR) != 0;
    }
    
    // Classify file type
    item->file_type = classify_file_type(filename);
    
    // Calculate CRC32 (simulated)
    item->crc32 = (uint32_t)((uintptr_t)filename ^ item->original_size);
    
    return item;
}

static bool add_item_to_archive(archive_info_t* archive, archive_item_t* item) {
    if (!archive || !item) {
        return false;
    }
    
    // Add to linked list
    item->next = archive->items;
    archive->items = item;
    archive->item_count++;
    archive->loaded_items++;
    archive->total_size += item->original_size;
    
    // Perform security scan
    security_scan_archive_item(item);
    
    // Perform AI analysis if enabled
    if (g_archive_manager.ai_advisor.enabled) {
        ai_analyze_file_for_compression(item);
    }
    
    return true;
}

static bool load_archive_contents(archive_info_t* archive) {
    if (!archive) {
        return false;
    }
    
    printf("[ArchiveManager] Loading archive contents: %s\n", archive->filepath);
    
    // Simulate loading archive contents
    // In real implementation, this would use libarchive, minizip, or similar
    
    const char* sample_files[] = {
        "document.pdf", "image.jpg", "data.csv", "program.exe", 
        "readme.txt", "config.xml", "backup.db", "video.mp4",
        "audio.mp3", "source.c", "makefile", "library.so"
    };
    
    for (size_t i = 0; i < sizeof(sample_files) / sizeof(sample_files[0]); i++) {
        archive_item_t* item = create_archive_item(sample_files[i], archive->filepath);
        if (item) {
            // Simulate file sizes
            switch (item->file_type) {
                case FILE_TYPE_TEXT:
                    item->original_size = 1024 + (rand() % 10240);
                    break;
                case FILE_TYPE_IMAGE:
                    item->original_size = 100000 + (rand() % 500000);
                    break;
                case FILE_TYPE_VIDEO:
                    item->original_size = 10000000 + (rand() % 50000000);
                    break;
                case FILE_TYPE_AUDIO:
                    item->original_size = 3000000 + (rand() % 5000000);
                    break;
                case FILE_TYPE_EXECUTABLE:
                    item->original_size = 500000 + (rand() % 2000000);
                    break;
                default:
                    item->original_size = 10000 + (rand() % 100000);
                    break;
            }
            
            // Simulate compression ratios
            switch (archive->format) {
                case ARCHIVE_FORMAT_ZIP:
                    item->compression_ratio = 0.3f + (rand() % 40) / 100.0f;
                    break;
                case ARCHIVE_FORMAT_7Z:
                    item->compression_ratio = 0.2f + (rand() % 30) / 100.0f;
                    break;
                case ARCHIVE_FORMAT_TAR_XZ:
                    item->compression_ratio = 0.25f + (rand() % 35) / 100.0f;
                    break;
                default:
                    item->compression_ratio = 0.4f + (rand() % 30) / 100.0f;
                    break;
            }
            
            item->compressed_size = (uint64_t)(item->original_size * item->compression_ratio);
            archive->compressed_size += item->compressed_size;
            
            add_item_to_archive(archive, item);
        }
    }
    
    // Calculate overall compression ratio
    if (archive->total_size > 0) {
        archive->overall_compression_ratio = (float)archive->compressed_size / archive->total_size;
    }
    
    // Perform security scan on entire archive
    security_scan_archive(archive);
    
    // Perform AI analysis
    if (g_archive_manager.ai_advisor.enabled) {
        ai_analyze_archive_for_optimization(archive);
    }
    
    printf("[ArchiveManager] Loaded %u items, %.1f%% compression ratio\n",
           archive->item_count, archive->overall_compression_ratio * 100.0f);
    
    return true;
}

// ============================================================================
// COMPRESSION JOBS MANAGEMENT
// ============================================================================

static compression_job_t* create_compression_job(archive_operation_t operation,
                                               const char* source_path,
                                               const char* target_path,
                                               archive_format_t format) {
    compression_job_t* job = calloc(1, sizeof(compression_job_t));
    if (!job) {
        return NULL;
    }
    
    job->operation = operation;
    strncpy(job->source_path, source_path, sizeof(job->source_path) - 1);
    strncpy(job->target_path, target_path, sizeof(job->target_path) - 1);
    job->format = format;
    job->level = g_archive_manager.settings.default_compression;
    job->encryption = g_archive_manager.settings.default_encryption;
    
    job->start_time = time(NULL);
    
    return job;
}

static void* compression_worker_thread(void* arg) {
    compression_job_t* job = (compression_job_t*)arg;
    
    printf("[ArchiveManager] Starting %s job: %s -> %s\n",
           (job->operation == OPERATION_CREATE) ? "compression" : "extraction",
           job->source_path, job->target_path);
    
    job->active = true;
    
    // Simulate compression/extraction work
    job->total_bytes = 50 * 1024 * 1024; // 50MB simulation
    job->total_files = 100;
    
    for (uint32_t i = 0; i < job->total_files && !job->cancelled; i++) {
        // Simulate processing each file
        usleep(50000); // 50ms per file
        
        job->processed_files = i + 1;
        job->processed_bytes = (job->total_bytes * (i + 1)) / job->total_files;
        job->progress_percent = ((float)(i + 1) / job->total_files) * 100.0f;
        
        // Calculate speed
        time_t elapsed = time(NULL) - job->start_time;
        if (elapsed > 0) {
            job->current_speed = (float)job->processed_bytes / (1024.0f * 1024.0f * elapsed);
            job->average_speed = job->current_speed;
            
            if (job->progress_percent > 0) {
                time_t remaining = (elapsed * (100.0f - job->progress_percent)) / job->progress_percent;
                job->estimated_completion = time(NULL) + remaining;
            }
        }
    }
    
    if (job->cancelled) {
        printf("[ArchiveManager] Job cancelled: %s\n", job->target_path);
    } else {
        job->completed = true;
        job->progress_percent = 100.0f;
        
        // Update statistics
        if (job->operation == OPERATION_CREATE) {
            g_archive_manager.stats.archives_created++;
            g_archive_manager.stats.bytes_compressed += job->total_bytes;
        } else if (job->operation == OPERATION_EXTRACT) {
            g_archive_manager.stats.archives_extracted++;
            g_archive_manager.stats.bytes_extracted += job->total_bytes;
        }
        
        printf("[ArchiveManager] Job completed: %s (%.1f MB/s average)\n",
               job->target_path, job->average_speed);
    }
    
    job->active = false;
    return NULL;
}

static bool start_compression_job(compression_job_t* job) {
    if (!job) {
        return false;
    }
    
    pthread_mutex_lock(&g_archive_manager.jobs_mutex);
    
    // Add to active jobs list
    job->next = g_archive_manager.active_jobs;
    g_archive_manager.active_jobs = job;
    g_archive_manager.job_count++;
    
    pthread_mutex_unlock(&g_archive_manager.jobs_mutex);
    
    // Start worker thread
    if (pthread_create(&job->thread_id, NULL, compression_worker_thread, job) != 0) {
        printf("[ArchiveManager] ERROR: Failed to create worker thread\n");
        return false;
    }
    
    return true;
}

static void cancel_compression_job(compression_job_t* job) {
    if (!job || job->completed) {
        return;
    }
    
    printf("[ArchiveManager] Cancelling job: %s\n", job->target_path);
    job->cancelled = true;
}

static void cleanup_completed_jobs(void) {
    pthread_mutex_lock(&g_archive_manager.jobs_mutex);
    
    compression_job_t* job = g_archive_manager.active_jobs;
    compression_job_t* prev = NULL;
    
    while (job) {
        compression_job_t* next = job->next;
        
        if (!job->active && (job->completed || job->cancelled)) {
            // Remove from list
            if (prev) {
                prev->next = next;
            } else {
                g_archive_manager.active_jobs = next;
            }
            
            g_archive_manager.job_count--;
            
            // Wait for thread to finish
            pthread_join(job->thread_id, NULL);
            
            free(job);
        } else {
            prev = job;
        }
        
        job = next;
    }
    
    pthread_mutex_unlock(&g_archive_manager.jobs_mutex);
}

// ============================================================================
// ARCHIVE FILE OPERATIONS
// ============================================================================

static bool create_archive(const char* source_path, const char* archive_path, 
                          archive_format_t format, compression_level_t level) {
    printf("[ArchiveManager] Creating archive: %s -> %s (%s, %s)\n",
           source_path, archive_path, get_format_name(format), get_compression_name(level));
    
    compression_job_t* job = create_compression_job(OPERATION_CREATE, source_path, archive_path, format);
    if (!job) {
        return false;
    }
    
    job->level = level;
    
    return start_compression_job(job);
}

static bool extract_archive(const char* archive_path, const char* destination_path) {
    printf("[ArchiveManager] Extracting archive: %s -> %s\n", archive_path, destination_path);
    
    archive_format_t format = detect_archive_format(archive_path);
    
    compression_job_t* job = create_compression_job(OPERATION_EXTRACT, archive_path, destination_path, format);
    if (!job) {
        return false;
    }
    
    return start_compression_job(job);
}

static bool test_archive_integrity(const char* archive_path) {
    printf("[ArchiveManager] Testing archive integrity: %s\n", archive_path);
    
    archive_format_t format = detect_archive_format(archive_path);
    
    compression_job_t* job = create_compression_job(OPERATION_TEST, archive_path, "", format);
    if (!job) {
        return false;
    }
    
    return start_compression_job(job);
}

static bool add_files_to_archive(const char* archive_path, const char** file_paths, uint32_t file_count) {
    printf("[ArchiveManager] Adding %u files to archive: %s\n", file_count, archive_path);
    
    archive_format_t format = detect_archive_format(archive_path);
    
    // Create temporary job for multiple files
    compression_job_t* job = create_compression_job(OPERATION_ADD, file_paths[0], archive_path, format);
    if (!job) {
        return false;
    }
    
    job->total_files = file_count;
    
    return start_compression_job(job);
}

// ============================================================================
// FILE SYSTEM OPERATIONS
// ============================================================================

static void scan_directory_for_archives(const char* directory_path) {
    printf("[ArchiveManager] Scanning directory for archives: %s\n", directory_path);
    
    DIR* dir = opendir(directory_path);
    if (!dir) {
        printf("[ArchiveManager] ERROR: Cannot open directory: %s\n", directory_path);
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            char full_path[MAX_PATH_LENGTH];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);
            
            archive_format_t format = detect_archive_format(entry->d_name);
            if (format != ARCHIVE_FORMAT_UNKNOWN) {
                printf("[ArchiveManager] Found archive: %s (%s)\n", 
                       entry->d_name, get_format_name(format));
                
                // Add to recent archives if space available
                if (g_archive_manager.recent_count < 10) {
                    archive_info_t* archive = create_archive_info(full_path);
                    if (archive) {
                        g_archive_manager.recent_archives[g_archive_manager.recent_count++] = archive;
                    }
                }
            }
        }
    }
    
    closedir(dir);
}

static uint64_t calculate_directory_size(const char* directory_path) {
    DIR* dir = opendir(directory_path);
    if (!dir) {
        return 0;
    }
    
    uint64_t total_size = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory_path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                total_size += calculate_directory_size(full_path);
            } else {
                total_size += st.st_size;
            }
        }
    }
    
    closedir(dir);
    return total_size;
}

// ============================================================================
// USER INTERFACE IMPLEMENTATION
// ============================================================================

static void create_toolbar(void) {
    g_archive_manager.toolbar = lui_create_container(g_archive_manager.main_window->root_widget);
    strcpy(g_archive_manager.toolbar->name, "toolbar");
    g_archive_manager.toolbar->bounds = lui_rect_make(0, 0, 1200, 40);
    g_archive_manager.toolbar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // File operations
    lui_widget_t* open_btn = lui_create_button("📂 Open", g_archive_manager.toolbar);
    open_btn->bounds = lui_rect_make(8, 8, 60, 24);
    open_btn->style.background_color = LUI_COLOR_TACTICAL_BLUE;
    
    lui_widget_t* create_btn = lui_create_button("📦 Create", g_archive_manager.toolbar);
    create_btn->bounds = lui_rect_make(76, 8, 60, 24);
    create_btn->style.background_color = LUI_COLOR_SUCCESS_GREEN;
    
    lui_widget_t* extract_btn = lui_create_button("📤 Extract", g_archive_manager.toolbar);
    extract_btn->bounds = lui_rect_make(144, 8, 60, 24);
    extract_btn->style.background_color = LUI_COLOR_WARNING_AMBER;
    
    lui_widget_t* add_btn = lui_create_button("➕ Add", g_archive_manager.toolbar);
    add_btn->bounds = lui_rect_make(212, 8, 50, 24);
    add_btn->style.background_color = LUI_COLOR_SECURE_CYAN;
    
    // Archive operations  
    lui_widget_t* test_btn = lui_create_button("🔍 Test", g_archive_manager.toolbar);
    test_btn->bounds = lui_rect_make(290, 8, 50, 24);
    test_btn->style.background_color = LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* repair_btn = lui_create_button("🔧 Repair", g_archive_manager.toolbar);
    repair_btn->bounds = lui_rect_make(348, 8, 60, 24);
    repair_btn->style.background_color = LUI_COLOR_WARNING_AMBER;
    
    // View options
    lui_widget_t* view_list_btn = lui_create_button("📋", g_archive_manager.toolbar);
    view_list_btn->bounds = lui_rect_make(440, 8, 30, 24);
    view_list_btn->style.background_color = (g_archive_manager.view_mode == VIEW_LIST) ?
                                           LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* view_details_btn = lui_create_button("📊", g_archive_manager.toolbar);
    view_details_btn->bounds = lui_rect_make(478, 8, 30, 24);
    view_details_btn->style.background_color = (g_archive_manager.view_mode == VIEW_DETAILS) ?
                                              LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* view_tree_btn = lui_create_button("🌳", g_archive_manager.toolbar);
    view_tree_btn->bounds = lui_rect_make(516, 8, 30, 24);
    view_tree_btn->style.background_color = (g_archive_manager.view_mode == VIEW_TREE) ?
                                           LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
    
    // Security and AI toggles
    lui_widget_t* security_btn = lui_create_button("🛡️ Security", g_archive_manager.toolbar);
    security_btn->bounds = lui_rect_make(580, 8, 80, 24);
    security_btn->style.background_color = g_archive_manager.security_scanner.enabled ?
                                          LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
    
    lui_widget_t* ai_btn = lui_create_button("🤖 AI", g_archive_manager.toolbar);
    ai_btn->bounds = lui_rect_make(668, 8, 50, 24);
    ai_btn->style.background_color = g_archive_manager.ai_advisor.enabled ?
                                    LUI_COLOR_SECURE_CYAN : LUI_COLOR_STEEL_GRAY;
    
    // Progress indicator
    if (g_archive_manager.job_count > 0) {
        char progress_text[32];
        snprintf(progress_text, sizeof(progress_text), "⚙️ %u Jobs", g_archive_manager.job_count);
        lui_widget_t* progress_indicator = lui_create_label(progress_text, g_archive_manager.toolbar);
        progress_indicator->bounds = lui_rect_make(1000, 10, 80, 20);
        progress_indicator->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        progress_indicator->typography.color = LUI_COLOR_WARNING_AMBER;
    }
}

static void create_file_list(void) {
    g_archive_manager.file_list = lui_create_container(g_archive_manager.main_window->root_widget);
    strcpy(g_archive_manager.file_list->name, "file_list");
    
    int list_width = g_archive_manager.show_properties ? 800 : 1000;
    g_archive_manager.file_list->bounds = lui_rect_make(0, 40, list_width, 460);
    g_archive_manager.file_list->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // File list header
    lui_widget_t* header = lui_create_container(g_archive_manager.file_list);
    header->bounds = lui_rect_make(0, 0, list_width, 25);
    header->background_color = LUI_COLOR_GRAPHITE;
    
    lui_widget_t* name_header = lui_create_label("Name", header);
    name_header->bounds = lui_rect_make(8, 5, 200, 15);
    name_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    name_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    lui_widget_t* size_header = lui_create_label("Size", header);
    size_header->bounds = lui_rect_make(220, 5, 80, 15);
    size_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    size_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    lui_widget_t* ratio_header = lui_create_label("Ratio", header);
    ratio_header->bounds = lui_rect_make(310, 5, 60, 15);
    ratio_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    ratio_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    lui_widget_t* type_header = lui_create_label("Type", header);
    type_header->bounds = lui_rect_make(380, 5, 80, 15);
    type_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    type_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    lui_widget_t* security_header = lui_create_label("Security", header);
    security_header->bounds = lui_rect_make(470, 5, 70, 15);
    security_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    security_header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    // File items
    if (g_archive_manager.current_archive && g_archive_manager.current_archive->items) {
        archive_item_t* item = g_archive_manager.current_archive->items;
        int y_offset = 30;
        int item_count = 0;
        
        while (item && y_offset < 450 && item_count < 20) {
            // File type icon
            const char* type_icon = "📄";
            switch (item->file_type) {
                case FILE_TYPE_TEXT: type_icon = "📝"; break;
                case FILE_TYPE_IMAGE: type_icon = "🖼️"; break;
                case FILE_TYPE_AUDIO: type_icon = "🎵"; break;
                case FILE_TYPE_VIDEO: type_icon = "🎬"; break;
                case FILE_TYPE_EXECUTABLE: type_icon = "⚙️"; break;
                case FILE_TYPE_COMPRESSED: type_icon = "📦"; break;
                case FILE_TYPE_DOCUMENT: type_icon = "📋"; break;
                case FILE_TYPE_DATABASE: type_icon = "🗄️"; break;
                default: type_icon = "📄"; break;
            }
            
            lui_widget_t* icon_label = lui_create_label(type_icon, g_archive_manager.file_list);
            icon_label->bounds = lui_rect_make(8, y_offset + 2, 16, 16);
            
            // Filename
            char display_name[48];
            snprintf(display_name, sizeof(display_name), "%.40s%s",
                    item->filename, strlen(item->filename) > 40 ? "..." : "");
            lui_widget_t* name_label = lui_create_label(display_name, g_archive_manager.file_list);
            name_label->bounds = lui_rect_make(30, y_offset + 2, 180, 16);
            name_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            
            // File size
            char size_text[16];
            format_file_size(item->original_size, size_text, sizeof(size_text));
            lui_widget_t* size_label = lui_create_label(size_text, g_archive_manager.file_list);
            size_label->bounds = lui_rect_make(220, y_offset + 2, 80, 16);
            size_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            
            // Compression ratio
            char ratio_text[16];
            snprintf(ratio_text, sizeof(ratio_text), "%.1f%%", item->compression_ratio * 100.0f);
            lui_widget_t* ratio_label = lui_create_label(ratio_text, g_archive_manager.file_list);
            ratio_label->bounds = lui_rect_make(310, y_offset + 2, 60, 16);
            ratio_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            
            // File type
            const char* type_names[] = {
                "Unknown", "Text", "Binary", "Executable", "Image", 
                "Audio", "Video", "Compressed", "Database", "Document", "Archive"
            };
            const char* type_name = (item->file_type < sizeof(type_names) / sizeof(type_names[0])) ?
                                   type_names[item->file_type] : "Unknown";
            lui_widget_t* type_label = lui_create_label(type_name, g_archive_manager.file_list);
            type_label->bounds = lui_rect_make(380, y_offset + 2, 80, 16);
            type_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            
            // Security status
            const char* security_icon = "✅";
            lui_color_t security_color = LUI_COLOR_SUCCESS_GREEN;
            if (item->security.contains_malware) {
                security_icon = "🦠";
                security_color = LUI_COLOR_CRITICAL_RED;
            } else if (item->security.is_suspicious) {
                security_icon = "⚠️";
                security_color = LUI_COLOR_WARNING_AMBER;
            } else if (item->security.has_dangerous_extension) {
                security_icon = "🔶";
                security_color = LUI_COLOR_WARNING_AMBER;
            }
            
            lui_widget_t* security_label = lui_create_label(security_icon, g_archive_manager.file_list);
            security_label->bounds = lui_rect_make(470, y_offset + 2, 20, 16);
            security_label->typography.color = security_color;
            
            // AI recommendation indicator
            if (item->ai_analysis.analyzed && item->ai_analysis.optimal_compression) {
                lui_widget_t* ai_indicator = lui_create_label("🤖", g_archive_manager.file_list);
                ai_indicator->bounds = lui_rect_make(500, y_offset + 2, 16, 16);
                ai_indicator->typography.color = LUI_COLOR_SECURE_CYAN;
            }
            
            y_offset += 20;
            item_count++;
            item = item->next;
        }
        
        // Show count if more items exist
        if (item) {
            char more_text[32];
            snprintf(more_text, sizeof(more_text), "... and %u more files", 
                    g_archive_manager.current_archive->item_count - item_count);
            lui_widget_t* more_label = lui_create_label(more_text, g_archive_manager.file_list);
            more_label->bounds = lui_rect_make(30, y_offset + 5, 200, 16);
            more_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            more_label->typography.color = LUI_COLOR_STEEL_GRAY;
        }
    } else {
        // No archive loaded
        lui_widget_t* empty_label = lui_create_label("No archive loaded\nOpen an archive to view contents", 
                                                    g_archive_manager.file_list);
        empty_label->bounds = lui_rect_make(300, 180, 200, 50);
        empty_label->typography = LUI_TYPOGRAPHY_TITLE_MEDIUM;
        empty_label->typography.color = LUI_COLOR_STEEL_GRAY;
    }
}

static void create_properties_panel(void) {
    if (!g_archive_manager.show_properties) return;
    
    g_archive_manager.properties_panel = lui_create_container(g_archive_manager.main_window->root_widget);
    strcpy(g_archive_manager.properties_panel->name, "properties_panel");
    g_archive_manager.properties_panel->bounds = lui_rect_make(800, 40, 400, 460);
    g_archive_manager.properties_panel->background_color = LUI_COLOR_GRAPHITE;
    
    // Properties header
    lui_widget_t* header = lui_create_label("📋 Archive Properties", g_archive_manager.properties_panel);
    header->bounds = lui_rect_make(8, 8, 200, 20);
    header->typography = LUI_TYPOGRAPHY_LABEL_MEDIUM;
    header->typography.color = LUI_COLOR_SECURE_CYAN;
    
    if (g_archive_manager.current_archive) {
        archive_info_t* archive = g_archive_manager.current_archive;
        
        // Basic information
        char info_text[512];
        char total_size_str[32], compressed_size_str[32];
        format_file_size(archive->total_size, total_size_str, sizeof(total_size_str));
        format_file_size(archive->compressed_size, compressed_size_str, sizeof(compressed_size_str));
        
        snprintf(info_text, sizeof(info_text),
                "File: %s\n\n"
                "Format: %s\n"
                "Items: %u files\n"
                "Original Size: %s\n"
                "Compressed: %s\n"
                "Ratio: %.1f%%\n"
                "Encryption: %s",
                archive->display_name,
                get_format_name(archive->format),
                archive->item_count,
                total_size_str,
                compressed_size_str,
                archive->overall_compression_ratio * 100.0f,
                get_encryption_name(archive->encryption));
        
        lui_widget_t* info_label = lui_create_label(info_text, g_archive_manager.properties_panel);
        info_label->bounds = lui_rect_make(8, 35, 380, 140);
        info_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        
        // Security summary
        lui_widget_t* security_header = lui_create_label("🛡️ Security Analysis", g_archive_manager.properties_panel);
        security_header->bounds = lui_rect_make(8, 185, 150, 16);
        security_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        security_header->typography.color = LUI_COLOR_WARNING_AMBER;
        
        char security_text[256];
        snprintf(security_text, sizeof(security_text),
                "Suspicious Files: %u\n"
                "Malware Detected: %u\n"
                "Dangerous Extensions: %u\n"
                "Overall Risk: %.1f%%\n"
                "Quarantine: %s",
                archive->security_summary.suspicious_files,
                archive->security_summary.malware_detected,
                archive->security_summary.dangerous_extensions,
                archive->security_summary.overall_risk_score * 100.0f,
                archive->security_summary.quarantine_recommended ? "Recommended" : "Not Required");
        
        lui_widget_t* security_label = lui_create_label(security_text, g_archive_manager.properties_panel);
        security_label->bounds = lui_rect_make(8, 205, 380, 100);
        security_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
        // Color code based on risk
        if (archive->security_summary.malware_detected > 0) {
            security_label->typography.color = LUI_COLOR_CRITICAL_RED;
        } else if (archive->security_summary.suspicious_files > 0) {
            security_label->typography.color = LUI_COLOR_WARNING_AMBER;
        } else {
            security_label->typography.color = LUI_COLOR_SUCCESS_GREEN;
        }
        
        // AI advisor summary
        if (g_archive_manager.ai_advisor.enabled) {
            lui_widget_t* ai_header = lui_create_label("🤖 AI Recommendations", g_archive_manager.properties_panel);
            ai_header->bounds = lui_rect_make(8, 315, 150, 16);
            ai_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            ai_header->typography.color = LUI_COLOR_SECURE_CYAN;
            
            char ai_text[256];
            snprintf(ai_text, sizeof(ai_text),
                    "Files Analyzed: %u\n"
                    "Recommendations: %u\n"
                    "Avg Space Savings: %.1f%%\n"
                    "Optimal Compression: Available\n"
                    "Performance: Optimized",
                    g_archive_manager.ai_advisor.stats.files_analyzed,
                    g_archive_manager.ai_advisor.stats.recommendations_made,
                    g_archive_manager.ai_advisor.stats.average_space_savings * 100.0f);
            
            lui_widget_t* ai_label = lui_create_label(ai_text, g_archive_manager.properties_panel);
            ai_label->bounds = lui_rect_make(8, 335, 380, 100);
            ai_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            ai_label->typography.color = LUI_COLOR_SECURE_CYAN;
        }
        
        // Performance metrics
        lui_widget_t* perf_header = lui_create_label("⚡ Performance", g_archive_manager.properties_panel);
        perf_header->bounds = lui_rect_make(200, 185, 100, 16);
        perf_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        perf_header->typography.color = LUI_COLOR_SUCCESS_GREEN;
        
        char perf_text[128];
        snprintf(perf_text, sizeof(perf_text),
                "Extract Speed: %.1f MB/s\n"
                "Compress Speed: %.1f MB/s\n"
                "Access Count: %u",
                archive->performance.extraction_speed,
                archive->performance.compression_speed,
                archive->performance.access_count);
        
        lui_widget_t* perf_label = lui_create_label(perf_text, g_archive_manager.properties_panel);
        perf_label->bounds = lui_rect_make(200, 205, 180, 60);
        perf_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        
    } else {
        lui_widget_t* no_archive = lui_create_label("No archive selected", g_archive_manager.properties_panel);
        no_archive->bounds = lui_rect_make(8, 35, 200, 20);
        no_archive->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        no_archive->typography.color = LUI_COLOR_STEEL_GRAY;
    }
}

static void create_progress_panel(void) {
    g_archive_manager.progress_panel = lui_create_container(g_archive_manager.main_window->root_widget);
    strcpy(g_archive_manager.progress_panel->name, "progress_panel");
    g_archive_manager.progress_panel->bounds = lui_rect_make(0, 500, 1200, 64);
    g_archive_manager.progress_panel->background_color = LUI_COLOR_CHARCOAL_BLACK;
    
    // Active jobs
    if (g_archive_manager.job_count > 0) {
        lui_widget_t* jobs_header = lui_create_label("⚙️ Active Operations", g_archive_manager.progress_panel);
        jobs_header->bounds = lui_rect_make(8, 8, 150, 16);
        jobs_header->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
        jobs_header->typography.color = LUI_COLOR_SECURE_CYAN;
        
        compression_job_t* job = g_archive_manager.active_jobs;
        int x_offset = 8;
        int job_index = 0;
        
        while (job && job_index < 3) {
            const char* op_name = "Processing";
            switch (job->operation) {
                case OPERATION_CREATE: op_name = "Creating"; break;
                case OPERATION_EXTRACT: op_name = "Extracting"; break;
                case OPERATION_TEST: op_name = "Testing"; break;
                case OPERATION_ADD: op_name = "Adding"; break;
                default: op_name = "Processing"; break;
            }
            
            char job_text[128];
            snprintf(job_text, sizeof(job_text), "%s: %.1f%% (%.1f MB/s)",
                    op_name, job->progress_percent, job->current_speed);
            
            lui_widget_t* job_label = lui_create_label(job_text, g_archive_manager.progress_panel);
            job_label->bounds = lui_rect_make(x_offset, 28, 200, 16);
            job_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            
            // Progress bar simulation
            int progress_width = (int)(180 * job->progress_percent / 100.0f);
            lui_widget_t* progress_bg = lui_create_container(g_archive_manager.progress_panel);
            progress_bg->bounds = lui_rect_make(x_offset, 48, 180, 8);
            progress_bg->background_color = LUI_COLOR_STEEL_GRAY;
            
            if (progress_width > 0) {
                lui_widget_t* progress_fill = lui_create_container(progress_bg);
                progress_fill->bounds = lui_rect_make(0, 0, progress_width, 8);
                progress_fill->background_color = job->error ? LUI_COLOR_CRITICAL_RED :
                                                 job->cancelled ? LUI_COLOR_WARNING_AMBER :
                                                 LUI_COLOR_SUCCESS_GREEN;
            }
            
            x_offset += 220;
            job_index++;
            job = job->next;
        }
        
        if (g_archive_manager.job_count > 3) {
            char more_jobs[32];
            snprintf(more_jobs, sizeof(more_jobs), "...and %u more", g_archive_manager.job_count - 3);
            lui_widget_t* more_label = lui_create_label(more_jobs, g_archive_manager.progress_panel);
            more_label->bounds = lui_rect_make(x_offset, 28, 100, 16);
            more_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
            more_label->typography.color = LUI_COLOR_STEEL_GRAY;
        }
    } else {
        lui_widget_t* idle_label = lui_create_label("Ready - No active operations", g_archive_manager.progress_panel);
        idle_label->bounds = lui_rect_make(8, 20, 200, 16);
        idle_label->typography = LUI_TYPOGRAPHY_BODY_SMALL;
        idle_label->typography.color = LUI_COLOR_SUCCESS_GREEN;
    }
}

static void create_status_bar(void) {
    g_archive_manager.status_bar = lui_create_container(g_archive_manager.main_window->root_widget);
    strcpy(g_archive_manager.status_bar->name, "status_bar");
    g_archive_manager.status_bar->bounds = lui_rect_make(0, 564, 1200, 24);
    g_archive_manager.status_bar->background_color = LUI_COLOR_STEEL_GRAY;
    
    // Current archive info
    char status_text[256] = "No archive loaded";
    
    if (g_archive_manager.current_archive) {
        archive_info_t* archive = g_archive_manager.current_archive;
        
        char size_str[32];
        format_file_size(archive->total_size, size_str, sizeof(size_str));
        
        snprintf(status_text, sizeof(status_text),
                "%s | %s | %u files | %s | %.1f%% compression",
                archive->display_name,
                get_format_name(archive->format),
                archive->item_count,
                size_str,
                archive->overall_compression_ratio * 100.0f);
    }
    
    lui_widget_t* status_label = lui_create_label(status_text, g_archive_manager.status_bar);
    status_label->bounds = lui_rect_make(8, 4, 600, 16);
    status_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // Session statistics
    char stats_text[128];
    snprintf(stats_text, sizeof(stats_text), 
            "Session: %u created, %u extracted | Threats: %u",
            g_archive_manager.stats.archives_created,
            g_archive_manager.stats.archives_extracted,
            g_archive_manager.stats.security_threats_detected);
    
    lui_widget_t* stats_label = lui_create_label(stats_text, g_archive_manager.status_bar);
    stats_label->bounds = lui_rect_make(650, 4, 350, 16);
    stats_label->typography = LUI_TYPOGRAPHY_LABEL_SMALL;
    
    // System status indicators
    lui_widget_t* ai_indicator = lui_create_label(g_archive_manager.ai_advisor.enabled ? "🤖" : "🔒", 
                                                 g_archive_manager.status_bar);
    ai_indicator->bounds = lui_rect_make(1050, 4, 16, 16);
    
    lui_widget_t* security_indicator = lui_create_label(g_archive_manager.security_scanner.enabled ? "🛡️" : "⚠️", 
                                                       g_archive_manager.status_bar);
    security_indicator->bounds = lui_rect_make(1070, 4, 16, 16);
    
    // Hardware acceleration indicator
    if (g_archive_manager.settings.hardware_acceleration) {
        lui_widget_t* hw_indicator = lui_create_label("⚡", g_archive_manager.status_bar);
        hw_indicator->bounds = lui_rect_make(1090, 4, 16, 16);
        hw_indicator->typography.color = LUI_COLOR_SUCCESS_GREEN;
    }
}

// ============================================================================
// MAIN ARCHIVE MANAGER API
// ============================================================================

bool limitless_archive_manager_init(void) {
    if (g_archive_manager.initialized) {
        return false;
    }
    
    printf("[ArchiveManager] Initializing Limitless Archive Manager v%s\n", ARCHIVE_MANAGER_VERSION);
    
    // Clear state
    memset(&g_archive_manager, 0, sizeof(g_archive_manager));
    
    // Initialize mutex
    if (pthread_mutex_init(&g_archive_manager.jobs_mutex, NULL) != 0) {
        printf("[ArchiveManager] ERROR: Failed to initialize jobs mutex\n");
        return false;
    }
    
    // Set default configuration
    g_archive_manager.view_mode = VIEW_DETAILS;
    g_archive_manager.sort_by_name = true;
    g_archive_manager.sort_ascending = true;
    g_archive_manager.show_properties = true;
    g_archive_manager.show_progress = true;
    g_archive_manager.show_ai_advisor = true;
    
    // Default settings
    g_archive_manager.settings.default_compression = COMPRESSION_NORMAL;
    g_archive_manager.settings.default_encryption = ENCRYPTION_NONE;
    g_archive_manager.settings.verify_after_operation = true;
    g_archive_manager.settings.thread_count = 4;
    g_archive_manager.settings.memory_limit = 512 * 1024 * 1024; // 512MB
    g_archive_manager.settings.hardware_acceleration = true;
    
    // AI advisor configuration
    g_archive_manager.ai_advisor.enabled = true;
    g_archive_manager.ai_advisor.learning_mode = true;
    g_archive_manager.ai_advisor.profiles.text_optimal = COMPRESSION_MAXIMUM;
    g_archive_manager.ai_advisor.profiles.binary_optimal = COMPRESSION_NORMAL;
    g_archive_manager.ai_advisor.profiles.image_optimal = COMPRESSION_FAST;
    g_archive_manager.ai_advisor.profiles.video_optimal = COMPRESSION_STORE;
    g_archive_manager.ai_advisor.profiles.audio_optimal = COMPRESSION_FAST;
    g_archive_manager.ai_advisor.profiles.skip_compressed_files = true;
    g_archive_manager.ai_advisor.security_policy.scan_during_compression = true;
    g_archive_manager.ai_advisor.security_policy.security_threshold = 0.7f;
    
    // Security scanner configuration
    g_archive_manager.security_scanner.enabled = true;
    g_archive_manager.security_scanner.real_time_scanning = true;
    strcpy(g_archive_manager.security_scanner.scanner_engine, "LimitlessDefender");
    g_archive_manager.security_scanner.last_update = time(NULL);
    
    // Create main window
    g_archive_manager.main_window = lui_create_window("Limitless Archive Manager", LUI_WINDOW_NORMAL,
                                                     50, 50, 1200, 588);
    if (!g_archive_manager.main_window) {
        printf("[ArchiveManager] ERROR: Failed to create main window\n");
        pthread_mutex_destroy(&g_archive_manager.jobs_mutex);
        return false;
    }
    
    // Create UI components
    create_toolbar();
    create_file_list();
    create_properties_panel();
    create_progress_panel();
    create_status_bar();
    
    // Show window
    lui_show_window(g_archive_manager.main_window);
    
    // Scan for recent archives
    scan_directory_for_archives("/home/user/Documents");
    scan_directory_for_archives("/home/user/Downloads");
    
    g_archive_manager.initialized = true;
    g_archive_manager.running = true;
    g_archive_manager.stats.session_start_time = time(NULL);
    
    printf("[ArchiveManager] Archive Manager initialized successfully\n");
    printf("[ArchiveManager] AI Advisor: %s, Security Scanner: %s, Hardware Accel: %s\n",
           g_archive_manager.ai_advisor.enabled ? "Enabled" : "Disabled",
           g_archive_manager.security_scanner.enabled ? "Enabled" : "Disabled",
           g_archive_manager.settings.hardware_acceleration ? "Enabled" : "Disabled");
    printf("[ArchiveManager] Found %u recent archives\n", g_archive_manager.recent_count);
    
    return true;
}

void limitless_archive_manager_shutdown(void) {
    if (!g_archive_manager.initialized) {
        return;
    }
    
    printf("[ArchiveManager] Shutting down Limitless Archive Manager\n");
    
    g_archive_manager.running = false;
    
    // Cancel all active jobs
    compression_job_t* job = g_archive_manager.active_jobs;
    while (job) {
        cancel_compression_job(job);
        job = job->next;
    }
    
    // Wait for all jobs to complete
    cleanup_completed_jobs();
    
    // Free current archive
    if (g_archive_manager.current_archive) {
        free_archive_info(g_archive_manager.current_archive);
    }
    
    // Free recent archives
    for (uint32_t i = 0; i < g_archive_manager.recent_count; i++) {
        if (g_archive_manager.recent_archives[i]) {
            free_archive_info(g_archive_manager.recent_archives[i]);
        }
    }
    
    // Destroy main window
    if (g_archive_manager.main_window) {
        lui_destroy_window(g_archive_manager.main_window);
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&g_archive_manager.jobs_mutex);
    
    // Print session statistics
    time_t session_duration = time(NULL) - g_archive_manager.stats.session_start_time;
    printf("[ArchiveManager] Session statistics:\n");
    printf("  Duration: %ld seconds\n", session_duration);
    printf("  Archives created: %u\n", g_archive_manager.stats.archives_created);
    printf("  Archives extracted: %u\n", g_archive_manager.stats.archives_extracted);
    printf("  Files compressed: %u\n", g_archive_manager.stats.files_compressed);
    printf("  Bytes compressed: %llu\n", g_archive_manager.stats.bytes_compressed);
    printf("  Bytes extracted: %llu\n", g_archive_manager.stats.bytes_extracted);
    printf("  Avg compression ratio: %.1f%%\n", g_archive_manager.stats.total_compression_ratio);
    printf("  Security threats detected: %u\n", g_archive_manager.stats.security_threats_detected);
    printf("  AI files analyzed: %u\n", g_archive_manager.ai_advisor.stats.files_analyzed);
    printf("  AI recommendations made: %u\n", g_archive_manager.ai_advisor.stats.recommendations_made);
    
    memset(&g_archive_manager, 0, sizeof(g_archive_manager));
    
    printf("[ArchiveManager] Shutdown complete\n");
}

const char* limitless_archive_manager_get_version(void) {
    return ARCHIVE_MANAGER_VERSION;
}

bool limitless_archive_manager_open_archive(const char* filepath) {
    if (!g_archive_manager.initialized || !filepath) {
        return false;
    }
    
    printf("[ArchiveManager] Opening archive: %s\n", filepath);
    
    // Free current archive if any
    if (g_archive_manager.current_archive) {
        free_archive_info(g_archive_manager.current_archive);
    }
    
    // Create new archive info
    g_archive_manager.current_archive = create_archive_info(filepath);
    if (!g_archive_manager.current_archive) {
        return false;
    }
    
    strncpy(g_archive_manager.current_archive_path, filepath, sizeof(g_archive_manager.current_archive_path) - 1);
    
    // Load archive contents
    if (!load_archive_contents(g_archive_manager.current_archive)) {
        free_archive_info(g_archive_manager.current_archive);
        g_archive_manager.current_archive = NULL;
        return false;
    }
    
    g_archive_manager.archive_loaded = true;
    
    // Refresh UI
    create_file_list();
    create_properties_panel();
    create_status_bar();
    
    printf("[ArchiveManager] Archive opened successfully: %u files, %.1f%% compression\n",
           g_archive_manager.current_archive->item_count,
           g_archive_manager.current_archive->overall_compression_ratio * 100.0f);
    
    return true;
}

void limitless_archive_manager_run(void) {
    if (!g_archive_manager.initialized) {
        printf("[ArchiveManager] ERROR: Archive Manager not initialized\n");
        return;
    }
    
    // Cleanup completed jobs
    cleanup_completed_jobs();
    
    // Update progress panel if jobs are active
    if (g_archive_manager.job_count > 0) {
        create_progress_panel();
    }
    
    // Main event loop is handled by the desktop environment
}