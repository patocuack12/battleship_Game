#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#define BUFFER_SIZE 1024 
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

char boardP1[10][10], boardP2[10][10]; 
int hit = 0; 
int winner = 0; 
int setAlarm = 0; 

typedef struct{
    char x;
    int y;
}coord;

typedef struct{
    int ship5;
    int ship4;
    int ship3;
    int ship2;
}shipsPerPlayer;

shipsPerPlayer shipsP1, shipsP2;

typedef struct{
    pthread_mutex_t mutexPlayer, mutexThread;
    int ready;
    coord coordinates;
    int currentPlayer;
} SharedData;

SharedData sharedData;

void initShipsPerPlayer(shipsPerPlayer *ships){
    ships->ship5 = 5;
    ships->ship4 = 4;
    ships->ship3 = 3;
    ships->ship2 = 2;
}

void addDrownedShip(char s, int player){
    switch(s){
        case '5':
            if(player == 1) shipsP1.ship5--;
            else shipsP2.ship5--;
            break;
        case '4':
            if(player == 1) shipsP1.ship4--;
            else shipsP2.ship4--;
            break;
        case '3':
            if(player == 1) shipsP1.ship3--;
            else shipsP2.ship3--;
            break;
        case '2':
            if(player == 1) shipsP1.ship2--;
            else shipsP2.ship2--;
            break;
    }
}

int allShipsDrowned(shipsPerPlayer ships){
    if(ships.ship5 == 0 && ships.ship4 == 0 && ships.ship3 == 0 && ships.ship2 == 0)
        return 1;
    return 0;
}

int gameEnded(){
    if(allShipsDrowned(shipsP1)){
        winner = 2;
        return 1;
    }
    else if(allShipsDrowned(shipsP2)){
        winner = 1;
        return 1;
    }

    return 0;
}


void printMatrix(char matrix[10][10]){
    
    printf("          0 1 2 3 4 5 6 7 8 9\n");
    for(int i = 0; i < 10; i++){
        
        printf("        %c ", i + 65);
        for(int j = 0; j < 10; j++)
            printf("%c ", matrix[i][j]);
        printf("\n");
    }
    printf("\n");
}


void printBothMatrices(char matrixP1[10][10], char matrixP2[10][10]){
    
    printf("\n    ==Your board==           ==Opponent board==\n\n");
    printf("  0 1 2 3 4 5 6 7 8 9        0 1 2 3 4 5 6 7 8 9\n");
    for(int i = 0; i < 10; i++){
        
        printf("%c ", i + 65);

        
        for(int j = 0; j < 10; j++){
            if(!(matrixP1[i][j] == '.' )){
                if(matrixP1[i][j] == 'X' )
                    printf(RED "%c ", matrixP1[i][j]);
                else if(matrixP1[i][j] == 'O')
                    printf(GREEN "%c ", matrixP1[i][j]);
                else
                    printf(CYAN "%c ", matrixP1[i][j]);

                printf(RESET);
            }
            else
                printf("%c ", matrixP1[i][j]);
        }
        printf("     ");
        printf("%c ", i + 65);

        
        for(int j = 0; j < 10; j++){
            if(!(matrixP2[i][j] == '.' )){
                if(matrixP2[i][j] == 'X' )
                    printf(RED "%c ", matrixP2[i][j]);
                else if(matrixP2[i][j] == 'O')
                    printf(GREEN "%c ", matrixP2[i][j]);
                else
                    printf(RESET ". ");

                printf(RESET);
            }
            else
                printf(". ");
        }
        printf("\n");
    }
}

void clearTerminal(){
    system("clear");
    fflush(stdout);
}


void initBoard(char matrix[10][10]){
    for(int i = 0; i < 10; i++)
        for(int j = 0; j < 10; j++)
            matrix[i][j] = '.';
}

void pressAnyKey(int signum){
    printf("\n\nPRESS ANY KEY TO CONTINUE...");
    getchar();
}

