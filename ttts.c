#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define BOARD_SIZE 9

enum Role
{
    X,
    O
};
enum Outcome
{
    WIN,
    LOSS,
    DRAW
};

void send_message(int sockfd, char *code, char *field3, char *field4, char *identifier, char *extra_token)
{
    char buffer[BUF_SIZE];
    if (field4 == NULL)
    {
        field4 = ""; // set field4 to empty string if it's NULL
    }
    if (identifier == NULL)
    {
        identifier = ""; // set identifier to empty string if it's NULL
    }
    /* printf("SERVER SENDING MESSAGE\n");
    printf("Code: %s ", code);
    printf("Field3: %s ", field3);
    printf("Field4: %s ", field4);
    printf("Identifier: %s ", identifier);
    printf("Extra token: %s \n", extra_token ? extra_token : "");
 */ sprintf(buffer, "%s|%s|%s|%s|%s|", code, field3, field4 ? field4 : "", identifier ? identifier : "", extra_token ? extra_token : "");
    // printf("Buffer: %s\n", buffer);
    // printf("FINISHING SENDING MESSAGE\n");
    printf("Sent message: %s\n", buffer);
    ssize_t total_bytes_written = 0;
    ssize_t bytes_written;
    while (total_bytes_written < strlen(buffer))
    {
        bytes_written = write(sockfd, buffer + total_bytes_written, strlen(buffer) - total_bytes_written);
        if (bytes_written <= 0)
        {
            if (errno == EINTR) // interrupted by signal
            {
                continue;
            }
            perror("write");
            exit(1);
        }
        total_bytes_written += bytes_written;
    }
    // Clear buffer
    memset(buffer, 0, BUF_SIZE);
}

void recv_message(int sockfd, char *code, char *field2, char **field3, char *field4, char *field5)
{
    char buffer[BUF_SIZE];
    int n = read(sockfd, buffer, BUF_SIZE - 1);
    if (n < 0)
    {
        perror("read");
        exit(1);
    }
    buffer[n] = '\0';
    printf("Received message: %s\n", buffer);

    char *token = strtok(buffer, "|");

    // extract code
    strncpy(code, token, 4);
    code[4] = '\0';
    // printf("Code: %s ", code);

    // extract field2
    token = strtok(NULL, "|");
    strncpy(field2, token, BUF_SIZE);
    field2[BUF_SIZE - 1] = '\0';
    // printf("Field 2: %s ", field2);

    // extract field3
    token = strtok(NULL, "|");
    *field3 = strdup(token);
    // printf("Field 3: %s ", *field3);

    // extract field4
    token = strtok(NULL, "|");
    strncpy(field4, token, BUF_SIZE);
    field4[BUF_SIZE - 1] = '\0';
    // printf("Field 4: %s ", field4);

    // extract field5
    token = strtok(NULL, "|");
    strncpy(field5, token, BUF_SIZE);
    field5[BUF_SIZE - 1] = '\0';
    // printf("Field 5: %s\n", field5);

    // Clear buffer
    memset(buffer, 0, BUF_SIZE);

    // Free field3 memory
    free(*field3);
}

// Function to print the current state of the board
void print_board(char *board)
{
    printf("%c|%c|%c\n", board[0], board[1], board[2]);
    printf("%c|%c|%c\n", board[3], board[4], board[5]);
    printf("%c|%c|%c\n", board[6], board[7], board[8]);
}

// Function to check if the board is full
int is_board_full(char *board)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        if (board[i] == '.')
        {
            return 0;
        }
    }
    return 1;
}

