/*
 * Terminal Command Parser
 * Advanced command line parsing with pipes, redirection, and scripting
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include "syscall.h"

/* Parser token types */
typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,    if (cmd->background) {
        printf("[Parser] Running in background\n");
        /* For background processes, fork and execute */
        long pid = sys_fork();
        if (pid == 0) {
            /* Child process - execute command */
            const char* envp[] = {NULL};
            sys_execve(cmd->args[0], (const char* const*)cmd->args, envp);
            sys_exit(1); /* If execve fails */
        } else if (pid > 0) {
            printf("[%ld]\n", pid);
            return 0;
        } else {
            printf("[Parser] Failed to fork background process\n");
            return -1;
        }
    } else {
        /* Foreground execution */
        long pid = sys_fork();
        if (pid == 0) {
            /* Child process - execute command */
            const char* envp[] = {NULL};
            sys_execve(cmd->args[0], (const char* const*)cmd->args, envp);
            sys_exit(1); /* If execve fails */
        } else if (pid > 0) {
            /* Parent process - wait for child */
            int status;
            sys_wait4(pid, &status, 0, NULL);
            return 0;
        } else {
            printf("[Parser] Failed to fork process\n");
            return -1;
        }
    }OKEN_BACKGROUND,
    TOKEN_SEMICOLON,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_EOF
} token_type_t;

typedef struct {
    token_type_t type;
    char* value;
} token_t;

/* Command structure */
typedef struct command {
    char** args;
    int argc;
    char* input_file;
    char* output_file;
    bool append_output;
    struct command* next;
} command_t;

/* Pipeline structure */
typedef struct {
    command_t* commands;
    bool background;
    bool conditional_and;  /* && */
    bool conditional_or;   /* || */
} pipeline_t;

/* Parser state */
typedef struct {
    const char* input;
    int pos;
    int length;
    char current_char;
} parser_state_t;

/* Function prototypes */
static void advance(parser_state_t* state);
static void skip_whitespace(parser_state_t* state);
static token_t* next_token(parser_state_t* state);
static char* parse_word(parser_state_t* state);
static command_t* parse_command(parser_state_t* state, token_t** current_token);
static pipeline_t* parse_pipeline(parser_state_t* state);
static int execute_pipeline(pipeline_t* pipeline);
static void free_command(command_t* cmd);
static void free_pipeline(pipeline_t* pipeline);
static char* expand_variables(const char* str);
static char* expand_tilde(const char* str);

/* Initialize parser state */
static void init_parser_state(parser_state_t* state, const char* input) {
    state->input = input;
    state->pos = 0;
    state->length = strlen(input);
    state->current_char = (state->length > 0) ? input[0] : '\0';
}

/* Advance to next character */
static void advance(parser_state_t* state) {
    state->pos++;
    if (state->pos >= state->length) {
        state->current_char = '\0';
    } else {
        state->current_char = state->input[state->pos];
    }
}

/* Skip whitespace */
static void skip_whitespace(parser_state_t* state) {
    while (state->current_char == ' ' || state->current_char == '\t') {
        advance(state);
    }
}

/* Parse a word (command, argument, or filename) */
static char* parse_word(parser_state_t* state) {
    char buffer[1024];
    int buf_pos = 0;
    bool in_quotes = false;
    bool in_single_quotes = false;
    bool escaped = false;
    
    while (state->current_char != '\0' && buf_pos < sizeof(buffer) - 1) {
        if (escaped) {
            buffer[buf_pos++] = state->current_char;
            escaped = false;
            advance(state);
        } else if (state->current_char == '\\') {
            escaped = true;
            advance(state);
        } else if (state->current_char == '"' && !in_single_quotes) {
            in_quotes = !in_quotes;
            advance(state);
        } else if (state->current_char == '\'' && !in_quotes) {
            in_single_quotes = !in_single_quotes;
            advance(state);
        } else if (!in_quotes && !in_single_quotes) {
            if (state->current_char == ' ' || state->current_char == '\t' ||
                state->current_char == '|' || state->current_char == '<' ||
                state->current_char == '>' || state->current_char == '&' ||
                state->current_char == ';') {
                break;
            }
            buffer[buf_pos++] = state->current_char;
            advance(state);
        } else {
            buffer[buf_pos++] = state->current_char;
            advance(state);
        }
    }
    
    buffer[buf_pos] = '\0';
    return strdup(buffer);
}

