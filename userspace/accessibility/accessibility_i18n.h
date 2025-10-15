/*
 * LimitlessOS Accessibility & Internationalization Framework
 * Complete accessibility support, multi-language, and cultural adaptations
 */

#ifndef ACCESSIBILITY_I18N_H
#define ACCESSIBILITY_I18N_H

#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>

/* Accessibility feature types */
typedef enum {
    A11Y_SCREEN_READER,         /* Screen reader for visually impaired */
    A11Y_MAGNIFIER,            /* Screen magnification */
    A11Y_HIGH_CONTRAST,        /* High contrast display */
    A11Y_LARGE_TEXT,           /* Large text display */
    A11Y_VOICE_CONTROL,        /* Voice command interface */
    A11Y_STICKY_KEYS,          /* Sticky keys for motor impairments */
    A11Y_SLOW_KEYS,            /* Slow keys filter */
    A11Y_BOUNCE_KEYS,          /* Bounce keys filter */
    A11Y_MOUSE_KEYS,           /* Mouse control via keyboard */
    A11Y_SWITCH_ACCESS,        /* Switch-based navigation */
    A11Y_EYE_TRACKING,         /* Eye tracking control */
    A11Y_CLOSED_CAPTIONS,      /* Closed captioning */
    A11Y_SIGN_LANGUAGE,        /* Sign language interpretation */
    A11Y_BRAILLE_DISPLAY,      /* Braille display support */
    A11Y_MOTOR_ASSISTANCE,     /* Motor impairment assistance */
    A11Y_COGNITIVE_ASSISTANCE, /* Cognitive assistance features */
    A11Y_MAX
} accessibility_feature_t;

/* Text-to-speech engines */
typedef enum {
    TTS_ENGINE_ESPEAK,         /* eSpeak synthesizer */
    TTS_ENGINE_FESTIVAL,       /* Festival speech system */
    TTS_ENGINE_PICO,           /* Pico TTS */
    TTS_ENGINE_MARY,           /* MaryTTS */
    TTS_ENGINE_NEURAL,         /* Neural TTS */
    TTS_ENGINE_CUSTOM,         /* Custom engine */
    TTS_ENGINE_MAX
} tts_engine_t;

/* Speech recognition engines */
typedef enum {
    ASR_ENGINE_POCKETSPHINX,   /* CMU PocketSphinx */
    ASR_ENGINE_JULIUS,         /* Julius speech recognition */
    ASR_ENGINE_KALDI,          /* Kaldi toolkit */
    ASR_ENGINE_WHISPER,        /* OpenAI Whisper */
    ASR_ENGINE_VOSK,           /* Vosk speech recognition */
    ASR_ENGINE_CUSTOM,         /* Custom engine */
    ASR_ENGINE_MAX
} asr_engine_t;