void showInstructions(){
    clearTerminal();
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    
    int fd = open("instructions.txt", 0);

   
    if(fd == -1){
        perror("Error opening the file");
        return;
    }

    
    while((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0){
        write(1, buffer, bytesRead);
    }

    
    if(bytesRead == -1){
        perror("Error reading the file");
        return;
    }

    close(fd);

    pause();
}

int toNumber(char letter){
   
    if(letter >= 97 && letter <= 122)
        letter -= 32;

    
    return letter - 65;
}

int validateCoordinates(char x, int y){
    
    if(toNumber(x) < 0 || toNumber(x) > 9 || y < 0 || y > 9){
        printf("Invalid coordinates, try again\n\n");
        return 0;
    }

    return 1;
}

void fillEachShip(int n, char matrix[10][10], int player){
    int orientation;
    coord coordinates;
    char tam = n + '0';

    
    printf("Select (1) to place the ship horizontally or (2) to place it vertically: ");
    
    
    do {
        scanf("%d", &orientation);
        
        while(getchar() != '\n');
        if(orientation != 1 && orientation != 2)
            printf("\nInvalid orientation, try again: ");
    } while(orientation != 1 && orientation != 2);
    
    
    printf("Enter the starting row (A-J): ");
    coordinates.x = getchar();
    printf("Enter the starting column (0-9): ");
    scanf("%d", &coordinates.y);
    
    while(getchar() != '\n');

    
    if(validateCoordinates(coordinates.x, coordinates.y)){
        
        if((orientation == 1 && (9 - coordinates.y + 1) < n ) ||
            (orientation == 2 && (9 - toNumber(coordinates.x) + 1) < n)){
            printf("The ship does not fit in that position, try again\n\n");
            fillEachShip(n, matrix, player);
            return;
        }
    }
    else{
        fillEachShip(n, matrix, player);
        return;
    }

    
    if(orientation == 1){
        
        for(int i = coordinates.y; i <= coordinates.y + n - 1; i++){
            if(matrix[toNumber(coordinates.x)][i] != '.'){
                printf("There is a overlap with other ship, try again\n\n");
                fillEachShip(n, matrix, player);
                return;
            }
        }

        
        for(int i = 0; i < n; i++){
            if(player == 1)
                boardP1[toNumber(coordinates.x)][coordinates.y + i] = tam;
            else
                boardP2[toNumber(coordinates.x)][coordinates.y + i] = tam;
            
        }
    }
    else{ 
        
        for(int i = toNumber(coordinates.x); i <= toNumber(coordinates.x) + n - 1; i++){
            if(matrix[i][coordinates.y] != '.'){
                printf("There is a overlap with other ship, try again\n");
                fillEachShip(n, matrix, player);
                return;
            }
        }

        
        for(int i = 0; i < n; i++){
            if(player == 1)
                boardP1[toNumber(coordinates.x) + i][coordinates.y] = tam;
            else
                boardP2[toNumber(coordinates.x) + i][coordinates.y] = tam;
            
        }
    }

   
    printf("\n");
    printMatrix(matrix);
    return;
}

void placeShips(char matrix[10][10], int player){
    clearTerminal();

    
    printf("\n  === PLAYER %d, PLACE YOUR SHIPS ===  \n\n", player);
    printMatrix(matrix);

    printf("[Ship of size 5]:\n\n");
    fillEachShip(5, matrix, player);

    printf("\n[Ship of size 4]:\n\n");
    fillEachShip(4, matrix, player);

    printf("\n[Ship of size 3]:\n\n");
    fillEachShip(3, matrix, player);

    printf("\n[Ship of size 2]:\n\n");
    fillEachShip(2, matrix, player);

    sleep(3);
    
    return;
}


void* playerInputThread(void* arg){

    
    while(1){
        if(gameEnded()){
            clearTerminal();
            if(winner == 1)
                execl("./gameOver", "gameOver", "1", NULL);
            else
                execl("./gameOver", "gameOver", "2" , NULL);
            exit(1);
        }

        if(sharedData.ready == 0){
            
            pthread_mutex_lock(&(sharedData.mutexThread));

            sleep(1);
            
            printf("\nEnter the row to attack (A-J): ");
            sharedData.coordinates.x = getchar();
            sleep(0.5);
            printf("Enter the column to attack (0-9): ");
            scanf("%d", &sharedData.coordinates.y);
            
            while(getchar() != '\n');
            printf("\n");

            
            if(validateCoordinates(sharedData.coordinates.x, sharedData.coordinates.y)){
                
                
                sharedData.ready = 1;
                
                pthread_mutex_unlock(&(sharedData.mutexThread));
            }
            else{
                continue;
                
                pthread_mutex_unlock(&(sharedData.mutexThread));
            }

        }
    }
    pthread_exit(NULL);
}


void* playerUpdateThread(void* arg){
    
    while(1){
        if(gameEnded()){
            clearTerminal();
            if(winner == 1)
                execl("./gameOver", "gameOver", "1", NULL);
            else
                execl("./gameOver", "gameOver", "2" , NULL);
            exit(1);
        }

        if(sharedData.ready == 1){
            
            pthread_mutex_lock(&(sharedData.mutexThread));

            
            if(sharedData.currentPlayer == 1){ 
                char objective = boardP2[toNumber(sharedData.coordinates.x)][sharedData.coordinates.y];

                if(objective != '.' && objective != 'X' && objective != 'O'){
                    addDrownedShip(objective, 2);
                    boardP2[toNumber(sharedData.coordinates.x)][sharedData.coordinates.y] = 'X';
                    hit = 1;
                }
                else{
                    printf("You missed\n");
                    boardP2[toNumber(sharedData.coordinates.x)][sharedData.coordinates.y] = 'O';
                    hit = 0;
                }
            }
            else if(sharedData.currentPlayer == 2){ 
                char objective = boardP1[toNumber(sharedData.coordinates.x)][sharedData.coordinates.y];

                if(objective != '.' && objective != 'X' && objective != 'O'){
                    addDrownedShip(objective, 1);
                    boardP1[toNumber(sharedData.coordinates.x)][sharedData.coordinates.y] = 'X';
                    hit = 1;
                }
                else{
                    printf("You missed\n");
                    boardP1[toNumber(sharedData.coordinates.x)][sharedData.coordinates.y] = 'O';
                    hit = 0;
                }
            }

            sharedData.ready = -1;
            pthread_mutex_unlock(&(sharedData.mutexThread));
        }

    }

    pthread_exit(NULL);
}

void processP1(){
    sleep(4);
    clearTerminal();
    printf("\n            ===== PLAYER 1 TURN =====\n");
    printBothMatrices(boardP1, boardP2);

    hit = 0;
    do{
        
        sharedData.ready = 0;

        if(gameEnded()){
            clearTerminal();
            if(winner == 1)
                execl("./gameOver", "gameOver", "1", NULL);
            else
                execl("./gameOver", "gameOver", "2" , NULL);
            exit(1);
        }

       
        while(sharedData.ready == 0 || sharedData.ready == 1){
            sleep(1);
        }

        if(hit == 1){
            clearTerminal();
            printf("You hit the other player!, you can play again\n");
            printBothMatrices(boardP1, boardP2);
            sleep(1);
        }
    }while(hit);

    
    sharedData.currentPlayer = 2;
}

void processP2(){
    sleep(4);
    clearTerminal();
    printf("\n            ===== PLAYER 2 TURN =====\n");
    printBothMatrices(boardP2, boardP1);

    hit = -1;
    do{
        
        sharedData.ready = 0;
        
        if(gameEnded()){
            clearTerminal();
            if(winner == 1)
                execl("./gameOver", "gameOver", "1", NULL);
            else
                execl("./gameOver", "gameOver", "2" , NULL);
            exit(1);
        }

        
        while(sharedData.ready == 0 || sharedData.ready == 1){
            sleep(1);
        }

        if(hit == 1){
            clearTerminal();
            printf("You hit the other player, you can play again\n\n");
            printBothMatrices(boardP2, boardP1);
            sleep(1);
        }
    }while(hit);

    
    sharedData.currentPlayer = 1;
}

int main(){
    
    
    signal(SIGALRM, pressAnyKey);
    alarm(5);
    showInstructions();

    int status;
    pid_t player1Process, player2Process;

    player1Process = fork();
    if(player1Process == 0){
        
        initBoard(boardP1);

        
        initShipsPerPlayer(&shipsP1);

       
        placeShips(boardP1, 1);
    }

    
    waitpid(player1Process, &status, 0);

    player2Process = fork();
    if(player2Process == 0){
       
        initBoard(boardP2);

        
        initShipsPerPlayer(&shipsP2);

        
        placeShips(boardP2, 2);
    }

    
    waitpid(player2Process, &status, 0);


    
    sharedData.ready = -1;
    sharedData.currentPlayer = 1;

    
    pthread_t threadInput, threadUpdate;

    
    pthread_create(&threadInput, NULL, playerInputThread, NULL);
    pthread_create(&threadUpdate, NULL, playerUpdateThread, NULL);
    

    while(1){
        if(gameEnded()){
            clearTerminal();
            if(winner == 1)
                execl("./gameOver", "gameOver", "1", NULL);
            else
                execl("./gameOver", "gameOver", "2" , NULL);
            exit(1);
        }

        if(sharedData.currentPlayer == 1){
            processP1();
        }
        else if (sharedData.currentPlayer == 2){
            processP2();
        }
    }

    
    pthread_join(threadInput, NULL);
    pthread_join(threadUpdate, NULL);


    return 0;
}