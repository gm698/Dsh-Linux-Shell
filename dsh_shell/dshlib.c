#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

int last_rc = 0;

// Allocate memory for the command buffer
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return ERR_MEMORY;
    }
    cmd_buff->_cmd_buffer = (char *)malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    return OK;
}

// Free memory for the command buffer
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return OK;
    }
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

// Clear the command buffer
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) {
        return OK;
    }
    cmd_buff->argc = 0;
    if (cmd_buff->_cmd_buffer != NULL) {
        memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    }
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

// Build the command buffer from the input command line
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    if (cmd_line == NULL || cmd_buff == NULL) {
        return ERR_CMD_ARGS_BAD;
    }
    
    if (alloc_cmd_buff(cmd_buff) != OK) {
        fprintf(stderr, "Error allocating command buffer\n");
        return ERR_MEMORY;
    }

    clear_cmd_buff(cmd_buff);
    
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0'; // Ensure null-termination

    char *str = cmd_buff->_cmd_buffer;
    char *start = str;
    int arg_index = 0;
    bool in_quotes = false;

    while (*str) {
        if (*str == '"') {
            in_quotes = !in_quotes;
            memmove(str, str + 1, strlen(str)); // Remove the quote
            continue;
        }

        if (!in_quotes && isspace((unsigned char)*str)) {
            if (str > start) {  // End of argument
                *str = '\0';
                cmd_buff->argv[arg_index++] = start;
            }
            start = str + 1;
        }

        str++;
    }

    if (start != str) {  // Last argument
        cmd_buff->argv[arg_index++] = start;
    }

    cmd_buff->argc = arg_index;
    cmd_buff->argv[arg_index] = NULL;

    return OK;
}

// Match a command to a built-in command type
Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "rc") == 0) {
        return BI_RC;
    }
    return BI_NOT_BI;
}

// Execute a built-in command
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argv[0] == NULL) {
        return BI_NOT_BI;
    }

    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);

    if (cmd_type == BI_CMD_EXIT) {
        exit(0);
    } else if (cmd_type == BI_CMD_CD) {
        if (cmd->argc > 1) {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
            }
        }
        return BI_EXECUTED;
    } else if (cmd_type == BI_RC) {
        printf("%d\n", last_rc);
        return BI_EXECUTED;
    }
    return BI_NOT_BI;
}

// Build a list of commands from the input command line
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (cmd_line == NULL || clist == NULL) {
        return ERR_CMD_ARGS_BAD;
    }
    clist->num = 0;
    char *cmd_start = cmd_line;
    char *cmd_end = strchr(cmd_start, PIPE_CHAR);

    while (cmd_end != NULL) {
        *cmd_end = '\0'; // Temporarily replace the pipe with a null character

        if (build_cmd_buff(cmd_start, &clist->commands[clist->num]) != OK) {
            // Free any previously allocated buffers
            for (int i = 0; i < clist->num; i++) {
                free_cmd_buff(&clist->commands[i]);
            }
            return ERR_MEMORY;
        }

        clist->num++;
        cmd_start = cmd_end + 1;
        cmd_end = strchr(cmd_start, PIPE_CHAR);
    }

    // Handle the last command (no pipe after it)
    if (build_cmd_buff(cmd_start, &clist->commands[clist->num]) != OK) {
        // Free any previously allocated buffers
        for (int i = 0; i < clist->num; i++) {
            free_cmd_buff(&clist->commands[i]);
        }
        return ERR_MEMORY;
    }
    clist->num++;

    return OK;
}

// Execute a single command
int exec_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argv[0] == NULL) {
        return ERR_CMD_ARGS_BAD;
    }

    if (strcmp(cmd->argv[0], "echo") == 0) {
        if (cmd->argc > 1) {
            for (int i = 1; i < cmd->argc; i++) {
                printf("%s", cmd->argv[i]);
                if (i < cmd->argc - 1) {
                    printf(" ");
                }
            }
            printf("\n");
        } else {
            printf("\n");
        }
        return OK;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        last_rc = errno;
        exit(errno);
    } else {
        int status;
        waitpid(pid, &status, 0);
        last_rc = WEXITSTATUS(status);
        if (last_rc != 0) {
            if (last_rc == ENOENT) {
                printf("Command not found in PATH\n");
            } else if (last_rc == EACCES) {
                printf("Permission denied\n");
            } else {
                printf("Error executing command\n");
            }
        }
        return OK;
    }
}

// Execute a pipeline of commands
int execute_pipeline(command_list_t *clist) {
    if (clist == NULL || clist->num == 0) {
        return ERR_CMD_ARGS_BAD;
    }

    int pipefd[2];
    pid_t pid;
    int prev_pipefd[2] = {-1, -1};

    for (int i = 0; i < clist->num; i++) {
        cmd_buff_t *cmd = &clist->commands[i];

        if (i < clist->num - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return ERR_EXEC_CMD;
            }
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            return ERR_EXEC_CMD;
        }

        if (pid == 0) {  // Child process
            if (prev_pipefd[0] != -1) {
                dup2(prev_pipefd[0], STDIN_FILENO);
                close(prev_pipefd[0]);
            }

            if (i < clist->num - 1) {
                close(pipefd[0]);  // Close unused read end
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            execvp(cmd->argv[0], cmd->argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {  // Parent process
            if (prev_pipefd[0] != -1) {
                close(prev_pipefd[0]);
            }
            if (i < clist->num - 1) {
                close(pipefd[1]);
                prev_pipefd[0] = pipefd[0];  // Set for next iteration
            }

            waitpid(pid, NULL, 0);  // Wait for child to finish
        }
    }

    return OK;
}

// Main command loop
int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    command_list_t clist;

    printf("%s", SH_PROMPT); // Print the initial prompt

    while (1) {
        if (fgets(cmd_line, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_line[strcspn(cmd_line, "\n")] = '\0'; // Remove newline

        if (strlen(cmd_line) == 0) continue;

        if (build_cmd_list(cmd_line, &clist) != OK) {
            fprintf(stderr, "Error building command list\n");
            continue;
        }

        if (clist.num == 0) {
            continue;
        }

        if (exec_built_in_cmd(&clist.commands[0]) == BI_NOT_BI) {
            execute_pipeline(&clist);
        }

        // Free memory after command execution
        for (int i = 0; i < clist.num; i++) {
            free_cmd_buff(&clist.commands[i]);
        }

        printf("%s", SH_PROMPT); // Print prompt for the next command
    }
    return OK;
}