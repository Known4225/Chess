/*
Created by Ryan Srichai 10.08.2026

Strategies:
- Random move
- Minimise opponent moves
*/

#define UNITYPE_LIST_IMPLEMENTATION
#include "chess.h"
#include <time.h>

#define ERROR_PRINT(fmt, ...) printf("ryan.exe ERROR: " fmt, ##__VA_ARGS__)

typedef enum {
    ENGINE_STRATEGY_RANDOM = 1,
    ENGINE_STRATEGY_MINIMISE_OPPONENT_MOVES = 2,
    ENGINE_STRATEGY_MAXIMISE_MOVES = 3,
} engine_strategy_t;

/* returns an index of moves, returns -1 if error */
int32_t engineStrategyRandom(char *board, chess_color_t turn, list_t *moves);
int32_t engineStrategyMinimiseOpponentMoves(char *board, chess_color_t turn, list_t *moves);
int32_t engineStrategyMaximiseMoves(char *board, chess_color_t turn, list_t *moves);

typedef struct {
    /* strategy */
    engine_strategy_t strategy;
    /* chess */
    chess_color_t turn;
    char board[64];
    list_t *moves;
} ryan_engine_t;

ryan_engine_t self;

int randomInt(int lowerBound, int upperBound) { // random integer between lower and upper bound (inclusive)
    return (rand() % (upperBound - lowerBound + 1) + lowerBound);
}

double randomDouble(double lowerBound, double upperBound) { // random double between lower and upper bound
    return (rand() * (upperBound - lowerBound) / RAND_MAX + lowerBound); // probably works idk
}

int main(int argc, char *argv[]) {
    /* set strategy */
    self.strategy = ENGINE_STRATEGY_RANDOM;
    /* go */
    if (argc != 3) {
        ERROR_PRINT("Expected 3 arguments, got %d\n", argc);
        return -1;
    }
    FILE *inputfp = fopen(argv[1], "r");
    if (inputfp == NULL) {
        ERROR_PRINT("Could not open input file %s\n", argv[1]);
        return -1;
    }
    self.moves = list_init();
    char line[1024];
    while (fgets(line, 1024, inputfp) != NULL) {
        for (int32_t i = 0; i < 2; i++) {
            if (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r') {
                line[strlen(line) - 1] = '\0';
            }
        }
        list_append(self.moves, (unitype) line, 's');
    }
    fclose(inputfp);
    if (self.moves -> length < 2) {
        ERROR_PRINT("No moves available\n");
        return -1;
    }
    memcpy(self.board, self.moves -> data[0].s + 1, 64);
    if (self.moves -> data[0].s[0] == 'w') {
        self.turn = CHESS_WHITE;
    } else if (self.moves -> data[0].s[0] == 'b') {
        self.turn = CHESS_BLACK;
    } else {
        ERROR_PRINT("Unknown character '%c', expected 'w' or 'b'\n", self.moves -> data[0].s[0]);
        return -1;
    }
    list_delete(self.moves, 0);
    srand(time(NULL));
    int32_t status = 0;
    if (self.strategy == ENGINE_STRATEGY_RANDOM) {
        status = engineStrategyRandom(self.board, self.turn, self.moves);
    } else if (self.strategy == ENGINE_STRATEGY_MINIMISE_OPPONENT_MOVES) {
        status = engineStrategyMinimiseOpponentMoves(self.board, self.turn, self.moves);
    } else if (self.strategy == ENGINE_STRATEGY_MAXIMISE_MOVES) {
        status = engineStrategyMaximiseMoves(self.board, self.turn, self.moves);
    } else {
        ERROR_PRINT("Unknown strategy %d\n", self.strategy);
        return -1;
    }
    if (status < 0 || status >= self.moves -> length) {
        /* assume failure message has been printed in engine-specific function */
        return -1;
    }
    FILE *outputfp = fopen(argv[2], "wb");
    if (outputfp == NULL) {
        ERROR_PRINT("Could not open output file %s\n", argv[2]);
        return -1;
    }
    fwrite(self.moves -> data[status].s, 1, 65, outputfp);
    fwrite("\n", 1, 1, outputfp);
    fprintf(outputfp, "Hello World\n");
    fclose(outputfp);
    return 0;
}

int32_t engineStrategyRandom(char *board, chess_color_t turn, list_t *moves) {
    return randomInt(0, moves -> length - 1);
}

int32_t engineStrategyMinimiseOpponentMoves(char *board, chess_color_t turn, list_t *moves) {

}

int32_t engineStrategyMaximiseMoves(char *board, chess_color_t turn, list_t *moves) {

}