// Function to check if a player has won
int is_winner(char *board, char player)
{
    // Check rows
    for (int i = 0; i < BOARD_SIZE; i += 3)
    {
        if (board[i] == player && board[i + 1] == player && board[i + 2] == player)
        {
            return 1;
        }
    }

    // Check columns
    for (int i = 0; i < 3; i++)
    {
        if (board[i] == player && board[i + 3] == player && board[i + 6] == player)
        {
            return 1;
        }
    }

    // Check diagonals
    if (board[0] == player && board[4] == player && board[8] == player)
    {
        return 1;
    }
    if (board[2] == player && board[4] == player && board[6] == player)
    {
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Bind socket to port
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    // Listen for connections
    if (listen(sockfd, 1) < 0)
    {
        perror("listen");
        return 1;
    }

    printf("Waiting for players...\n");

    // Accept first client connection
    int player1_fd = accept(sockfd, NULL, NULL);
    if (player1_fd < 0)
    {
        perror("accept");
        return 1;
    }
    printf("Player 1 connected.\n");

    // Send WAIT message to player 1
    send_message(player1_fd, "WAIT", "3", "3", NULL, "");
    int player2_fd = accept(sockfd, NULL, NULL);
    if (player2_fd < 0)
    {
        perror("accept");
        return 1;
    }
    printf("Player 2 connected.\n");
    send_message(player2_fd, "WAIT", "3", "3", NULL, "");

    // Send BEGN message to player 1
    char player1_id[BUF_SIZE];
    sprintf(player1_id, "%d", player1_fd);
    // printf("\nPlayer 1 Info id,fd: %s,%d\n", player1_id, player1_fd);

    // Send BEGN message to player 2
    char player2_id[BUF_SIZE];
    sprintf(player2_id, "%d", player2_fd);
    // printf("\nPlayer 2 Info id,fd: %s,%d\n", player2_id, player2_fd);

    send_message(player1_fd, "BEGN", "X", "Player 2", player1_id, player2_id);
    // printf("\nFIRST BEGN IS DONE\n");
    send_message(player2_fd, "BEGN", "O", "Player 1", player2_id, player1_id);
    // printf("\nSECOND BEGN IS DONE\n");

    // Initialize board
    char board[BOARD_SIZE] = ".........";
    printf("Initialized board: %s\n", board);

    // Player 1 always starts as X
    enum Role current_player = X;
    int current_player_fd = player1_fd;
    int other_player_fd = player2_fd;

    printf("Starting player: %c\n", current_player == X ? 'X' : 'O');

    // Game loop
    printf("Current player: %c, current_player_fd: %d, other_player_fd: %d\n", current_player == X ? 'X' : 'O', current_player_fd, other_player_fd);
    char other_player_fd_str[16];
    snprintf(other_player_fd_str, sizeof(other_player_fd_str), "%d", other_player_fd);

    send_message(current_player_fd, "MOVD", current_player == X ? "X" : "O", NULL, NULL, other_player_fd_str);

    // printf("\nAFTER MOVE\n");
    printf("Sent MOVD message to player %c\n", current_player == X ? 'X' : 'O');
    while (1)
    {
        // printf("\nIN THE SERVER WHILE LOOP\n");

        // Receive message from current player
        char code[5], field2[BUF_SIZE], field3[BUF_SIZE], field4[BUF_SIZE], field5[BUF_SIZE];
        // printf("\nABOUT TO RECIEVE MOVE FROM PLAYER %c\n",current_player == X ? 'X' : 'O');
        recv_message(current_player_fd, code, field2, &field3, field4, field5);
        //printf("Received message from player %c with code %s and position %s\n", current_player == X ? 'X' : 'O', code, field4);

        if (strcmp(code, "MOVE") == 0 && field4 != NULL && field4[0] != '\0')
        {
            // printf("\nIN SERVER MOVE IF\n");
            //  Update board with player's move
            int row = field4[0] - '0' - 1;
            int col = field4[2] - '0' - 1;
            if (board[row * 3 + col] == '.')
            {
                board[row * 3 + col] = current_player == X ? 'X' : 'O';
                printf("Board after player %c move:\n", current_player == X ? 'X' : 'O');
                print_board(board);

                // Check for winner or draw
                if (is_winner(board, current_player == X ? 'X' : 'O'))
                {
                    send_message(current_player_fd, "OVER", "W", "You won!", NULL, "");
                    send_message(other_player_fd, "OVER", "L", "You lost!", NULL, "");
                    printf("Player %c won!\n", current_player == X ? 'X' : 'O');
                    break;
                }
                else if (is_board_full(board))
                {
                    send_message(current_player_fd, "OVER", "D", "Draw!", NULL, "");
                    send_message(other_player_fd, "OVER", "D", "Draw!", NULL, "");
                    printf("Draw!\n");
                    break;
                }
                else
                {
                    // Switch players
                    current_player = current_player == X ? O : X;
                    current_player_fd = current_player == X ? player1_fd : player2_fd;
                    other_player_fd = current_player == X ? player2_fd : player1_fd;
                    printf("Switched players, current player: %c\n", current_player == X ? 'X' : 'O');
                    send_message(current_player_fd, "MOVD", current_player == X ? "X" : "O", "", "", "");
                    printf("Sent MOVD message to player %c\n", current_player == X ? 'X' : 'O');
                }
            }
            else
            {
                // Invalid move
                printf("!Invalid move by player %c\n", current_player == X ? 'X' : 'O');
                break;
                // send_message(current_player_fd, "INVL", "Invalid move.", "", "", other_player_fd);
            }
        }

        else if (strcmp(code, "RSGN") == 0)
        {
            // Player has resigned
            send_message(other_player_fd, "OVER", "W", "Opponent resigned.", "", "");
            send_message(current_player_fd, "OVER", "L", "You resigned.", "", "");
            printf("Player %c has resigned!\n", current_player == X ? 'X' : 'O');
            printf("Closing sockets...\n");
            close(player1_fd);
            close(player2_fd);
            close(sockfd);

            return 0;
            break;
        }
        else if (strcmp(code, "DRAW") == 0)
        {
            if (field2[0] == 'S')
            {
                printf("Player %c suggested a draw.\n", current_player == X ? 'X' : 'O');
                // Draw suggested by player
                send_message(other_player_fd, "DRAW", "S", "", "", current_player_fd);
            }
            else if (field2[0] == 'A')
            {
                printf("Draw accepted by both players.\n");
                // Draw accepted by player
                send_message(current_player_fd, "OVER", "D", "Draw!", "", other_player_fd);
                send_message(other_player_fd, "OVER", "D", "Draw!", "", current_player_fd);
                break;
            }
            else if (field2[0] == 'R')
            {
                printf("Player %c rejected the draw.\n", current_player == X ? 'X' : 'O');
                // Draw rejected by player
                send_message(current_player_fd, "MOVD", current_player == X ? "X" : "O", "", "", "");
            }
        }
        else
        {
            // Invalid message
            printf("!Invalid message received from player %c.\n", current_player == X ? 'X' : 'O');
            send_message(current_player_fd, "INVL", "Invalid message.", "", "", other_player_fd);
        }
    }
    // Close sockets
    printf("Closing sockets...\n");
    close(player1_fd);
    close(player2_fd);
    close(sockfd);

    return 0;
}