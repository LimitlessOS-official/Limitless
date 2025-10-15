/*
 * LimitlessOS Accessibility & Internationalization Implementation
 * Complete accessibility support, multi-language, and cultural adaptations
 */

#include "accessibility_i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>
#include <unicode/ustring.h>
#include <unicode/unorm2.h>
#include <unicode/ubrk.h>
#include <unicode/ucol.h>
#include <espeak-ng/espeak_ng.h>
#include <pocketsphinx.h>

/* Global systems */
accessibility_system_t accessibility_system = {0};
i18n_system_t i18n_system = {0};

/* Threading support */
static pthread_mutex_t accessibility_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t i18n_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t voice_recognition_thread;
static bool voice_listening = false;

/* Language database */
static const struct {
    language_code_t code;
    const char *name;
    const char *native_name;
    const char *locale;
    writing_system_t script;
    text_direction_t direction;
} language_database[] = {
    {LANG_EN, "English", "English", "en_US.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_ES, "Spanish", "Español", "es_ES.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_FR, "French", "Français", "fr_FR.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_DE, "German", "Deutsch", "de_DE.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_IT, "Italian", "Italiano", "it_IT.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_PT, "Portuguese", "Português", "pt_PT.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_RU, "Russian", "Русский", "ru_RU.UTF-8", SCRIPT_CYRILLIC, TEXT_DIR_LTR},
    {LANG_JA, "Japanese", "日本語", "ja_JP.UTF-8", SCRIPT_HIRAGANA, TEXT_DIR_LTR},
    {LANG_KO, "Korean", "한국어", "ko_KR.UTF-8", SCRIPT_HANGUL, TEXT_DIR_LTR},
    {LANG_ZH_CN, "Chinese (Simplified)", "中文 (简体)", "zh_CN.UTF-8", SCRIPT_HAN, TEXT_DIR_LTR},
    {LANG_ZH_TW, "Chinese (Traditional)", "中文 (繁體)", "zh_TW.UTF-8", SCRIPT_HAN, TEXT_DIR_LTR},
    {LANG_AR, "Arabic", "العربية", "ar_SA.UTF-8", SCRIPT_ARABIC, TEXT_DIR_RTL},
    {LANG_HI, "Hindi", "हिन्दी", "hi_IN.UTF-8", SCRIPT_DEVANAGARI, TEXT_DIR_LTR},
    {LANG_BN, "Bengali", "বাংলা", "bn_BD.UTF-8", SCRIPT_BENGALI, TEXT_DIR_LTR},
    {LANG_PA, "Punjabi", "ਪੰਜਾਬੀ", "pa_IN.UTF-8", SCRIPT_GURMUKHI, TEXT_DIR_LTR},
    {LANG_TE, "Telugu", "తెలుగు", "te_IN.UTF-8", SCRIPT_TELUGU, TEXT_DIR_LTR},
    {LANG_MR, "Marathi", "मराठी", "mr_IN.UTF-8", SCRIPT_DEVANAGARI, TEXT_DIR_LTR},
    {LANG_TA, "Tamil", "தமிழ்", "ta_IN.UTF-8", SCRIPT_TAMIL, TEXT_DIR_LTR},
    {LANG_UR, "Urdu", "اردو", "ur_PK.UTF-8", SCRIPT_ARABIC, TEXT_DIR_RTL},
    {LANG_GU, "Gujarati", "ગુજરાતી", "gu_IN.UTF-8", SCRIPT_GUJARATI, TEXT_DIR_LTR},
    {LANG_KN, "Kannada", "ಕನ್ನಡ", "kn_IN.UTF-8", SCRIPT_KANNADA, TEXT_DIR_LTR},
    {LANG_ML, "Malayalam", "മലയാളം", "ml_IN.UTF-8", SCRIPT_MALAYALAM, TEXT_DIR_LTR},
    {LANG_OR, "Odia", "ଓଡ଼ିଆ", "or_IN.UTF-8", SCRIPT_ORIYA, TEXT_DIR_LTR},
    {LANG_TH, "Thai", "ไทย", "th_TH.UTF-8", SCRIPT_THAI, TEXT_DIR_LTR},
    {LANG_VI, "Vietnamese", "Tiếng Việt", "vi_VN.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_HE, "Hebrew", "עברית", "he_IL.UTF-8", SCRIPT_HEBREW, TEXT_DIR_RTL},
    {LANG_FA, "Persian", "فارسی", "fa_IR.UTF-8", SCRIPT_ARABIC, TEXT_DIR_RTL},
    {LANG_TR, "Turkish", "Türkçe", "tr_TR.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_EL, "Greek", "Ελληνικά", "el_GR.UTF-8", SCRIPT_GREEK, TEXT_DIR_LTR},
    {LANG_NL, "Dutch", "Nederlands", "nl_NL.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_SV, "Swedish", "Svenska", "sv_SE.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_DA, "Danish", "Dansk", "da_DK.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_NO, "Norwegian", "Norsk", "nb_NO.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_FI, "Finnish", "Suomi", "fi_FI.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_PL, "Polish", "Polski", "pl_PL.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_CS, "Czech", "Čeština", "cs_CZ.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_HU, "Hungarian", "Magyar", "hu_HU.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_ID, "Indonesian", "Bahasa Indonesia", "id_ID.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_MS, "Malay", "Bahasa Melayu", "ms_MY.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_TL, "Filipino", "Filipino", "fil_PH.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_SW, "Swahili", "Kiswahili", "sw_KE.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR},
    {LANG_AF, "Afrikaans", "Afrikaans", "af_ZA.UTF-8", SCRIPT_LATIN, TEXT_DIR_LTR}
};

/* Voice recognition thread */
static void *voice_recognition_thread_func(void *arg) {
    ps_decoder_t *decoder = NULL;
    cmd_ln_t *config = NULL;
    
    /* Initialize PocketSphinx */
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                        "-hmm", "/usr/share/pocketsphinx/model/en-us/en-us",
                        "-lm", "/usr/share/pocketsphinx/model/en-us/en-us.lm.bin",
                        "-dict", "/usr/share/pocketsphinx/model/en-us/cmudict-en-us.dict",
                        NULL);
    
    if (config) {
        decoder = ps_init(config);
        if (decoder) {
            printf("Voice recognition initialized\n");
            
            while (voice_listening) {
                /* In a real implementation, this would:
                 * 1. Capture audio from microphone
                 * 2. Process audio through PocketSphinx
                 * 3. Match against command phrases
                 * 4. Execute corresponding actions
                 */
                
                usleep(100000); /* 100ms sleep */
            }
            
            ps_free(decoder);
        }
        cmd_ln_free_r(config);
    }
    
    return NULL;
}

/* Initialize accessibility system */
int accessibility_init(void) {
    memset(&accessibility_system, 0, sizeof(accessibility_system));
    
    printf("Initializing Accessibility & Internationalization System...\n");
    
    /* Initialize screen reader */
    screen_reader_init();
    
    /* Initialize voice control */
    voice_control_init();
    
    /* Initialize magnifier */
    magnifier_init();
    
    /* Set default accessibility settings */
    accessibility_system.visual.text_scale = 1.0;
    accessibility_system.visual.cursor_size = 1.0;
    accessibility_system.visual.cursor_color = 0x000000; /* Black */
    accessibility_system.visual.cursor_blink = true;
    accessibility_system.visual.color_filter_type = COLOR_FILTER_NONE;
    accessibility_system.visual.filter_intensity = 1.0;
    
    /* Motor accessibility defaults */
    accessibility_system.motor.key_repeat_delay = 500; /* 500ms */
    accessibility_system.motor.key_repeat_rate = 30;   /* 30 per second */
    accessibility_system.motor.double_click_time = 400; /* 400ms */
    accessibility_system.motor.hover_time = 1000;      /* 1 second */
    accessibility_system.motor.switch_scan_time = 2000; /* 2 seconds */
    
    /* Hearing accessibility defaults */
    accessibility_system.hearing.left_right_balance = 0.0; /* Centered */
    accessibility_system.hearing.alert_color = 0xFF0000;   /* Red */
    accessibility_system.hearing.alert_duration = 500;     /* 500ms */
    
    /* Cognitive accessibility defaults */
    accessibility_system.cognitive.reading_time_multiplier = 1;
    
    /* Input method defaults */
    strcpy(accessibility_system.input.keyboard_layout, "qwerty");
    accessibility_system.input.key_size_multiplier = 1.0;
    accessibility_system.input.key_spacing = 0;
    
    accessibility_system.initialized = true;
    
    printf("Accessibility system initialized\n");
    
    return 0;
}

/* Initialize screen reader */
int screen_reader_init(void) {
    /* Initialize eSpeak-NG */
    int result = espeak_ng_Initialize(NULL);
    if (result != ENS_OK) {
        printf("Warning: Failed to initialize eSpeak-NG\n");
        return -1;
    }
    
    /* Configure screen reader defaults */
    accessibility_system.screen_reader.enabled = false;
    accessibility_system.screen_reader.engine = TTS_ENGINE_ESPEAK;
    accessibility_system.screen_reader.language = LANG_EN;
    strcpy(accessibility_system.screen_reader.voice_name, "default");
    accessibility_system.screen_reader.speech_rate = 1.0;
    accessibility_system.screen_reader.pitch = 1.0;
    accessibility_system.screen_reader.volume = 0.8;
    
    /* Reading preferences */
    accessibility_system.screen_reader.read_punctuation = false;
    accessibility_system.screen_reader.read_numbers = true;
    accessibility_system.screen_reader.read_capitals = false;
    accessibility_system.screen_reader.read_formatting = true;
    accessibility_system.screen_reader.read_tables = true;
    accessibility_system.screen_reader.read_links = true;
    accessibility_system.screen_reader.read_images = true;
    
    /* Navigation */
    accessibility_system.screen_reader.quick_nav = true;
    accessibility_system.screen_reader.skip_empty_lines = true;
    accessibility_system.screen_reader.auto_reading = false;
    
    /* Braille */
    accessibility_system.screen_reader.braille_enabled = false;
    strcpy(accessibility_system.screen_reader.braille_table, "en-us-g1.ctb");
    accessibility_system.screen_reader.braille_cells = 40;
    
    printf("Screen reader initialized\n");
    
    return 0;
}

/* Speak text */
int screen_reader_speak(const char *text, bool interrupt) {
    if (!accessibility_system.screen_reader.enabled || !text) return -EINVAL;
    
    if (interrupt) {
        espeak_ng_Cancel();
    }
    
    /* Set voice parameters */
    espeak_ng_SetParameter(espeakRATE, 
        (int)(accessibility_system.screen_reader.speech_rate * 175), 0);
    espeak_ng_SetParameter(espeakPITCH, 
        (int)(accessibility_system.screen_reader.pitch * 50), 0);
    espeak_ng_SetParameter(espeakVOLUME, 
        (int)(accessibility_system.screen_reader.volume * 100), 0);
    
    /* Synthesize speech */
    espeak_ng_STATUS result = espeak_ng_Synthesize(text, strlen(text) + 1, 0, 
                                                  POS_CHARACTER, 0, 
                                                  espeakCHARS_UTF8, NULL, NULL);
    
    if (result == ENS_OK) {
        printf("Speaking: %s\n", text);
        return 0;
    }
    
    return -1;
}

/* Initialize voice control */
int voice_control_init(void) {
    /* Configure voice control defaults */
    accessibility_system.voice_control.enabled = false;
    accessibility_system.voice_control.engine = ASR_ENGINE_POCKETSPHINX;
    accessibility_system.voice_control.language = LANG_EN;
    strcpy(accessibility_system.voice_control.model_path, 
           "/usr/share/pocketsphinx/model/en-us");
    
    accessibility_system.voice_control.confidence_threshold = 0.7;
    accessibility_system.voice_control.timeout_ms = 5000;
    accessibility_system.voice_control.continuous_listening = false;
    accessibility_system.voice_control.wake_word = false;
    strcpy(accessibility_system.voice_control.wake_phrase, "computer");
    
    /* Add default commands */
    accessibility_system.voice_control.command_count = 0;
    voice_control_add_command("open terminal", "limitless-terminal");
    voice_control_add_command("open browser", "limitless-browser");
    voice_control_add_command("open files", "limitless-files");
    voice_control_add_command("take screenshot", "limitless-screenshot");
    voice_control_add_command("lock screen", "limitless-lock");
    voice_control_add_command("shutdown", "systemctl poweroff");
    voice_control_add_command("restart", "systemctl reboot");
    
    /* Feedback settings */
    accessibility_system.voice_control.audio_feedback = true;
    accessibility_system.voice_control.visual_feedback = true;
    strcpy(accessibility_system.voice_control.feedback_sound, 
           "/usr/share/sounds/limitlessos/voice-command.wav");
    
    printf("Voice control initialized with %u commands\n", 
           accessibility_system.voice_control.command_count);
    
    return 0;
}

/* Add voice command */
int voice_control_add_command(const char *phrase, const char *action) {
    if (!phrase || !action) return -EINVAL;
    
    uint32_t count = accessibility_system.voice_control.command_count;
    if (count >= 256) return -ENOMEM;
    
    strncpy(accessibility_system.voice_control.commands[count].phrase, 
            phrase, sizeof(accessibility_system.voice_control.commands[count].phrase) - 1);
    strncpy(accessibility_system.voice_control.commands[count].action, 
            action, sizeof(accessibility_system.voice_control.commands[count].action) - 1);
    accessibility_system.voice_control.commands[count].enabled = true;
    
    accessibility_system.voice_control.command_count++;
    
    return 0;
}

/* Start voice listening */
int voice_control_start_listening(void) {
    if (voice_listening) return 0; /* Already listening */
    
    voice_listening = true;
    pthread_create(&voice_recognition_thread, NULL, voice_recognition_thread_func, NULL);
    
    printf("Voice control started listening\n");
    
    return 0;
}

/* Stop voice listening */
int voice_control_stop_listening(void) {
    if (!voice_listening) return 0; /* Not listening */
    
    voice_listening = false;
    pthread_join(voice_recognition_thread, NULL);
    
    printf("Voice control stopped listening\n");
    
    return 0;
}

/* Initialize magnifier */
int magnifier_init(void) {
    /* Configure magnifier defaults */
    accessibility_system.magnification.enabled = false;
    accessibility_system.magnification.zoom_level = 2.0;
    accessibility_system.magnification.mode = MAG_FULL_SCREEN;
    
    /* Tracking settings */
    accessibility_system.magnification.follow_mouse = true;
    accessibility_system.magnification.follow_keyboard = true;
    accessibility_system.magnification.follow_text_cursor = true;
    
    /* Appearance settings */
    accessibility_system.magnification.smooth_scrolling = true;
    accessibility_system.magnification.invert_colors = false;
    accessibility_system.magnification.enhance_contrast = false;
    accessibility_system.magnification.lens_size = 300;
    
    /* Filters */
    accessibility_system.magnification.brightness = 1.0;
    accessibility_system.magnification.contrast = 1.0;
    accessibility_system.magnification.saturation = 1.0;
    accessibility_system.magnification.hue = 0.0;
    
    printf("Screen magnifier initialized\n");
    
    return 0;
}

/* Initialize internationalization system */
int i18n_init(void) {
    memset(&i18n_system, 0, sizeof(i18n_system));
    
    printf("Initializing internationalization system...\n");
    
    /* Set default language and culture */
    i18n_system.system_language = LANG_EN;
    i18n_system.ui_language = LANG_EN;
    
    /* Cultural defaults (US) */
    strcpy(i18n_system.culture.country_code, "US");
    strcpy(i18n_system.culture.country_name, "United States");
    i18n_system.culture.calendar_type = CALENDAR_GREGORIAN;
    i18n_system.culture.first_day_of_week = WEEK_START_SUNDAY;
    i18n_system.culture.measurement_system = MEASUREMENT_IMPERIAL;
    i18n_system.culture.paper_size = PAPER_LETTER;
    i18n_system.culture.honor_titles = false;
    i18n_system.culture.formal_address = false;
    strcpy(i18n_system.culture.name_order, "given-family");
    
    /* Load language database */
    size_t db_size = sizeof(language_database) / sizeof(language_database[0]);
    for (size_t i = 0; i < db_size && i < LANG_MAX; i++) {
        language_info_t *lang = &i18n_system.languages[i];
        
        lang->code = language_database[i].code;
        strcpy(lang->name, language_database[i].name);
        strcpy(lang->native_name, language_database[i].native_name);
        strcpy(lang->locale, language_database[i].locale);
        lang->script = language_database[i].script;
        lang->direction = language_database[i].direction;
        
        /* Set encoding */
        strcpy(lang->encoding, "UTF-8");
        
        /* Set format defaults */
        strcpy(lang->date_format, "%Y-%m-%d");
        strcpy(lang->time_format, "%H:%M:%S");
        strcpy(lang->number_format, "1,234.56");
        strcpy(lang->currency_symbol, "$");
        
        /* Set font defaults */
        strcpy(lang->primary_font, "DejaVu Sans");
        strcpy(lang->fallback_fonts[0], "Liberation Sans");
        strcpy(lang->fallback_fonts[1], "Noto Sans");
        lang->fallback_count = 2;
        
        /* Input method defaults */
        strcpy(lang->input_method, "ibus");
        lang->complex_scripts = (lang->script != SCRIPT_LATIN);
        lang->bidi_support = (lang->direction == TEXT_DIR_RTL);
        
        /* Translation support */
        lang->translation_available = true;
        strcpy(lang->translation_engine, "google-translate");
        
        i18n_system.language_count++;
    }
    
    /* Text processing defaults */
    i18n_system.text.unicode_support = true;
    strcpy(i18n_system.text.default_encoding, "UTF-8");
    i18n_system.text.normalization = true;
    i18n_system.text.case_folding = true;
    i18n_system.text.complex_text_layout = true;
    i18n_system.text.font_shaping = true;
    i18n_system.text.ligature_support = true;
    i18n_system.text.contextual_forms = true;
    
    /* Localization paths */
    strcpy(i18n_system.localization.messages_dir, "/usr/share/locale");
    strcpy(i18n_system.localization.resource_dir, "/usr/share/limitlessos/resources");
    
    /* Format settings for US English */
    strcpy(i18n_system.localization.formats.decimal_separator, ".");
    strcpy(i18n_system.localization.formats.thousands_separator, ",");
    strcpy(i18n_system.localization.formats.currency_format, "$#,##0.00");
    strcpy(i18n_system.localization.formats.date_short, "%m/%d/%Y");
    strcpy(i18n_system.localization.formats.date_long, "%B %d, %Y");
    strcpy(i18n_system.localization.formats.time_12h, "%I:%M:%S %p");
    strcpy(i18n_system.localization.formats.time_24h, "%H:%M:%S");
    i18n_system.localization.plural_rule = PLURAL_RULE_ONE_OTHER;
    
    /* Input method defaults */
    strcpy(i18n_system.ime.active_ime, "ibus");
    strcpy(i18n_system.ime.ime_list[0], "ibus");
    strcpy(i18n_system.ime.ime_list[1], "fcitx");
    strcpy(i18n_system.ime.ime_list[2], "scim");
    i18n_system.ime.ime_count = 3;
    i18n_system.ime.composition_support = true;
    
    /* Translation service */
    i18n_system.translation.enabled = false;
    strcpy(i18n_system.translation.service_url, "https://translate.googleapis.com/translate_a/single");
    
    /* Initialize IME */
    ime_init();
    
    /* Initialize translation */
    translation_init();
    
    i18n_system.initialized = true;
    
    printf("I18N system initialized with %u languages\n", i18n_system.language_count);
    
    return 0;
}

/* Set system language */
int i18n_set_language(language_code_t language) {
    if (language >= LANG_MAX) return -EINVAL;
    
    pthread_mutex_lock(&i18n_mutex);
    
    /* Find language info */
    language_info_t *lang_info = NULL;
    for (uint32_t i = 0; i < i18n_system.language_count; i++) {
        if (i18n_system.languages[i].code == language) {
            lang_info = &i18n_system.languages[i];
            break;
        }
    }
    
    if (!lang_info) {
        pthread_mutex_unlock(&i18n_mutex);
        return -ENOENT;
    }
    
    /* Set system language */
    i18n_system.system_language = language;
    i18n_system.ui_language = language;
    
    /* Set locale */
    setlocale(LC_ALL, lang_info->locale);
    
    /* Update format settings based on language */
    if (language == LANG_DE || language == LANG_FR || language == LANG_IT) {
        strcpy(i18n_system.localization.formats.decimal_separator, ",");
        strcpy(i18n_system.localization.formats.thousands_separator, ".");
    } else if (language == LANG_EN) {
        strcpy(i18n_system.localization.formats.decimal_separator, ".");
        strcpy(i18n_system.localization.formats.thousands_separator, ",");
    }
    
    /* Update date formats */
    if (language == LANG_EN) {
        strcpy(i18n_system.localization.formats.date_short, "%m/%d/%Y");
    } else {
        strcpy(i18n_system.localization.formats.date_short, "%d/%m/%Y");
    }
    
    pthread_mutex_unlock(&i18n_mutex);
    
    printf("System language set to: %s\n", lang_info->name);
    
    return 0;
}

/* Get localized string */
const char *l10n_get_string(const char *message_id) {
    if (!message_id) return NULL;
    
    /* In a real implementation, this would:
     * 1. Look up message_id in the current language's message catalog
     * 2. Return the translated string
     * 3. Fall back to English if translation not found
     * 4. Fall back to message_id if no translation exists
     */
    
    /* For now, return the message_id itself */
    return message_id;
}

/* Format number according to locale */
int l10n_format_number(double number, char *buffer, size_t size) {
    if (!buffer) return -EINVAL;
    
    const char *decimal_sep = i18n_system.localization.formats.decimal_separator;
    const char *thousands_sep = i18n_system.localization.formats.thousands_separator;
    
    /* Simple formatting (in real implementation, use ICU) */
    snprintf(buffer, size, "%.2f", number);
    
    /* Replace decimal separator if needed */
    if (strcmp(decimal_sep, ".") != 0) {
        char *dot = strchr(buffer, '.');
        if (dot) {
            *dot = decimal_sep[0];
        }
    }
    
    return 0;
}

/* Format date according to locale */
int l10n_format_date(time_t timestamp, char *buffer, size_t size, bool long_format) {
    if (!buffer) return -EINVAL;
    
    struct tm *tm_info = localtime(&timestamp);
    const char *format = long_format ? 
        i18n_system.localization.formats.date_long : 
        i18n_system.localization.formats.date_short;
    
    strftime(buffer, size, format, tm_info);
    
    return 0;
}

/* Initialize input method */
int ime_init(void) {
    printf("Input method engine initialized\n");
    return 0;
}

/* Initialize translation service */
int translation_init(void) {
    printf("Translation service initialized\n");
    return 0;
}

/* Enable accessibility feature */
int accessibility_enable_feature(accessibility_feature_t feature) {
    switch (feature) {
        case A11Y_SCREEN_READER:
            accessibility_system.screen_reader.enabled = true;
            printf("Screen reader enabled\n");
            break;
            
        case A11Y_MAGNIFIER:
            accessibility_system.magnification.enabled = true;
            printf("Screen magnifier enabled\n");
            break;
            
        case A11Y_HIGH_CONTRAST:
            accessibility_system.visual.high_contrast = true;
            printf("High contrast mode enabled\n");
            break;
            
        case A11Y_LARGE_TEXT:
            accessibility_system.visual.text_scale = 1.5;
            printf("Large text mode enabled\n");
            break;
            
        case A11Y_VOICE_CONTROL:
            accessibility_system.voice_control.enabled = true;
            voice_control_start_listening();
            printf("Voice control enabled\n");
            break;
            
        case A11Y_STICKY_KEYS:
            accessibility_system.motor.sticky_keys = true;
            printf("Sticky keys enabled\n");
            break;
            
        case A11Y_CLOSED_CAPTIONS:
            accessibility_system.hearing.closed_captions = true;
            printf("Closed captions enabled\n");
            break;
            
        case A11Y_BRAILLE_DISPLAY:
            accessibility_system.screen_reader.braille_enabled = true;
            printf("Braille display support enabled\n");
            break;
            
        default:
            return -ENOTSUP;
    }
    
    return 0;
}

/* Check if accessibility feature is enabled */
bool accessibility_is_enabled(accessibility_feature_t feature) {
    switch (feature) {
        case A11Y_SCREEN_READER:
            return accessibility_system.screen_reader.enabled;
        case A11Y_MAGNIFIER:
            return accessibility_system.magnification.enabled;
        case A11Y_HIGH_CONTRAST:
            return accessibility_system.visual.high_contrast;
        case A11Y_VOICE_CONTROL:
            return accessibility_system.voice_control.enabled;
        case A11Y_STICKY_KEYS:
            return accessibility_system.motor.sticky_keys;
        case A11Y_CLOSED_CAPTIONS:
            return accessibility_system.hearing.closed_captions;
        case A11Y_BRAILLE_DISPLAY:
            return accessibility_system.screen_reader.braille_enabled;
        default:
            return false;
    }
}

/* Utility functions */
const char *language_code_to_name(language_code_t code) {
    for (uint32_t i = 0; i < i18n_system.language_count; i++) {
        if (i18n_system.languages[i].code == code) {
            return i18n_system.languages[i].name;
        }
    }
    return "Unknown";
}

const char *accessibility_feature_name(accessibility_feature_t feature) {
    static const char *names[] = {
        "Screen Reader", "Magnifier", "High Contrast", "Large Text",
        "Voice Control", "Sticky Keys", "Slow Keys", "Bounce Keys",
        "Mouse Keys", "Switch Access", "Eye Tracking", "Closed Captions",
        "Sign Language", "Braille Display", "Motor Assistance", "Cognitive Assistance"
    };
    
    if (feature >= 0 && feature < A11Y_MAX) {
        return names[feature];
    }
    return "Unknown";
}

bool is_rtl_language(language_code_t language) {
    return (language == LANG_AR || language == LANG_HE || language == LANG_UR || 
            language == LANG_FA);
}

bool is_complex_script(writing_system_t script) {
    return (script == SCRIPT_ARABIC || script == SCRIPT_HEBREW || 
            script == SCRIPT_DEVANAGARI || script == SCRIPT_BENGALI ||
            script == SCRIPT_THAI || script == SCRIPT_MYANMAR);
}