/* Get next token */
static token_t* next_token(parser_state_t* state) {
    skip_whitespace(state);
    
    token_t* token = malloc(sizeof(token_t));
    if (!token) return NULL;
    
    token->value = NULL;
    
    if (state->current_char == '\0') {
        token->type = TOKEN_EOF;
        return token;
    }
    
    switch (state->current_char) {
        case '|':
            token->type = TOKEN_PIPE;
            advance(state);
            break;
            
        case '<':
            token->type = TOKEN_REDIRECT_IN;
            advance(state);
            break;
            
        case '>':
            advance(state);
            if (state->current_char == '>') {
                token->type = TOKEN_REDIRECT_APPEND;
                advance(state);
            } else {
                token->type = TOKEN_REDIRECT_OUT;
            }
            break;
            
        case '&':
            advance(state);
            if (state->current_char == '&') {
                token->type = TOKEN_AND;
                advance(state);
            } else {
                token->type = TOKEN_BACKGROUND;
            }
            break;
            
        case ';':
            token->type = TOKEN_SEMICOLON;
            advance(state);
            break;
            
        default:
            token->type = TOKEN_WORD;
            token->value = parse_word(state);
            break;
    }
    
    return token;
}

/* Parse a single command */
static command_t* parse_command(parser_state_t* state, token_t** current_token) {
    command_t* cmd = calloc(1, sizeof(command_t));
    if (!cmd) return NULL;
    
    char* args[64];  /* Max 64 arguments */
    int argc = 0;
    
    /* Parse command and arguments */
    while ((*current_token)->type == TOKEN_WORD) {
        if (argc < 63) {  /* Leave room for NULL terminator */
            args[argc] = expand_variables((*current_token)->value);
            args[argc] = expand_tilde(args[argc]);
            argc++;
        }
        
        free((*current_token)->value);
        free(*current_token);
        *current_token = next_token(state);
    }
    
    /* Handle redirections */
    while ((*current_token)->type == TOKEN_REDIRECT_IN ||
           (*current_token)->type == TOKEN_REDIRECT_OUT ||
           (*current_token)->type == TOKEN_REDIRECT_APPEND) {
        
        token_type_t redirect_type = (*current_token)->type;
        free(*current_token);
        *current_token = next_token(state);
        
        if ((*current_token)->type != TOKEN_WORD) {
            printf("parser: syntax error: expected filename after redirection\n");
            free_command(cmd);
            return NULL;
        }
        
        char* filename = expand_variables((*current_token)->value);
        filename = expand_tilde(filename);
        
        switch (redirect_type) {
            case TOKEN_REDIRECT_IN:
                cmd->input_file = filename;
                break;
            case TOKEN_REDIRECT_OUT:
                cmd->output_file = filename;
                cmd->append_output = false;
                break;
            case TOKEN_REDIRECT_APPEND:
                cmd->output_file = filename;
                cmd->append_output = true;
                break;
            default:
                break;
        }
        
        free((*current_token)->value);
        free(*current_token);
        *current_token = next_token(state);
    }
    
    if (argc == 0) {
        free_command(cmd);
        return NULL;
    }
    
    /* Copy arguments */
    cmd->args = malloc((argc + 1) * sizeof(char*));
    if (!cmd->args) {
        free_command(cmd);
        return NULL;
    }
    
    for (int i = 0; i < argc; i++) {
        cmd->args[i] = args[i];
    }
    cmd->args[argc] = NULL;
    cmd->argc = argc;
    
    return cmd;
}

/* Parse a pipeline (commands connected by pipes) */
static pipeline_t* parse_pipeline(parser_state_t* state) {
    pipeline_t* pipeline = calloc(1, sizeof(pipeline_t));
    if (!pipeline) return NULL;
    
    token_t* token = next_token(state);
    
    /* Parse first command */
    command_t* first_cmd = parse_command(state, &token);
    if (!first_cmd) {
        free_pipeline(pipeline);
        return NULL;
    }
    
    pipeline->commands = first_cmd;
    command_t* current_cmd = first_cmd;
    
    /* Parse additional commands in pipeline */
    while (token->type == TOKEN_PIPE) {
        free(token);
        token = next_token(state);
        
        command_t* next_cmd = parse_command(state, &token);
        if (!next_cmd) {
            free_pipeline(pipeline);
            return NULL;
        }
        
        current_cmd->next = next_cmd;
        current_cmd = next_cmd;
    }
    
    /* Check for background execution */
    if (token->type == TOKEN_BACKGROUND) {
        pipeline->background = true;
        free(token);
        token = next_token(state);
    }
    
    /* Check for conditional execution */
    if (token->type == TOKEN_AND) {
        pipeline->conditional_and = true;
    } else if (token->type == TOKEN_OR) {
        pipeline->conditional_or = true;
    }
    
    if (token) {
        if (token->value) free(token->value);
        free(token);
    }
    
    return pipeline;
}

