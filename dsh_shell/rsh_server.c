#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>



#include "dshlib.h"
#include "rshlib.h"

/
int start_server(char *ifaces, int port, int is_threaded) {
    int svr_socket;
    int rc;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        int err_code = svr_socket;  
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);

    return rc;
}


int stop_server(int svr_socket) {
    return close(svr_socket);
}


int boot_server(char *ifaces, int port) {
    int svr_socket;
    int ret;
    struct sockaddr_in addr;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ifaces);
    addr.sin_port = htons(port);

    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        return ERR_RDSH_COMMUNICATION;
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}


int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc = OK;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    while (1) {
        cli_socket = accept(svr_socket, (struct sockaddr *)&cli_addr, &cli_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        rc = exec_client_requests(cli_socket);
        if (rc == OK_EXIT) {
            break;
        }

        close(cli_socket);
    }

    return rc;
}


int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int rc;
    int cmd_rc;
    int last_rc;
    char *io_buff;

    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL) {
        return ERR_RDSH_SERVER;
    }

    while (1) {
        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (io_size <= 0) {
            break;
        }

        io_buff[io_size - 1] = '\0';  // Null-terminate the command

        if (build_cmd_list(io_buff, &cmd_list) != OK) {
            fprintf(stderr, "Error building command list\n");
            continue;
        }

        cmd_rc = rsh_execute_pipeline(cli_socket, &cmd_list);
        send_message_eof(cli_socket);

        if (cmd_rc == EXIT_SC) {
            break;
        }
    }

    free(io_buff);
    return OK;
}


int send_message_eof(int cli_socket) {
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

int send_message_string(int cli_socket, char *buff) {
    int send_len = strlen(buff) + 1;
    int sent_len = send(cli_socket, buff, send_len, 0);

    if (sent_len != send_len) {
        return ERR_RDSH_COMMUNICATION;
    }

    return send_message_eof(cli_socket);
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];  // Array of pipes
    pid_t pids[clist->num];
    int pids_st[clist->num];  // Array to store process IDs
    int exit_code;

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_RDSH_CMD_EXEC;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return ERR_RDSH_CMD_EXEC;
        }

        if (pids[i] == 0) {  // Child process
            // Redirect stdin for the first process to the client socket
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            }

            // Redirect stdout for the last process to the client socket
            if (i == clist->num - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                // Redirect stdout to the write end of the pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Redirect stdin for subsequent processes to the read end of the previous pipe
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Close all pipe ends in the child process
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute the command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");  // If execvp fails
            exit(EXIT_FAILURE);
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    // Get the exit code of the last process in the pipeline
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);

    // Check if any command in the pipeline requested an exit
    for (int i = 0; i < clist->num; i++) {
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC) {
            exit_code = EXIT_SC;
            break;
        }
    }

    return exit_code;
}


Built_In_Cmds rsh_match_command(const char *input) {
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}


Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype) {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}