/*
 * Action Card System Implementation
 * Complete system operations framework for desktop environment interactions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include "action_card.h"

/* Global action card system state */
static struct {
    bool initialized;
    uint64_t next_id;
    action_card_settings_t settings;
    consent_rule_t consent_rules[64];
    uint32_t consent_rule_count;
    action_card_t* audit_history[256];
    uint32_t audit_count;
    action_card_t* pending_actions[16];
    uint32_t pending_count;
} g_action_system = {0};

/* Helper functions */
static uint64_t get_timestamp(void);
static void format_action_type(action_type_t type, char* buffer, size_t size);
static void format_privilege(action_privilege_t priv, char* buffer, size_t size);
static int calculate_risk_score(action_card_t* card);
static bool pattern_matches(const char* pattern, const char* command);
static int execute_system_command(const char* command, char* output, size_t output_size);
static void timeout_handler(int sig);

/* Signal handler for timeout */
static void timeout_handler(int sig) {
    /* Timeout occurred - do nothing, just return */
    (void)sig; /* Suppress unused parameter warning */
}

/* Get current timestamp */
static uint64_t get_timestamp(void) {
    return (uint64_t)time(NULL);
}

/* Format action type as string */
static void format_action_type(action_type_t type, char* buffer, size_t size) {
    const char* type_strings[] = {
        "COMMAND", "FILE_WRITE", "FILE_DELETE", "INSTALL", "UNINSTALL",
        "SETTING_CHANGE", "NETWORK", "CODE_EXEC", "PERMISSION", "CUSTOM"
    };
    
    if (type < sizeof(type_strings) / sizeof(type_strings[0])) {
        strncpy(buffer, type_strings[type], size - 1);
        buffer[size - 1] = '\0';
    } else {
        snprintf(buffer, size, "UNKNOWN_%d", type);
    }
}

/* Format privilege level as string */
static void format_privilege(action_privilege_t priv, char* buffer, size_t size) {
    const char* priv_strings[] = {"USER", "ADMIN", "SYSTEM", "SECURITY"};
    
    if (priv < sizeof(priv_strings) / sizeof(priv_strings[0])) {
        strncpy(buffer, priv_strings[priv], size - 1);
        buffer[size - 1] = '\0';
    } else {
        snprintf(buffer, size, "UNKNOWN_%d", priv);
    }
}

/* Calculate risk score based on action characteristics */
static int calculate_risk_score(action_card_t* card) {
    int score = 0;
    
    /* Base score by type */
    switch (card->type) {
        case ACTION_FILE_DELETE: score += 30; break;
        case ACTION_UNINSTALL: score += 25; break;
        case ACTION_SETTING_CHANGE: score += 20; break;
        case ACTION_PERMISSION: score += 35; break;
        case ACTION_CODE_EXEC: score += 40; break;
        case ACTION_NETWORK: score += 15; break;
        default: score += 10; break;
    }
    
    /* Privilege escalation */
    switch (card->privilege) {
        case PRIVILEGE_ADMIN: score += 20; break;
        case PRIVILEGE_SYSTEM: score += 30; break;
        case PRIVILEGE_SECURITY: score += 40; break;
        default: break;
    }
    
    /* Impact factors */
    if (card->impact.affects_security) score += 25;
    if (card->impact.affects_privacy) score += 20;
    if (card->impact.affects_data) score += 15;
    if (!card->impact.reversible) score += 20;
    if (card->impact.persistent) score += 10;
    
    /* Force flag increases risk */
    if (card->force) score += 15;
    
    return (score > 100) ? 100 : score;
}

/* Check if pattern matches command */
static bool pattern_matches(const char* pattern, const char* command) {
    if (!pattern || !command) return false;
    
    /* Simple wildcard matching */
    if (strcmp(pattern, "*") == 0) return true;
    
    /* Check for exact match */
    if (strcmp(pattern, command) == 0) return true;
    
    /* Check for prefix match with * */
    size_t pattern_len = strlen(pattern);
    if (pattern_len > 0 && pattern[pattern_len - 1] == '*') {
        return strncmp(pattern, command, pattern_len - 1) == 0;
    }
    
    return false;
}

