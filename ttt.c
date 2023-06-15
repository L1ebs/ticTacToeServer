#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 2048

// Function to send a message to the server
// Function to send a message to the server
void send_message(int sockfd, char *code, char *field3, char *field4, char *identifier)
{
    char buffer[BUF_SIZE];
    sprintf(buffer, "%s|%d|%s|%s|%s|", code, (int)(strlen(field3) + strlen(field4) + strlen(identifier) + 3), field3, field4, identifier);
    int len = strlen(buffer);

    int n = write(sockfd, buffer, len);
    if (n < 0)
    {
        perror("write");
        exit(1);
    }
    printf("Client Sent message: %s\n", buffer);
    // Clear buffer
    memset(buffer, 0, BUF_SIZE);
}

void recv_message(int sockfd, char *code, char *field3, char **field4, char *identifier)
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

    char copy[BUF_SIZE];
    strcpy(copy, buffer); // make a copy of buffer

    char *token = strtok(copy, "|"); // use the copy instead of buffer
    if (token == NULL)
    {
        printf("Invalid message received: empty code\n");
        exit(1);
    }

    char tmp_code[BUF_SIZE];
    strcpy(tmp_code, token);
    // printf("Code: %s\n", tmp_code);

    token = strtok(NULL, "|");
    if (token == NULL)
    {
        printf("Invalid message received: empty field3\n");
        printf("Full message: %s\n", buffer);
        exit(1);
    }
    else
    {
        strcpy(field3, token);
        // printf("Field 3: %s\n", field3);
    }
    // printf("\nCHECKPOINT 1 FOR CODE: %s\n", tmp_code);

    if (strcmp(tmp_code, "BEGN") == 0)
    {
        token = strtok(NULL, "|");
        if (token == NULL)
        {
            printf("Invalid message received: empty field4\n");
            printf("Full message: %s\n", buffer);
            exit(1);
        }
        else
        {
            if (field4 != NULL)
            {
                *field4 = malloc(strlen(token) + 1);
                if (*field4 == NULL)
                {
                    perror("malloc");
                    exit(1);
                }
                strcpy(*field4, token);
                // printf("Field 4: %s\n", *field4);
            }
        }
        // printf("\nCHECKPOINT 2 FOR CODE: %s\n", tmp_code);

        token = strtok(NULL, "|");
        if (token == NULL)
        {
            printf("Invalid message received: empty identifier\n");
            printf("Full message: %s\n", buffer);
            exit(1);
        }
        else
        {
            strcpy(identifier, token);
            // printf("Identifier: %s\n", identifier);
        }
        // printf("\nCHECKPOINT 3 FOR CODE: %s\n", tmp_code);
    }

    else
    {
        // printf("\nCHECKPOINT 4 FOR CODE: %s\n", tmp_code);
    }
    // printf("\nCHECKPOINT 5 FOR CODE: %s\n", tmp_code);
    token = strtok(NULL, "|");
    while (token != NULL)
    {
        // printf("Ignoring extra token: %s\n", token);
        token = strtok(NULL, "|");
    }
    // printf("\nFINISHED RECEIVING %s FROM SERVER\n", tmp_code);
    strcpy(code, tmp_code);

    // Clear buffer
    memset(buffer, 0, BUF_SIZE);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s domain port\n", argv[0]);
        return 1;
    }

    char *domain = argv[1];
    int port = atoi(argv[2]);

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    // Connect to server
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, domain, &serv_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        return 1;
    }
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    // Send PLAY message
    char code[5];
    char name[BUF_SIZE];
    char role[2] = "";
    char identifier[BUF_SIZE];
    strcpy(name, "Player1");

    // Create unique identifier for player
    sprintf(identifier, "%d", sockfd);

    // Allocate memory for field4
    char *field4 = malloc(BUF_SIZE);
    if (field4 == NULL)
    {
        perror("malloc");
        return 1;
    }

    // Send PLAY message to server with identifier
    // send_message(sockfd, "PLAY", role, "", identifier);
    // printf("Sent PLAY message to server.\n");

    // Wait for WAIT message from server
    recv_message(sockfd, code, role, NULL, identifier);
    printf("Received message from server. Code: %s, Name: %s, Identifier: %s\n", code, name, identifier);

    if (strcmp(code, "WAIT") != 0)
    {
        printf("Unexpected response from server: %s\n", code);
        return 1;
    }
    printf("Received WAIT message from server. Waiting for opponent...\n");

    // Wait for opponent's move or make a move
    char board[10];
    while (1)
    {
        // Receive message from server
        // printf("\nIN THE CLIENT WHILE LOOP\n");
        char position[10];

        // first time will get BEGN code
        recv_message(sockfd, code, role, position, identifier);

        if (strcmp(code, "BEGN") == 0)
        {
            char move1[10];
            char act1[10];
            // Game has started, initialize board
            strcpy(board, ".........");
            printf("Game has started, you are %s\n", role);

            if (strcmp(role, "X") == 0)
            {
                printf("It's your turn, enter MOVE or DRAW or RSGN: ");
                scanf("%s", move1);
                if (strncmp(move1, "MOVE", 4) == 0)
                {
                    printf("Enter your move (row,column): ");
                    scanf("%s", position);
                    send_message(sockfd, "MOVE", role, position, identifier);
                }
                else if (strncmp(move1, "DRAW", 4) == 0)
                {
                    printf("Enter your DRAW action: S/A/R: ");
                    scanf("%s", act1);
                    if (strcmp(act1, "S") == 0)
                    {
                        //send_message(sockfd, "DRAW", "S", "", identifier);
                        printf("You have suggested a draw.\n");
                        send_message(sockfd, "RSGN", role, position, identifier);
                    }
                    else if (strcmp(act1, "A") == 0)
                    {
                        send_message(sockfd, "DRAW", "A", "", identifier);
                        printf("You have accepted the draw proposal.\n");
                    }
                    else if (strcmp(act1, "R") == 0)
                    {
                        send_message(sockfd, "DRAW", "R", "", identifier);
                        printf("You have rejected the draw proposal.\n");
                    }
                    else
                    {
                        printf("Invalid command.\n");
                    }
                }
                else if (strncmp(move1, "RSGN", 4) == 0)
                {
                    send_message(sockfd, "RSGN", role, position, identifier);
                    printf("You have forfeited the game.\n");
                    break;
                }
                else
                {
                    printf("Invalid command.\n");
                }
            }
        }

        else if (strcmp(code, "OVER") == 0)
        {
            // Game is over, determine whether the player won or lost
            if (strcmp(role, "W") == 0)
            {
                printf("Congratulations, you won!\n");
            }
            else if (strcmp(role, "D") == 0)
            {
                printf("DRAW!!!\n");
            }
            else{
                printf("You lost :(");
            }
            break;
        }

        else if (strcmp(code, "MOVD") == 0)
        {
            int row, col;
            char move[10];
            char act[10];

            // Update board with opponent's move
            row = position[0] - '0' - 1;
            col = position[2] - '0' - 1;
            board[row * 3 + col] = strcmp(role, "X") == 0 ? 'O' : 'X';

            /* printf("%s made a move at (%s), current board state:\n", role, position);
            printf("%c|%c|%c\n", board[0], board[1], board[2]);
            printf("%c|%c|%c\n", board[3], board[4], board[5]);
            printf("%c|%c|%c\n", board[6], board[7], board[8]);
             */

            if (strcmp(role, "O") == 0)
            {
                printf("It's your turn, enter MOVE or DRAW or RSGN: ");
                scanf("%s", move);
                if (strncmp(move, "MOVE", 4) == 0)
                {
                    printf("Enter your move (row,column): ");
                    scanf("%s", position);
                    send_message(sockfd, "MOVE", role, position, identifier);
                }
                else if (strncmp(move, "DRAW", 4) == 0)
                {
                    printf("Enter your DRAW action: S/A/R: ");
                    scanf("%s", act);
                    if (strcmp(act, "S") == 0)
                    {
                        send_message(sockfd, "DRAW", act, "", identifier);
                        printf("You have suggested a draw.\n");
                    }
                    else if (strcmp(act, "A") == 0)
                    {
                        send_message(sockfd, "DRAW", act, position, identifier);
                        printf("You have accepted the draw proposal.\n");
                    }
                    else if (strcmp(act, "R") == 0)
                    {
                        send_message(sockfd, "DRAW", act, position, identifier);
                        printf("You have rejected the draw proposal.\n");
                    }
                    else
                    {
                        printf("Invalid command.\n");
                    }
                }
                else if (strncmp(move, "RSGN", 4) == 0)
                {
                    send_message(sockfd, "RSGN", role, position, identifier);
                    printf("You have forfeited the game.\n");
                    break;
                }
                else
                {
                    printf("Invalid command.\n");
                }
            }
            else if (strcmp(role, "X") == 0)
            {
                printf("It's your turn, enter MOVE or DRAW or RSGN: ");
                scanf("%s", move);
                if (strncmp(move, "MOVE", 4) == 0)
                {
                    printf("Enter your move (row,column): ");
                    scanf("%s", position);
                    send_message(sockfd, "MOVE", role, position, identifier);
                }
                else if (strncmp(move, "DRAW", 4) == 0)
                {
                    printf("Enter your DRAW action: S/A/R: ");
                    scanf("%s", act);
                    if (strcmp(act, "S") == 0)
                    {
                        //send_message(sockfd, "DRAW", "S", "", identifier);
                        printf("You have suggested a draw.\n");
                        send_message(sockfd, "RSGN", role, position, identifier);
                    }
                    else if (strcmp(act, "A") == 0)
                    {
                        send_message(sockfd, "DRAW", "A", "", identifier);
                        printf("You have accepted the draw proposal.\n");
                    }
                    else if (strcmp(act, "R") == 0)
                    {
                        send_message(sockfd, "DRAW", "R", "", identifier);
                        printf("You have rejected the draw proposal.\n");
                    }
                    else
                    {
                        printf("Invalid command.\n");
                    }
                }
                else if (strncmp(move, "RSGN", 4) == 0)
                {
                    send_message(sockfd, "RSGN", role, position, identifier);
                    printf("You have forfeited the game.\n");
                    break;
                }
                else
                {
                    printf("Invalid command.\n");
                }
            }
        }
        else
        {
            // Invalid message
            printf("!Invalid message\n");
            send_message(sockfd, "INVL", "!Invalid message.", "", "");
        }
    }

    // Close socket
    close(sockfd);

    return 0;
}