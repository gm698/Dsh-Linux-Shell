#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"


int exec_remote_cmd_loop(char *address, int port) {
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    int cli_socket;
    ssize_t io_size;
    int is_eof;

    if (cmd_buff == NULL || rsp_buff == NULL) {
        return ERR_MEMORY;
    }

    cli_socket = start_client(address, port);
    if (cli_socket < 0) {
        perror("start client");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    while (1) {
        printf("%s", SH_PROMPT);  // Print the prompt

        if (fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin) == NULL) {
            break;  // Exit on EOF or error
        }

        // Remove newline character from the command
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Send the command to the server
        int send_len = strlen(cmd_buff) + 1;  // Include null terminator
        int sent_len = send(cli_socket, cmd_buff, send_len, 0);
        if (sent_len != send_len) {
            perror("send");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

        // Receive responses from the server
        while ((io_size = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0))) {
            if (io_size < 0) {
                perror("recv");
                return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
            }

            // Check if the last byte is EOF
            is_eof = (rsp_buff[io_size - 1] == RDSH_EOF_CHAR) ? 1 : 0;

            // Print the received data
            printf("%.*s", (int)io_size - is_eof, rsp_buff);

            if (is_eof) {
                break;  // Exit the recv loop if EOF is received
            }
        }

        // Check if the command is "exit" to terminate the client
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            break;
        }
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}


int start_client(char *server_ip, int port) {
    struct sockaddr_in addr;
    int cli_socket;
    int ret;

    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(port);

    ret = connect(cli_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }

    return cli_socket;
}


int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    // If a valid socket number, close it.
    if (cli_socket > 0) {
        close(cli_socket);
    }

    // Free up the buffers
    free(cmd_buff);
    free(rsp_buff);

    // Echo the return value that was passed as a parameter
    return rc;
}