/* Execute system command */
static int execute_system_command(const char* command, char* output, size_t output_size) {
    if (!command) return -1;
    
    printf("[ActionCard] Executing: %s\n", command);
    
    /* For demonstration, simulate command execution */
    if (output && output_size > 0) {
        snprintf(output, output_size, "Command '%s' executed successfully.", command);
    }
    
    return 0; /* Success */
}

/* Initialize action card system */
int action_card_init(void) {
    if (g_action_system.initialized) {
        return -1; /* Already initialized */
    }
    
    printf("[ActionCard] Initializing action card system\n");
    
    g_action_system.next_id = 1;
    
    /* Set default settings */
    g_action_system.settings.enabled = true;
    g_action_system.settings.force_bypass_enabled = false;
    g_action_system.settings.show_technical_details = true;
    g_action_system.settings.require_confirmation = true;
    g_action_system.settings.approval_timeout_sec = 60;
    g_action_system.settings.audit_log_enabled = true;
    g_action_system.settings.undo_enabled = true;
    g_action_system.settings.undo_history_size = 100;
    
    g_action_system.consent_rule_count = 0;
    g_action_system.audit_count = 0;
    g_action_system.pending_count = 0;
    
    g_action_system.initialized = true;
    
    printf("[ActionCard] System initialized successfully\n");
    return 0;
}

/* Create new action card */
action_card_t* action_card_create(action_type_t type, const char* title, const char* summary) {
    if (!g_action_system.initialized) {
        action_card_init();
    }
    
    action_card_t* card = calloc(1, sizeof(action_card_t));
    if (!card) {
        return NULL;
    }
    
    card->id = g_action_system.next_id++;
    card->type = type;
    card->privilege = PRIVILEGE_USER;
    card->response = RESPONSE_PENDING;
    card->created_at = get_timestamp();
    
    if (title) {
        strncpy(card->title, title, sizeof(card->title) - 1);
        card->title[sizeof(card->title) - 1] = '\0';
    }
    
    if (summary) {
        strncpy(card->summary, summary, sizeof(card->summary) - 1);
        card->summary[sizeof(card->summary) - 1] = '\0';
    }
    
    /* Default impact assessment */
    card->impact.reversible = true;
    card->impact.persistent = false;
    card->impact.affects_security = false;
    card->impact.affects_privacy = false;
    card->impact.affects_data = false;
    
    printf("[ActionCard] Created action card ID %llu: %s\n", card->id, title ? title : "Untitled");
    
    return card;
}

/* Set action command */
void action_card_set_command(action_card_t* card, const char* command) {
    if (!card || !command) return;
    
    strncpy(card->command, command, sizeof(card->command) - 1);
    card->command[sizeof(card->command) - 1] = '\0';
}

/* Set action details */
void action_card_set_details(action_card_t* card, const char* details) {
    if (!card || !details) return;
    
    strncpy(card->details, details, sizeof(card->details) - 1);
    card->details[sizeof(card->details) - 1] = '\0';
}

/* Set privilege level */
void action_card_set_privilege(action_card_t* card, action_privilege_t privilege) {
    if (!card) return;
    card->privilege = privilege;
}

/* Set force flag */
void action_card_set_force(action_card_t* card, bool force) {
    if (!card) return;
    card->force = force;
}

/* Assess impact of action */
void action_card_assess_impact(action_card_t* card) {
    if (!card) return;
    
    /* Analyze command for impact indicators */
    const char* cmd = card->command;
    
    /* Check for dangerous operations */
    if (strstr(cmd, "rm ") || strstr(cmd, "delete ") || strstr(cmd, "uninstall ")) {
        card->impact.reversible = false;
        card->impact.affects_data = true;
    }
    
    if (strstr(cmd, "chmod ") || strstr(cmd, "chown ") || strstr(cmd, "passwd ")) {
        card->impact.affects_security = true;
    }
    
    if (strstr(cmd, "install ") || strstr(cmd, "config ") || strstr(cmd, "settings ")) {
        card->impact.persistent = true;
    }
    
    if (strstr(cmd, "network ") || strstr(cmd, "connect ") || strstr(cmd, "ssh ")) {
        card->impact.affects_privacy = true;
    }
    
    /* Calculate risk score */
    card->impact.risk_level = calculate_risk_score(card);
}