/* Language codes (ISO 639-1) */
typedef enum {
    LANG_EN,    /* English */
    LANG_ES,    /* Spanish */
    LANG_FR,    /* French */
    LANG_DE,    /* German */
    LANG_IT,    /* Italian */
    LANG_PT,    /* Portuguese */
    LANG_RU,    /* Russian */
    LANG_JA,    /* Japanese */
    LANG_KO,    /* Korean */
    LANG_ZH_CN, /* Chinese (Simplified) */
    LANG_ZH_TW, /* Chinese (Traditional) */
    LANG_AR,    /* Arabic */
    LANG_HI,    /* Hindi */
    LANG_BN,    /* Bengali */
    LANG_PA,    /* Punjabi */
    LANG_TE,    /* Telugu */
    LANG_MR,    /* Marathi */
    LANG_TA,    /* Tamil */
    LANG_UR,    /* Urdu */
    LANG_GU,    /* Gujarati */
    LANG_KN,    /* Kannada */
    LANG_ML,    /* Malayalam */
    LANG_OR,    /* Odia */
    LANG_AS,    /* Assamese */
    LANG_NL,    /* Dutch */
    LANG_SV,    /* Swedish */
    LANG_DA,    /* Danish */
    LANG_NO,    /* Norwegian */
    LANG_FI,    /* Finnish */
    LANG_PL,    /* Polish */
    LANG_CS,    /* Czech */
    LANG_SK,    /* Slovak */
    LANG_HU,    /* Hungarian */
    LANG_RO,    /* Romanian */
    LANG_BG,    /* Bulgarian */
    LANG_HR,    /* Croatian */
    LANG_SR,    /* Serbian */
    LANG_SL,    /* Slovenian */
    LANG_LT,    /* Lithuanian */
    LANG_LV,    /* Latvian */
    LANG_ET,    /* Estonian */
    LANG_MT,    /* Maltese */
    LANG_GA,    /* Irish */
    LANG_CY,    /* Welsh */
    LANG_IS,    /* Icelandic */
    LANG_MK,    /* Macedonian */
    LANG_AL,    /* Albanian */
    LANG_TR,    /* Turkish */
    LANG_EL,    /* Greek */
    LANG_HE,    /* Hebrew */
    LANG_FA,    /* Persian */
    LANG_TH,    /* Thai */
    LANG_VI,    /* Vietnamese */
    LANG_ID,    /* Indonesian */
    LANG_MS,    /* Malay */
    LANG_TL,    /* Filipino */
    LANG_SW,    /* Swahili */
    LANG_AM,    /* Amharic */
    LANG_IG,    /* Igbo */
    LANG_YO,    /* Yoruba */
    LANG_HA,    /* Hausa */
    LANG_ZU,    /* Zulu */
    LANG_AF,    /* Afrikaans */
    LANG_MAX
} language_code_t;

/* Writing systems */
typedef enum {
    SCRIPT_LATIN,              /* Latin alphabet */
    SCRIPT_CYRILLIC,           /* Cyrillic alphabet */
    SCRIPT_GREEK,              /* Greek alphabet */
    SCRIPT_ARABIC,             /* Arabic script */
    SCRIPT_HEBREW,             /* Hebrew script */
    SCRIPT_DEVANAGARI,         /* Devanagari (Hindi, Sanskrit) */
    SCRIPT_BENGALI,            /* Bengali script */
    SCRIPT_GURMUKHI,           /* Gurmukhi (Punjabi) */
    SCRIPT_GUJARATI,           /* Gujarati script */
    SCRIPT_ORIYA,              /* Odia script */
    SCRIPT_TAMIL,              /* Tamil script */
    SCRIPT_TELUGU,             /* Telugu script */
    SCRIPT_KANNADA,            /* Kannada script */
    SCRIPT_MALAYALAM,          /* Malayalam script */
    SCRIPT_SINHALA,            /* Sinhala script */
    SCRIPT_THAI,               /* Thai script */
    SCRIPT_LAO,                /* Lao script */
    SCRIPT_TIBETAN,            /* Tibetan script */
    SCRIPT_MYANMAR,            /* Myanmar script */
    SCRIPT_GEORGIAN,           /* Georgian script */
    SCRIPT_HANGUL,             /* Hangul (Korean) */
    SCRIPT_HIRAGANA,           /* Hiragana (Japanese) */
    SCRIPT_KATAKANA,           /* Katakana (Japanese) */
    SCRIPT_HAN,                /* Han characters (Chinese/Japanese/Korean) */
    SCRIPT_ETHIOPIC,           /* Ge'ez/Ethiopic script */
    SCRIPT_CHEROKEE,           /* Cherokee syllabary */
    SCRIPT_CANADIAN_ABORIGINAL, /* Canadian Aboriginal syllabics */
    SCRIPT_OGHAM,              /* Ogham script */
    SCRIPT_RUNIC,              /* Runic alphabet */
    SCRIPT_KHMER,              /* Khmer script */
    SCRIPT_MONGOLIAN,          /* Mongolian script */
    SCRIPT_MAX
} writing_system_t;