/* Execute a pipeline */
static int execute_pipeline(pipeline_t* pipeline) {
    if (!pipeline || !pipeline->commands) {
        return -1;
    }
    
    /* For now, just execute the first command */
    command_t* cmd = pipeline->commands;
    
    printf("[Parser] Executing: %s", cmd->args[0]);
    for (int i = 1; i < cmd->argc; i++) {
        printf(" %s", cmd->args[i]);
    }
    printf("\n");
    
    if (cmd->input_file) {
        printf("[Parser] Input from: %s\n", cmd->input_file);
    }
    
    if (cmd->output_file) {
        printf("[Parser] Output to: %s %s\n", 
               cmd->output_file, cmd->append_output ? "(append)" : "(overwrite)");
    }
    
    if (pipeline->background) {
        printf("[Parser] Running in background\n");
    }
    
    /* Execute the command using fork and execve */
    if (!cmd->argv[0]) {
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        if (cmd->input_redirect) {
            int fd = sys_open(cmd->input_redirect, O_RDONLY);
            if (fd >= 0) {
                sys_dup2(fd, STDIN_FILENO);
                sys_close(fd);
            }
        }
        if (cmd->output_redirect) {
            int fd = sys_open(cmd->output_redirect, O_WRONLY | O_CREAT | O_TRUNC);
            if (fd >= 0) {
                sys_dup2(fd, STDOUT_FILENO);
                sys_close(fd);
            }
        }
        
        execve(cmd->argv[0], cmd->argv, NULL);
        exit(127); /* Command not found */
    } else if (pid > 0) {
        /* Parent process */
        if (!cmd->background) {
            int status;
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
        return 0;
    }
    return -1;
}

/* Variable expansion */
static char* expand_variables(const char* str) {
    if (!str) return NULL;
    
    /* Look for $ variables */
    char* result = malloc(strlen(str) * 2 + 256); /* Extra space for expansions */
    if (!result) return strdup(str);
    
    const char* src = str;
    char* dst = result;
    
    while (*src) {
        if (*src == '$' && *(src + 1)) {
            src++; /* Skip $ */
            
            /* Extract variable name */
            char varname[64];
            int i = 0;
            while (*src && (isalnum(*src) || *src == '_') && i < 63) {
                varname[i++] = *src++;
            }
            varname[i] = '\0';
            
            /* Expand common variables */
            if (strcmp(varname, "HOME") == 0) {
                strcpy(dst, "/home/user");
                dst += strlen("/home/user");
            } else if (strcmp(varname, "PATH") == 0) {
                strcpy(dst, "/bin:/usr/bin:/usr/local/bin");
                dst += strlen("/bin:/usr/bin:/usr/local/bin");
            } else if (strcmp(varname, "USER") == 0) {
                strcpy(dst, "user");
                dst += strlen("user");
            } else if (strcmp(varname, "PWD") == 0) {
                strcpy(dst, "/"); /* Current directory */
                dst += strlen("/");
            } else {
                /* Unknown variable - leave as is */
                *dst++ = '$';
                strcpy(dst, varname);
                dst += strlen(varname);
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    return result;
}

/* Tilde expansion */
static char* expand_tilde(const char* str) {
    if (!str || str[0] != '~') {
        return (char*)str;  /* No expansion needed */
    }
    
    /* Simple implementation - expand ~ to /home/user */
    char* expanded = malloc(strlen(str) + 32);
    if (!expanded) return (char*)str;
    
    strcpy(expanded, "/home/user");
    strcat(expanded, str + 1);
    
    free((char*)str);  /* Free original string */
    return expanded;
}

/* Free command structure */
static void free_command(command_t* cmd) {
    if (!cmd) return;
    
    if (cmd->args) {
        for (int i = 0; i < cmd->argc; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }
    
    free(cmd->input_file);
    free(cmd->output_file);
    
    if (cmd->next) {
        free_command(cmd->next);
    }
    
    free(cmd);
}

/* Free pipeline structure */
static void free_pipeline(pipeline_t* pipeline) {
    if (!pipeline) return;
    
    free_command(pipeline->commands);
    free(pipeline);
}

/* Main parsing function */
int terminal_parse_command(const char* line) {
    if (!line || strlen(line) == 0) {
        return 0;
    }
    
    parser_state_t state;
    init_parser_state(&state, line);
    
    pipeline_t* pipeline = parse_pipeline(&state);
    if (!pipeline) {
        return -1;
    }
    
    int result = execute_pipeline(pipeline);
    free_pipeline(pipeline);
    
    return result;
}

/* Parse multiple commands separated by ; */
int terminal_parse_multiple_commands(const char* line) {
    if (!line) return -1;
    
    char* line_copy = strdup(line);
    char* saveptr;
    char* cmd = strtok_r(line_copy, ";", &saveptr);
    int result = 0;
    
    while (cmd) {
        /* Skip leading whitespace */
        while (*cmd == ' ' || *cmd == '\t') cmd++;
        
        if (strlen(cmd) > 0) {
            int cmd_result = terminal_parse_command(cmd);
            if (cmd_result != 0) {
                result = cmd_result;
            }
        }
        
        cmd = strtok_r(NULL, ";", &saveptr);
    }
    
    free(line_copy);
    return result;
}