/* Set reversible flag and undo command */
void action_card_set_reversible(action_card_t* card, bool reversible, const char* undo_cmd) {
    if (!card) return;
    
    card->can_undo = reversible;
    card->impact.reversible = reversible;
    
    if (undo_cmd && reversible) {
        strncpy(card->undo_command, undo_cmd, sizeof(card->undo_command) - 1);
        card->undo_command[sizeof(card->undo_command) - 1] = '\0';
    }
}

/* Add step to action */
void action_card_add_step(action_card_t* card, const char* description) {
    if (!card || !description || card->step_count >= ACTION_MAX_STEPS) return;
    
    action_step_t* step = &card->steps[card->step_count];
    strncpy(step->description, description, sizeof(step->description) - 1);
    step->description[sizeof(step->description) - 1] = '\0';
    step->completed = false;
    step->timestamp = 0;
    
    card->step_count++;
}

/* Complete a step */
void action_card_complete_step(action_card_t* card, uint32_t step_index) {
    if (!card || step_index >= card->step_count) return;
    
    card->steps[step_index].completed = true;
    card->steps[step_index].timestamp = get_timestamp();
}

/* Add required permission */
void action_card_add_permission(action_card_t* card, const char* permission) {
    if (!card || !permission || card->permission_count >= ACTION_MAX_PERMISSIONS) return;
    
    strncpy(card->permissions[card->permission_count], permission, 63);
    card->permissions[card->permission_count][63] = '\0';
    card->permission_count++;
}

/* Present action card to user */
int action_card_present(action_card_t* card) {
    if (!card) return -1;
    
    card->presented_at = get_timestamp();
    
    /* Add to pending actions */
    if (g_action_system.pending_count < sizeof(g_action_system.pending_actions) / sizeof(g_action_system.pending_actions[0])) {
        g_action_system.pending_actions[g_action_system.pending_count++] = card;
    }
    
    printf("\n" "═══════════════════════════════════════════════════════════════\n");
    printf("                      ACTION CARD #%llu\n", card->id);
    printf("═══════════════════════════════════════════════════════════════\n");
    
    char type_str[32], priv_str[32];
    format_action_type(card->type, type_str, sizeof(type_str));
    format_privilege(card->privilege, priv_str, sizeof(priv_str));
    
    printf("Title: %s\n", card->title);
    printf("Type: %s | Privilege: %s | Risk Level: %u%%\n", 
           type_str, priv_str, card->impact.risk_level);
    printf("Command: %s\n", card->command);
    
    if (strlen(card->summary) > 0) {
        printf("\nSummary:\n%s\n", card->summary);
    }
    
    if (strlen(card->details) > 0) {
        printf("\nDetails:\n%s\n", card->details);
    }
    
    /* Show steps */
    if (card->step_count > 0) {
        printf("\nSteps:\n");
        for (uint32_t i = 0; i < card->step_count; i++) {
            printf("  %u. %s %s\n", i + 1, 
                   card->steps[i].completed ? "✓" : "○",
                   card->steps[i].description);
        }
    }
    
    /* Show permissions */
    if (card->permission_count > 0) {
        printf("\nRequired permissions:\n");
        for (uint32_t i = 0; i < card->permission_count; i++) {
            printf("  • %s\n", card->permissions[i]);
        }
    }
    
    /* Show impact assessment */
    printf("\nImpact Assessment:\n");
    printf("  Reversible: %s\n", card->impact.reversible ? "Yes" : "No");
    printf("  Persistent: %s\n", card->impact.persistent ? "Yes" : "No");
    printf("  Security Impact: %s\n", card->impact.affects_security ? "Yes" : "No");
    printf("  Privacy Impact: %s\n", card->impact.affects_privacy ? "Yes" : "No");
    printf("  Data Impact: %s\n", card->impact.affects_data ? "Yes" : "No");
    
    if (card->can_undo) {
        printf("  Undo Command: %s\n", card->undo_command);
    }
    
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("Options: [A]pprove | [D]eny | [E]dit | [?] More info\n");
    printf("Response: ");
    fflush(stdout);
    
    return 0;
}

