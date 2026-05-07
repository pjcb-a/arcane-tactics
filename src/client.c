#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"

// [NEW CODE - Typewriter function for animated reveal]
void typewriter(const char *text, int delay_ms) {
    if (text == NULL) return;
    for (int i = 0; text[i] != '\0'; i++) {
        printf("%c", text[i]);
        fflush(stdout);
        usleep(delay_ms * 1000);
    }
    printf("\n");
}

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

    // Receive initial welcome message
    n = recv(client_sock, &game, sizeof(game), 0);
    if(n < 0) die_with_error("Error: Server Disconnected...\n");
    typewriter(game.message, 30);

    /* [OLD DISCARDED CODE - Initial Dice Roll (Turn order is now round-by-round)]
    printf("\n--- DICE ROLL RESULT ---\n");
    printf("You (P2) rolled: %d\n", game.p2_roll);
    printf("Opponent (P1) rolled: %d\n", game.p1_roll); 
    printf("------------------------\n\n");
    */

    // 2. GAME PHASE -- Display of cards and start of round 
        while(1) {
                // Receive round start header (contains Arcane Tactics title)
                n = recv(client_sock, &game, sizeof(game), 0);
                if(n < 0){
                    die_with_error("Error: Server Disconnected...\n");
                    break;
                }

                // [NEW/ALTERED CODE - Unified UI Print]
                printf("%s", game.message);
                printf("Your HP: %d | Your Energy: %d\n", game.p2_status.hp, game.p2_status.energy);
                printf("Opponent HP: %d | Opponent Energy: %d\n", game.p1_status.hp, game.p1_status.energy);
                
                printf(" \n---- YOUR HAND ---- \n\n");
                for(int i = 0; i < game.p2_status.hand_count; i++) {
                    printf("[%d] %-15s (DMG:%d, UTIL:%d, COST:%d, PRIO:%d)\n", i + 1,
                    game.p2_status.hand[i].name,
                    game.p2_status.hand[i].damage,
                    game.p2_status.hand[i].utility,
                    game.p2_status.hand[i].cost,
                    game.p2_status.hand[i].priority);
                }

            // NEW: CONTINUOUS VALIDATION LOOP 
            int p2_valid = 0;
            while (!p2_valid) {
                // [NEW/ALTERED CODE - Generic prompt because order is unknown]
                printf("\n< Select card index (0 to Skip) >> ");

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

            // Receive combat resolution from server
            n = recv(client_sock, &game, sizeof(game), 0);
            if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
                break;
            }
            
            // [NEW/ALTERED CODE - Animated Typewriter Output]
            // Apply P2 perspective before displaying on client side
            char client_log[1024];
            apply_perspective(game.message, client_log, 0); // 0 = P2 = "You"
            typewriter(client_log, 20);

            // Receive updated game state with new cards
            n = recv(client_sock, &game, sizeof(game), 0);
            if(n < 0){
                die_with_error("Error: Server Disconnected...\n");
                break;
            }
        }

    close(client_sock);
    return 0;
}