/* Text direction */
typedef enum {
    TEXT_DIR_LTR,              /* Left-to-right */
    TEXT_DIR_RTL,              /* Right-to-left */
    TEXT_DIR_TTB,              /* Top-to-bottom */
    TEXT_DIR_BTT               /* Bottom-to-top */
} text_direction_t;

/* Screen reader configuration */
typedef struct {
    bool enabled;
    tts_engine_t engine;        /* TTS engine */
    language_code_t language;   /* Voice language */
    char voice_name[64];        /* Specific voice */
    float speech_rate;          /* Speech rate (0.1-3.0) */
    float pitch;                /* Voice pitch (0.1-3.0) */
    float volume;               /* Voice volume (0.0-1.0) */
    
    /* Reading preferences */
    bool read_punctuation;      /* Read punctuation marks */
    bool read_numbers;          /* Read numbers as digits or words */
    bool read_capitals;         /* Announce capital letters */
    bool read_formatting;       /* Read text formatting */
    bool read_tables;           /* Read table structure */
    bool read_links;            /* Announce hyperlinks */
    bool read_images;           /* Read image alt text */
    
    /* Navigation */
    bool quick_nav;             /* Quick navigation keys */
    bool skip_empty_lines;      /* Skip empty lines */
    bool auto_reading;          /* Automatic reading mode */
    
    /* Braille integration */
    bool braille_enabled;       /* Braille display support */
    char braille_table[64];     /* Braille translation table */
    uint32_t braille_cells;     /* Number of braille cells */
} screen_reader_config_t;

/* Voice control configuration */
typedef struct {
    bool enabled;
    asr_engine_t engine;        /* ASR engine */
    language_code_t language;   /* Recognition language */
    char model_path[256];       /* Language model path */
    
    /* Recognition settings */
    float confidence_threshold; /* Minimum confidence (0.0-1.0) */
    uint32_t timeout_ms;        /* Recognition timeout */
    bool continuous_listening;  /* Continuous listening mode */
    bool wake_word;             /* Wake word activation */
    char wake_phrase[64];       /* Wake word/phrase */
    
    /* Commands */
    struct {
        char phrase[128];        /* Voice command phrase */
        char action[256];        /* Action to execute */
        bool enabled;            /* Command enabled */
    } commands[256];
    uint32_t command_count;
    
    /* Feedback */
    bool audio_feedback;        /* Audio confirmation */
    bool visual_feedback;       /* Visual confirmation */
    char feedback_sound[256];   /* Feedback sound file */
} voice_control_config_t;

/* Magnification configuration */
typedef struct {
    bool enabled;
    float zoom_level;           /* Magnification level (1.0-20.0) */
    enum {
        MAG_FULL_SCREEN,        /* Full screen magnification */
        MAG_LENS,              /* Magnifying lens */
        MAG_DOCKED             /* Docked magnifier */
    } mode;
    
    /* Tracking */
    bool follow_mouse;          /* Follow mouse cursor */
    bool follow_keyboard;       /* Follow keyboard focus */
    bool follow_text_cursor;    /* Follow text insertion point */
    
    /* Appearance */
    bool smooth_scrolling;      /* Smooth scrolling */
    bool invert_colors;        /* Color inversion */
    bool enhance_contrast;     /* Contrast enhancement */
    uint32_t lens_size;        /* Lens size in pixels */
    
    /* Filters */
    float brightness;          /* Brightness adjustment */
    float contrast;            /* Contrast adjustment */
    float saturation;          /* Color saturation */
    float hue;                 /* Hue adjustment */
} magnification_config_t;

