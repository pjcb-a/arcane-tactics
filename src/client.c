#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"


int main(int argc,  char *argv[]){
    
    int client_sock,  port_no, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[32];

    GameState game;

    if (argc < 3) {
        printf("Usage: %s hostname port_no", argv[0]);
        exit(1);
    }
    //client initialization
    printf("Client starting ...\n");
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
        die_with_error("Error: socket() Failed.");

    //server connection setup
    printf("Looking for host '%s'...\n", argv[1]);
    server = gethostbyname(argv[1]);
    if (server == NULL)
        die_with_error("Error: No such host.");
    printf("Host found!\n");

    port_no = atoi(argv[2]);
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_no);

    printf("Connecting to server at port %d...\n", port_no);
    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: connect() Failed.");

    printf("Connection successful!\n");





// Recieve game state of the dice roll from server
        n = recv(client_sock, &game, sizeof(game), 0);
        if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
        }

   // 1. PREPARATION PHASE -- Dice roll results from server
    printf("\n%s\n", game.message);
    printf("\n--- DICE ROLL RESULT ---\n");
    printf("You (P2) rolled: %d\n", game.p2_roll);
    printf("Opponent (P1) rolled: %d\n", game.p1_roll); 
    printf("------------------------\n\n");

    // turn_winner from server
    if (game.turn_winner == 2) {
        printf("You (P2) go first!\n");
    } else {
        printf("AIN'T NO WAAAYY, opponent (P1) goes first!\n");
    }

    // 2. GAME PHASE -- Display of cards and start of round 
        while(1) {
                n = recv(client_sock, &game, sizeof(game), 0);
                if(n < 0){
                    die_with_error("Error: Server Disconnected...\n");
                    break;
                }

                printf("\n------------------------------------\n");
                printf( "%s", game.message);
                printf("\n------------------------------------\n");

                printf("Your HP: %d,     Your Energy: %d\n", game.p2_status.hp, game.p2_status.energy);
                printf("Opponent HP: %d, Opponent Energy: %d\n", game.p1_status.hp, game.p1_status.energy);
                
                int j = 1;
                printf(" \n---- YOUR HAND ---- \n\n");
                for(int i = 0; i < game.p2_status.hand_count; i++) {
                    printf("[%d] %s (DMG:%d, UTIL:%d, COST:%d)\n", j,
                    game.p2_status.hand[i].name,
                    game.p2_status.hand[i].damage,
                    game.p2_status.hand[i].utility,
                    game.p2_status.hand[i].cost);
                    j++;
            }

            // NEW: CONTINUOUS VALIDATION LOOP 
            int p2_valid = 0;
            while (!p2_valid) {
                if(game.turn_winner == 2){
                    printf("\n< You(P2) go first. Select card index (0 to Skip) >> ");
                } else {
                    printf("\nOpponent (P1) goes first, Then enter next move (0 to Skip) >> ");
                }

                //Send updates to server 
                bzero(buffer, sizeof(buffer));
                fgets(buffer, sizeof(buffer), stdin);
                send(client_sock, buffer, sizeof(buffer), 0);

                // Receive validation from server
                char status[16];
                bzero(status, 16);
                n = recv(client_sock, status, sizeof(status), 0);
                
                if (n > 0 && strcmp(status, "OK") == 0) {
                    p2_valid = 1; // Server confirmed card is affordable
                } else {
                    printf("[!] Not enough energy or invalid choice! Try again.\n");
                }
            }
            // end new

            // Receive combat resolution from server
            n = recv(client_sock, &game, sizeof(game), 0);
            if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
                break;
            }
            printf("%s", game.message);

            // Receive updated game state wiht new cards
            n = recv(client_sock, &game, sizeof(game), 0);
            if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
                break;
            }
        }

    close(client_sock);
    return 0;
}