/* Wait for user response */
action_response_t action_card_wait_response(action_card_t* card, uint32_t timeout_sec) {
    if (!card) return RESPONSE_DENIED;
    
    /* Implement timeout using alarm signal */
    if (timeout_sec > 0) {
        signal(SIGALRM, timeout_handler);
        alarm(timeout_sec);
    }
    
    char response[64];
    if (fgets(response, sizeof(response), stdin)) {
        char first_char = response[0];
        
        switch (first_char) {
            case 'A':
            case 'a':
                card->response = RESPONSE_APPROVED;
                card->responded_at = get_timestamp();
                printf("Action approved.\n");
                break;
                
            case 'D':
            case 'd':
                card->response = RESPONSE_DENIED;
                card->responded_at = get_timestamp();
                printf("Action denied.\n");
                break;
                
            case 'E':
            case 'e':
                card->response = RESPONSE_EDITED;
                card->responded_at = get_timestamp();
                printf("Enter new command: ");
                fflush(stdout);
                
                char new_cmd[ACTION_COMMAND_SIZE];
                if (fgets(new_cmd, sizeof(new_cmd), stdin)) {
                    /* Remove newline */
                    size_t len = strlen(new_cmd);
                    if (len > 0 && new_cmd[len - 1] == '\n') {
                        new_cmd[len - 1] = '\0';
                    }
                    
                    action_card_edit(card, new_cmd);
                    printf("Command updated to: %s\n", card->command);
                }
                break;
                
            default:
                printf("Invalid response. Please enter A, D, or E.\n");
                return action_card_wait_response(card, timeout_sec);
        }
    } else {
        card->response = RESPONSE_TIMEOUT;
    }
    
    return card->response;
}

/* Edit action card */
int action_card_edit(action_card_t* card, const char* new_command) {
    if (!card || !new_command) return -1;
    
    strncpy(card->command, new_command, sizeof(card->command) - 1);
    card->command[sizeof(card->command) - 1] = '\0';
    
    /* Re-assess impact */
    action_card_assess_impact(card);
    
    return 0;
}

/* Execute action */
int action_card_execute(action_card_t* card) {
    if (!card) return -1;
    
    if (card->response != RESPONSE_APPROVED && card->response != RESPONSE_EDITED && !card->force) {
        printf("[ActionCard] Cannot execute: action not approved\n");
        return -1;
    }
    
    printf("[ActionCard] Executing action card #%llu\n", card->id);
    
    card->executed_at = get_timestamp();
    card->executed = true;
    
    /* Execute the command */
    card->exit_code = execute_system_command(card->command, card->output, sizeof(card->output));
    
    printf("[ActionCard] Execution completed with exit code %d\n", card->exit_code);
    
    /* Log to audit trail */
    action_card_audit_log(card);
    
    return card->exit_code;
}

/* Undo action */
int action_card_undo(uint64_t action_id) {
    /* Find action in audit history */
    for (uint32_t i = 0; i < g_action_system.audit_count; i++) {
        action_card_t* card = g_action_system.audit_history[i];
        if (card && card->id == action_id && card->can_undo) {
            printf("[ActionCard] Undoing action #%llu: %s\n", action_id, card->undo_command);
            
            char undo_output[1024];
            int result = execute_system_command(card->undo_command, undo_output, sizeof(undo_output));
            
            printf("[ActionCard] Undo completed with exit code %d\n", result);
            return result;
        }
    }
    
    printf("[ActionCard] Action #%llu not found or not reversible\n", action_id);
    return -1;
}