/* Language configuration */
typedef struct {
    language_code_t code;       /* Language code */
    char name[64];              /* Language name */
    char native_name[64];       /* Native language name */
    writing_system_t script;    /* Writing system */
    text_direction_t direction; /* Text direction */
    
    /* Localization */
    char locale[32];            /* System locale */
    char encoding[32];          /* Character encoding */
    char date_format[64];       /* Date format */
    char time_format[64];       /* Time format */
    char number_format[64];     /* Number format */
    char currency_symbol[8];    /* Currency symbol */
    
    /* Fonts */
    char primary_font[64];      /* Primary font family */
    char fallback_fonts[8][64]; /* Fallback fonts */
    uint32_t fallback_count;
    
    /* Input methods */
    char input_method[64];      /* Input method engine */
    bool complex_scripts;       /* Complex script support */
    bool bidi_support;          /* Bidirectional text support */
    
    /* Translation */
    bool translation_available; /* Translation support */
    char translation_engine[64]; /* Translation engine */
} language_info_t;

/* Cultural settings */
typedef struct {
    char country_code[4];       /* ISO 3166 country code */
    char country_name[64];      /* Country name */
    
    /* Calendar */
    enum {
        CALENDAR_GREGORIAN,
        CALENDAR_ISLAMIC,
        CALENDAR_HEBREW,
        CALENDAR_BUDDHIST,
        CALENDAR_JAPANESE,
        CALENDAR_THAI,
        CALENDAR_PERSIAN,
        CALENDAR_ETHIOPIAN
    } calendar_type;
    
    /* Week settings */
    enum {
        WEEK_START_SUNDAY = 0,
        WEEK_START_MONDAY = 1,
        WEEK_START_SATURDAY = 6
    } first_day_of_week;
    
    /* Measurement system */
    enum {
        MEASUREMENT_METRIC,
        MEASUREMENT_IMPERIAL,
        MEASUREMENT_MIXED
    } measurement_system;
    
    /* Paper size */
    enum {
        PAPER_A4,
        PAPER_LETTER,
        PAPER_LEGAL,
        PAPER_A3,
        PAPER_TABLOID
    } paper_size;
    
    /* Cultural preferences */
    bool honor_titles;          /* Use honor/courtesy titles */
    bool formal_address;        /* Formal addressing */
    char name_order[32];        /* Name ordering (given-family, family-given) */
} cultural_settings_t;

/* Accessibility system */
typedef struct {
    bool initialized;
    
    /* Feature configurations */
    screen_reader_config_t screen_reader;
    voice_control_config_t voice_control;
    magnification_config_t magnification;
    
    /* Visual accessibility */
    struct {
        bool high_contrast;      /* High contrast mode */
        bool invert_colors;      /* Color inversion */
        bool reduce_motion;      /* Reduce motion/animations */
        bool reduce_transparency; /* Reduce transparency effects */
        float text_scale;        /* Text scaling factor */
        float cursor_size;       /* Cursor size multiplier */
        uint32_t cursor_color;   /* Cursor color */
        bool cursor_blink;       /* Cursor blinking */
        
        /* Color filters */
        bool color_filters;      /* Color filter support */
        enum {
            COLOR_FILTER_NONE,
            COLOR_FILTER_PROTANOPIA,
            COLOR_FILTER_DEUTERANOPIA,
            COLOR_FILTER_TRITANOPIA,
            COLOR_FILTER_GRAYSCALE
        } color_filter_type;
        float filter_intensity;  /* Filter intensity */
    } visual;
    
    /* Motor accessibility */
    struct {
        bool sticky_keys;        /* Sticky keys */
        bool slow_keys;          /* Slow keys */
        bool bounce_keys;        /* Bounce keys */
        bool mouse_keys;         /* Mouse keys */
        bool switch_control;     /* Switch control */
        bool dwell_click;        /* Dwell clicking */
        
        /* Timing settings */
        uint32_t key_repeat_delay; /* Key repeat delay */
        uint32_t key_repeat_rate;  /* Key repeat rate */
        uint32_t double_click_time; /* Double-click time */
        uint32_t hover_time;       /* Hover activation time */
        
        /* Switch settings */
        char switch_device[256]; /* Switch device path */
        uint32_t switch_scan_time; /* Auto-scan timing */
    } motor;
    
    /* Hearing accessibility */
    struct {
        bool visual_alerts;      /* Visual notifications */
        bool flash_screen;       /* Screen flashing alerts */
        bool closed_captions;    /* Closed captions */
        bool sign_language;      /* Sign language */
        bool audio_description;  /* Audio descriptions */
        
        /* Sound settings */
        bool mono_audio;         /* Mono audio output */
        float left_right_balance; /* Audio balance */
        bool reduce_loud_sounds; /* Dynamic range compression */
        
        /* Visual feedback */
        uint32_t alert_color;    /* Visual alert color */
        uint32_t alert_duration; /* Alert duration (ms) */
    } hearing;
    
    /* Cognitive accessibility */
    struct {
        bool guided_access;      /* Guided access mode */
        bool simplified_ui;      /* Simplified interface */
        bool reading_assistance; /* Reading assistance */
        bool focus_indicators;   /* Enhanced focus indicators */
        bool reduced_clutter;    /* Reduce UI clutter */
        
        /* Timing assistance */
        bool extended_timeouts;  /* Extended timeout periods */
        bool pause_on_focus_loss; /* Pause on focus loss */
        uint32_t reading_time_multiplier; /* Reading time multiplier */
    } cognitive;
    
    /* Input methods */
    struct {
        bool on_screen_keyboard; /* Virtual keyboard */
        bool word_prediction;    /* Word prediction */
        bool auto_correction;    /* Auto-correction */
        bool gesture_typing;     /* Gesture/swipe typing */
        
        char keyboard_layout[64]; /* Keyboard layout */
        float key_size_multiplier; /* Key size scaling */
        uint32_t key_spacing;    /* Additional key spacing */
    } input;
} accessibility_system_t;

