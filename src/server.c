#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "common.h"
#include <time.h>

// typewriter animation for server side
void typewriter(const char *text, int delay_ms) {
    if (text == NULL) return;
    for (int i = 0; text[i] != '\0'; i++) {
        printf("%c", text[i]);
        fflush(stdout); // Forces the letter to print immediately
        usleep(delay_ms * 1000);
    }
    printf("\n");
}


int main(int argc, char *argv[]){

    srand(time(NULL)); // Seed the random number generator for card drawing

    int server_sock, client_sock, port_no, client_size, n;
    struct sockaddr_in server_addr, client_addr;

    // Avoid buffer overflows 
    char buffer[32];
    int p1_choice, p2_choice;
    int round_num = 1;
    
    if (argc < 2) {
        printf( "Usage: %s port_no", argv[0]);
        exit(1);
    }

    // server initialization
    printf("Server starting ...\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
        die_with_error("Error: socket() Failed.");

    bzero((char *) &server_addr, sizeof(server_addr));
    port_no = atoi(argv[1]);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        die_with_error("Error: bind() Failed.");

    listen(server_sock, 5);
    printf("Server listening on port %d ...\n", port_no);

    client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock < 0)
        die_with_error("Error: accept() Failed.");

    printf("Client successfully connected ...\n");

    GameState game;
    memset(&game, 0, sizeof(GameState));

    // Initialize game state for both players (server and client)
    game.p1_status.hp = MAX_HP;
    game.p1_status.energy = START_ENERGY;
    game.p1_status.hand_count = 0;
    char my_hp[32], my_stat[64];

    game.p2_status.hp = MAX_HP;
    game.p2_status.energy = START_ENERGY;
    game.p2_status.hand_count = 0;
    char op_hp[32], op_stat[64];

    sprintf(game.message, "Welcome to Arcane Tactics! Match Initiated.");

    // Send initial game state
    send(client_sock, &game, sizeof(game), 0);

    typewriter(game.message, 30); // typewriter anim

    draw_card(&game.p1_status, START_HAND_SIZE);
    draw_card(&game.p2_status, START_HAND_SIZE);

    // 2. GAME PHASE -- Actual loop of the game
    while (game.p1_status.hp > 0 && game.p2_status.hp > 0) {
            game.p1_status.shield = 0; // Reset P1 shield for the new round
            game.p2_status.shield = 0; // Reset P2 shield for the new round

            // [NEW/ALTERED CODE - Unified Header for Server with scaling energy info]
            printf("\n========================================\n");
            printf("             ARCANE TACTICS             \n");
            printf("               ROUND %d                 \n", round_num);
            if (round_num > 5) printf("         MANA SURGE ACTIVE!        \n");
            printf("========================================\n");

            // [NEW/ALTERED CODE - Send Round Header to Client via game.message]
            memset(game.message, 0, sizeof(game.message));
            sprintf(game.message, "\n========================================\n"
                                  "             ARCANE TACTICS             \n"
                                  "               ROUND %d                 \n"
                                  "%s"
                                  "========================================\n", 
                                  round_num, (round_num > 5) ? "         MANA SURGE ACTIVE!        \n" : "");
            send(client_sock, &game, sizeof(game), 0);

            get_ui_elements(&game.p1_status, my_hp, my_stat);
            get_ui_elements(&game.p2_status, op_hp, op_stat);

            printf("\n\nYour HP:     %s %d/%d | Shield: %d | Status: %s\n", my_hp, game.p1_status.hp, MAX_HP, game.p1_status.shield, my_stat);
            printf("Opponent HP: %s %d/%d | Shield: %d | Status: %s\n", op_hp, game.p2_status.hp, MAX_HP, game.p2_status.shield, op_stat);
            printf("Your Energy: %d | Opponent Energy: %d\n", game.p1_status.energy, game.p2_status.energy);


            printf(" \n---- YOUR HAND ---- \n\n");
            int j = 1;
            for(int i = 0; i < game.p1_status.hand_count; i++) {
                printf("[%d] %-15s (DMG:%d, UTIL:%d, COST:%d, PRIO:%d)\n", j,
                    game.p1_status.hand[i].name,
                    game.p1_status.hand[i].damage,
                    game.p1_status.hand[i].utility,
                    game.p1_status.hand[i].cost,
                    game.p1_status.hand[i].priority);
                    j++;
            }

            // === NEW: CONTINUOUS AFFORDABILITY CHECK PHASE ===
            
            // P1 (Server) Input Validation Loop
            int p1_valid = 0;
            while (!p1_valid) {
                printf("\n< Select card index (0 to Skip) >>  ");
                fgets(buffer, sizeof(buffer), stdin);
                p1_choice = atoi(buffer) - 1;

                if (p1_choice == -1) {
                    p1_valid = 1; // Skip is always valid
                } else if (p1_choice >= 0 && p1_choice < game.p1_status.hand_count) {
                    if (game.p1_status.hand[p1_choice].cost <= game.p1_status.energy) {
                        p1_valid = 1;
                    } else {
                        printf("[!] Not enough energy! %s costs %d.\n", 
                            game.p1_status.hand[p1_choice].name, game.p1_status.hand[p1_choice].cost);
                    }
                } else {
                    printf("[!] Invalid choice. Try again.\n");
                }
            }

            // P2 (Client) Input Validation Loop
            int p2_valid = 0;
            while (!p2_valid) {
                printf("\nWaiting for opponent's move...\n");
                bzero(buffer, 32);
                n = recv(client_sock, buffer, sizeof(buffer), 0);
                if (n < 0) {
                    die_with_error("Error: Client Disconnected...\n");
                    break;
                }
                p2_choice = atoi(buffer) - 1;

                // Validate P2's energy server-side
                if (p2_choice == -1) {
                    p2_valid = 1;
                    send(client_sock, "OK", 3, 0); // Inform client move accepted
                } else if (p2_choice >= 0 && p2_choice < game.p2_status.hand_count) {
                    if (game.p2_status.hand[p2_choice].cost <= game.p2_status.energy) {
                        p2_valid = 1;
                        send(client_sock, "OK", 3, 0); 
                    } else {
                        send(client_sock, "RETRY", 6, 0); // Client must prompt again
                    }
                } else {
                    send(client_sock, "RETRY", 6, 0);
                }
            }
            usleep(10000);
            
            // [NEW/ALTERED CODE - Gambled Turn Order determined AFTER selection]
            game.turn_winner = dice_roll(&game.p1_roll, &game.p2_roll);

            // EXECUTION PHASE
            ActionQueue priority_queue;
            ActionQueue regular_queue;
            init_queue(&priority_queue, 10);
            init_queue(&regular_queue, 10);
            
            // CONVERT CHOICES TO ACTIONS WITH SKIP LOGIC
            Action p1_action, p2_action;
            p1_action.player_id = 1;
            p2_action.player_id = 2;

            if (p1_choice == -1) {
                Card skip = {"Skip", 0, 0, 0, 0};
                p1_action.card = skip;
            } else {
                
                p1_action.card = game.p1_status.hand[p1_choice];
                game.p1_status.energy -= p1_action.card.cost;
            }

            if (p2_choice == -1) {
                Card skip = {"Skip", 0, 0, 0, 0};
                p2_action.card = skip;
            } else {
                p2_action.card = game.p2_status.hand[p2_choice];
                game.p2_status.energy -= p2_action.card.cost;
            }
            
            // [NEW/ALTERED CODE - Enqueue logic using the NEW turn_winner result]
            if (game.turn_winner == 1) {
                if(p1_action.card.priority == 1) enqueue(&priority_queue, p1_action);
                else enqueue(&regular_queue, p1_action);

                if(p2_action.card.priority == 1) enqueue(&priority_queue, p2_action);
                else enqueue(&regular_queue, p2_action);
            } else {
                if(p2_action.card.priority == 1) enqueue(&priority_queue, p2_action);
                else enqueue(&regular_queue, p2_action);

                if(p1_action.card.priority == 1) enqueue(&priority_queue, p1_action);
                else enqueue(&regular_queue, p1_action);
            }
            
            // 3. CLOSURE PHASE
            // Perspective gamble winner
            const char *server_winner = (game.turn_winner == 1) ? "You" : "Opponent";
            const char *client_winner = (game.turn_winner == 2) ? "You" : "Opponent";

            char server_gamble[256];
            sprintf(server_gamble, "\n------- TURN GAMBLE -------\n"
                                "You rolled: %d, Opponent rolled: %d\n"
                                "Winner: %s goes first!\n"
                                "---------------------------\n",
                                game.p1_roll, game.p2_roll, server_winner);

            char client_gamble[256];
            sprintf(client_gamble, "\n------- TURN GAMBLE -------\n"
                                "You rolled: %d, Opponent rolled: %d\n"
                                "Winner: %s goes first!\n"
                                "---------------------------\n",
                                game.p2_roll, game.p1_roll, client_winner);

            char combat_log[1024];
            memset(combat_log, 0, sizeof(combat_log));

            for (int k = 0; k < 2; k++) {
                if(game.p1_status.hp <= 0 || game.p2_status.hp <= 0){
                    break;
                }

                Action current_move;
                if (!is_empty(&priority_queue)) {
                    current_move = dequeue(&priority_queue);
                } else if (!is_empty(&regular_queue)) {
                    current_move = dequeue(&regular_queue);
                } else {
                    break;
                }

                if (current_move.player_id == 1) {
                    execute_card(&game.p1_status, &game.p2_status, current_move.card, 1, combat_log);
                } else {
                    execute_card(&game.p2_status, &game.p1_status, current_move.card, 0, combat_log);
                }
            }

            // client message
            char client_full[1280];
            snprintf(client_full, sizeof(client_full), "%s%s", client_gamble, combat_log);
            strcpy(game.message, client_full);
            send(client_sock, &game, sizeof(game), 0);

            // server message
            char server_combat[1024];
            apply_perspective(combat_log, server_combat, 1);
            char server_full[1280];
            snprintf(server_full, sizeof(server_full), "%s%s", server_gamble, server_combat);
            typewriter(server_full, 20);

            usleep(500000); 

            // 4. RELAPSE PHASE
            if (p1_choice != -1) remove_card(&game.p1_status, p1_choice);
            if (p2_choice != -1) remove_card(&game.p2_status, p2_choice);
            
            draw_card(&game.p1_status, 1);
            draw_card(&game.p2_status, 1);
            
            // scaling energy regen:
            // round 1-5 = +1 energy, round 6 onwards = +2 energy
            if (round_num > 5){
                game.p1_status.energy += 2;
                game.p2_status.energy += 2;
            } else {
                game.p1_status.energy += 1;
                game.p2_status.energy += 1;
            }
            
            if(game.p1_status.energy > 10) game.p1_status.energy = 10;
            if(game.p2_status.energy > 10) game.p2_status.energy = 10;
            
            round_num++; // increment round after energy calculation
            
            send(client_sock, &game, sizeof(game), 0);
            }

    close(client_sock);
    close(server_sock);
    return 0; 
}