/* Add consent rule */
int action_card_add_consent_rule(action_type_t type, consent_policy_t policy, const char* pattern) {
    if (!pattern || g_action_system.consent_rule_count >= 64) return -1;
    
    consent_rule_t* rule = &g_action_system.consent_rules[g_action_system.consent_rule_count];
    rule->action_type = type;
    rule->policy = policy;
    strncpy(rule->pattern, pattern, sizeof(rule->pattern) - 1);
    rule->pattern[sizeof(rule->pattern) - 1] = '\0';
    rule->created_at = get_timestamp();
    rule->active = true;
    
    g_action_system.consent_rule_count++;
    
    printf("[ActionCard] Added consent rule: %s -> %d\n", pattern, policy);
    return 0;
}

/* Remove consent rule */
int action_card_remove_consent_rule(uint64_t rule_id) {
    if (rule_id >= g_action_system.consent_rule_count) return -1;
    
    g_action_system.consent_rules[rule_id].active = false;
    return 0;
}

/* Check consent for action */
consent_policy_t action_card_check_consent(action_type_t type, const char* command) {
    if (!command) return CONSENT_ALWAYS_ASK;
    
    /* Check consent rules */
    for (uint32_t i = 0; i < g_action_system.consent_rule_count; i++) {
        consent_rule_t* rule = &g_action_system.consent_rules[i];
        if (rule->active && rule->action_type == type) {
            if (pattern_matches(rule->pattern, command)) {
                return rule->policy;
            }
        }
    }
    
    return CONSENT_ALWAYS_ASK;
}

/* Get settings */
void action_card_get_settings(action_card_settings_t* settings) {
    if (!settings) return;
    *settings = g_action_system.settings;
}

/* Set settings */
void action_card_set_settings(const action_card_settings_t* settings) {
    if (!settings) return;
    g_action_system.settings = *settings;
}

/* Add to audit log */
int action_card_audit_log(const action_card_t* card) {
    if (!card || !g_action_system.settings.audit_log_enabled) return -1;
    
    if (g_action_system.audit_count < sizeof(g_action_system.audit_history) / sizeof(g_action_system.audit_history[0])) {
        /* Create a copy for audit history */
        action_card_t* audit_card = malloc(sizeof(action_card_t));
        if (audit_card) {
            *audit_card = *card;
            g_action_system.audit_history[g_action_system.audit_count++] = audit_card;
        }
    }
    
    printf("[ActionCard] Logged action #%llu to audit trail\n", card->id);
    return 0;
}

/* Get audit history */
int action_card_get_audit_history(action_card_t** cards, uint32_t* count, uint32_t max) {
    if (!cards || !count) return -1;
    
    uint32_t to_copy = (g_action_system.audit_count < max) ? g_action_system.audit_count : max;
    
    for (uint32_t i = 0; i < to_copy; i++) {
        cards[i] = g_action_system.audit_history[i];
    }
    
    *count = to_copy;
    return 0;
}

/* Destroy action card */
void action_card_destroy(action_card_t* card) {
    if (!card) return;
    
    /* Remove from pending actions */
    for (uint32_t i = 0; i < g_action_system.pending_count; i++) {
        if (g_action_system.pending_actions[i] == card) {
            /* Shift remaining items */
            for (uint32_t j = i; j < g_action_system.pending_count - 1; j++) {
                g_action_system.pending_actions[j] = g_action_system.pending_actions[j + 1];
            }
            g_action_system.pending_count--;
            break;
        }
    }
    
    free(card->undo_data);
    free(card);
}

/* Render action card for display */
void action_card_render(const action_card_t* card, char* output, uint32_t output_size) {
    if (!card || !output || output_size == 0) return;
    
    char type_str[32], priv_str[32];
    format_action_type(card->type, type_str, sizeof(type_str));
    format_privilege(card->privilege, priv_str, sizeof(priv_str));
    
    snprintf(output, output_size,
        "Action Card #%llu\n"
        "Title: %s\n"
        "Type: %s | Privilege: %s\n"
        "Command: %s\n"
        "Risk Level: %u%%\n"
        "Status: %s\n",
        card->id, card->title, type_str, priv_str, card->command,
        card->impact.risk_level,
        card->executed ? "Executed" : "Pending");
}