/* Internationalization system */
typedef struct {
    bool initialized;
    
    /* Current settings */
    language_code_t system_language;    /* System language */
    language_code_t ui_language;        /* User interface language */
    cultural_settings_t culture;        /* Cultural settings */
    
    /* Available languages */
    language_info_t languages[LANG_MAX];
    uint32_t language_count;
    
    /* Text processing */
    struct {
        bool unicode_support;    /* Unicode support enabled */
        char default_encoding[32]; /* Default text encoding */
        bool normalization;      /* Unicode normalization */
        bool case_folding;       /* Case folding support */
        
        /* Complex scripts */
        bool complex_text_layout; /* Complex text layout */
        bool font_shaping;       /* Advanced font shaping */
        bool ligature_support;   /* Ligature rendering */
        bool contextual_forms;   /* Contextual character forms */
    } text;
    
    /* Localization */
    struct {
        char messages_dir[256];   /* Message catalogs directory */
        char resource_dir[256];   /* Localized resources directory */
        
        /* Format settings */
        struct {
            char decimal_separator[4];
            char thousands_separator[4];
            char currency_format[32];
            char date_short[32];
            char date_long[64];
            char time_12h[32];
            char time_24h[32];
        } formats;
        
        /* Plural rules */
        enum {
            PLURAL_RULE_NONE,
            PLURAL_RULE_ONE_OTHER,
            PLURAL_RULE_ONE_TWO_OTHER,
            PLURAL_RULE_COMPLEX
        } plural_rule;
    } localization;
    
    /* Input methods */
    struct {
        char active_ime[64];     /* Active input method */
        char ime_list[16][64];   /* Available input methods */
        uint32_t ime_count;
        
        /* Composition */
        bool composition_support; /* Input composition */
        wchar_t composition_buffer[256]; /* Composition buffer */
        uint32_t composition_length;
        uint32_t cursor_position;
    } ime;
    
    /* Translation */
    struct {
        bool enabled;            /* Translation service enabled */
        char service_url[256];   /* Translation service URL */
        char api_key[128];       /* API key */
        
        /* Cache */
        struct {
            char source_text[1024];
            char target_text[1024];
            language_code_t source_lang;
            language_code_t target_lang;
            time_t timestamp;
        } cache[256];
        uint32_t cache_count;
    } translation;
} i18n_system_t;

/* Function prototypes */

/* Accessibility system */
int accessibility_init(void);
void accessibility_exit(void);
int accessibility_enable_feature(accessibility_feature_t feature);
int accessibility_disable_feature(accessibility_feature_t feature);
bool accessibility_is_enabled(accessibility_feature_t feature);

/* Screen reader */
int screen_reader_init(void);
int screen_reader_speak(const char *text, bool interrupt);
int screen_reader_speak_formatted(const char *text, const char *format_info);
int screen_reader_stop_speech(void);
int screen_reader_set_voice(const char *voice_name);
int screen_reader_set_rate(float rate);
int screen_reader_set_pitch(float pitch);
int screen_reader_set_volume(float volume);

/* Voice control */
int voice_control_init(void);
int voice_control_start_listening(void);
int voice_control_stop_listening(void);
int voice_control_add_command(const char *phrase, const char *action);
int voice_control_remove_command(const char *phrase);
int voice_control_set_wake_word(const char *phrase);
int voice_control_get_recognition_result(char *result, size_t size);

/* Screen magnification */
int magnifier_init(void);
int magnifier_enable(void);
int magnifier_disable(void);
int magnifier_set_zoom(float zoom_level);
int magnifier_set_mode(int mode);
int magnifier_follow_cursor(bool follow);
int magnifier_set_filters(float brightness, float contrast, float saturation);

/* Internationalization */
int i18n_init(void);
void i18n_exit(void);
int i18n_set_language(language_code_t language);
int i18n_set_locale(const char *locale);
int i18n_add_language_pack(const language_info_t *language);

/* Text processing */
int text_get_direction(const wchar_t *text);
int text_normalize_unicode(const wchar_t *input, wchar_t *output, size_t size);
int text_convert_case(const wchar_t *input, wchar_t *output, size_t size, bool to_upper);
int text_shape_complex(const wchar_t *input, uint32_t *glyphs, size_t glyph_count);

/* Localization */
const char *l10n_get_string(const char *message_id);
const char *l10n_get_plural(const char *message_id, int count);
int l10n_format_number(double number, char *buffer, size_t size);
int l10n_format_currency(double amount, char *buffer, size_t size);
int l10n_format_date(time_t timestamp, char *buffer, size_t size, bool long_format);
int l10n_format_time(time_t timestamp, char *buffer, size_t size, bool format_24h);

/* Input methods */
int ime_init(void);
int ime_activate(const char *ime_name);
int ime_deactivate(void);
int ime_process_key(uint32_t keycode, uint32_t modifiers);
int ime_get_composition(wchar_t *buffer, size_t size);
int ime_commit_composition(void);
int ime_cancel_composition(void);

/* Translation */
int translation_init(void);
int translation_translate_text(const char *text, language_code_t source_lang, 
                              language_code_t target_lang, char *result, size_t size);
int translation_detect_language(const char *text, language_code_t *detected_lang);
int translation_get_supported_languages(language_code_t *languages, size_t max_count);

/* Braille support */
int braille_init(void);
int braille_connect_display(const char *device_path);
int braille_disconnect_display(void);
int braille_write_text(const char *text);
int braille_set_cursor_position(uint32_t position);
int braille_get_key_input(uint32_t *keys);

/* Sign language */
int sign_language_init(void);
int sign_language_translate_text(const char *text, const char *output_video);
int sign_language_recognize_gesture(const void *video_data, char *text, size_t size);

/* Utility functions */
const char *language_code_to_name(language_code_t code);
language_code_t language_name_to_code(const char *name);
const char *accessibility_feature_name(accessibility_feature_t feature);
const char *writing_system_name(writing_system_t script);
bool is_rtl_language(language_code_t language);
bool is_complex_script(writing_system_t script);

/* Global systems */
extern accessibility_system_t accessibility_system;
extern i18n_system_t i18n_system;

#endif /* ACCESSIBILITY_I18N_H */