/*
Created by Ryan Srichai 10.08.2026

Strategies:
- Random move
- Minimise opponent moves
- Heuristic

From extensive testing (2 games), it seems like pointHeuristic beats positionHeuristic (on black and white)
pointHeuristic also beat hybridHeuristic twice (on black and white)
*/

#define UNITYPE_LIST_IMPLEMENTATION
#include "chess.h"
#include <time.h>

#define ERROR_PRINT(fmt, ...) printf("ryan.exe ERROR: " fmt, ##__VA_ARGS__)

typedef enum {
    ENGINE_STRATEGY_RANDOM = 1,
    ENGINE_STRATEGY_MINIMISE_OPPONENT_MOVES = 2,
    ENGINE_STRATEGY_MAXIMISE_MOVES = 3,
    ENGINE_STRATEGY_NAIVE_HEURISTIC = 4,
    ENGINE_STRATEGY_HEURISTIC = 5,
    ENGINE_STRATEGY_HEURISTIC_HYBRID = 6,
} engine_strategy_t;

char dialog[][128] = {
    "Hey it's nice to meet you",
    "This world is imperfect.",
    "What",
    "If only I could wipe away\nthe impurities,",
    "Is anybody else listening\nto this",
    "and make it as beautiful\nas me!",
    "14 hours of gameplay later",
    "Lysanderoth! You were behind\nall this?",
    "Yes it was I. My machinations lay\nundetected for years for\nI am a master of deception",
};

/* heuristics */
double pointHeuristic(char *board, chess_color_t turn);
double positionHeuristic(char *board, chess_color_t turn);
double hybridHeuristic(char *board, chess_color_t turn);

/* engines - returns an index of moves, returns -1 if error */
int32_t engineStrategyRandom(char *board, chess_color_t turn, list_t *moves);
int32_t engineStrategyMinimiseOpponentMoves(char *board, chess_color_t turn, list_t *moves);
int32_t engineStrategyMaximiseMoves(char *board, chess_color_t turn, list_t *moves);
int32_t engineStrategyNaiveHeuristic(char *board, chess_color_t turn, list_t *moves, double (*heuristic)(char *, chess_color_t));
int32_t engineStrategyHeuristic(char *board, chess_color_t turn, list_t *moves, double (*heuristic)(char *, chess_color_t));
int32_t engineStrategyHeuristicHybrid(char *board, chess_color_t turn, list_t *moves, double (*heuristic)(char *, chess_color_t));

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
    self.strategy = ENGINE_STRATEGY_HEURISTIC;
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
    list_delete(self.moves, 0); // delete metadata
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
    list_delete(self.moves, 0); // delete input board
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    srand(currentTime.tv_nsec);
    int32_t status = 0;
    switch (self.strategy) {
        case ENGINE_STRATEGY_RANDOM:
        status = engineStrategyRandom(self.board, self.turn, self.moves);
        break;
        case ENGINE_STRATEGY_MINIMISE_OPPONENT_MOVES:
        status = engineStrategyRandom(self.board, self.turn, self.moves);
        break;
        case ENGINE_STRATEGY_MAXIMISE_MOVES:
        status = engineStrategyRandom(self.board, self.turn, self.moves);
        break;
        case ENGINE_STRATEGY_NAIVE_HEURISTIC:
        status = engineStrategyNaiveHeuristic(self.board, self.turn, self.moves, hybridHeuristic);
        break;
        case ENGINE_STRATEGY_HEURISTIC:
        status = engineStrategyHeuristic(self.board, self.turn, self.moves, hybridHeuristic);
        break;
        case ENGINE_STRATEGY_HEURISTIC_HYBRID:
        status = engineStrategyHeuristicHybrid(self.board, self.turn, self.moves, hybridHeuristic);
        break;
        default:
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
    fprintf(outputfp, dialog[randomInt(0, sizeof(dialog) / 128 - 1)]);
    fclose(outputfp);
    return 0;
}

/*
███████╗████████╗██████╗  █████╗ ████████╗███████╗ ██████╗ ██╗███████╗███████╗
██╔════╝╚══██╔══╝██╔══██╗██╔══██╗╚══██╔══╝██╔════╝██╔════╝ ██║██╔════╝██╔════╝
███████╗   ██║   ██████╔╝███████║   ██║   █████╗  ██║  ███╗██║█████╗  ███████╗
╚════██║   ██║   ██╔══██╗██╔══██║   ██║   ██╔══╝  ██║   ██║██║██╔══╝  ╚════██║
███████║   ██║   ██║  ██║██║  ██║   ██║   ███████╗╚██████╔╝██║███████╗███████║
╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝ ╚═════╝ ╚═╝╚══════╝╚══════╝
https://patorjk.com/software/taag/#p=display&f=ANSI%20Shadow
*/

int32_t engineStrategyRandom(char *board, chess_color_t turn, list_t *moves) {
    return randomInt(0, moves -> length - 1);
}

int32_t engineStrategyMinimiseOpponentMoves(char *board, chess_color_t turn, list_t *moves) {
    int32_t minMoves = 1000000000;
    list_t *opponentMoves = list_init();
    for (int32_t move = 0; move < moves -> length; move++) {
        list_t *possibleOpponentMoves = generateAllMoves(moves -> data[move].s + 1, !turn);
        list_append(opponentMoves, (unitype) (possibleOpponentMoves -> length / MOVES_NUMBER_OF_FIELDS), 'i');
        if ((possibleOpponentMoves -> length / MOVES_NUMBER_OF_FIELDS) < minMoves) {
            minMoves = (possibleOpponentMoves -> length / MOVES_NUMBER_OF_FIELDS);
        }
        list_free(possibleOpponentMoves);
    }
    /* determine all moves that result in the opponent having minMoves */
    list_t *moveIndex = list_init();
    for (int32_t move = 0; move < opponentMoves -> length; move++) {
        if (opponentMoves -> data[move].i == minMoves) {
            list_append(moveIndex, (unitype) move, 'i');
        }
    }
    list_free(opponentMoves);
    int32_t randomIndex = randomInt(0, moveIndex -> length - 1);
    int32_t pick = moveIndex -> data[randomIndex].i;
    list_free(moveIndex);
    return pick;
}

int32_t engineStrategyMaximiseMoves(char *board, chess_color_t turn, list_t *moves) {
    return 0;
}

int32_t getPiecePoints(char code) {
    switch (code) {
        case WHITE_PAWN:
        case WHITE_PAWN_EN_PASSANT:
        case BLACK_PAWN:
        case BLACK_PAWN_EN_PASSANT:
        return 1;
        case WHITE_ROOK:
        case WHITE_ROOK_NO_MOVE:
        case BLACK_ROOK:
        case BLACK_ROOK_NO_MOVE:
        return 5;
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
        case WHITE_BISHOP:
        case BLACK_BISHOP:
        return 3;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
        return 9;
        case WHITE_KING:
        case WHITE_KING_NO_MOVE:
        case BLACK_KING:
        case BLACK_KING_NO_MOVE:
        return 200;
        default:
        return 0;
    }
}

/* point-based heuristic */
double pointHeuristic(char *board, chess_color_t turn) {
    double whitePoints = 0;
    double blackPoints = 0;
    for (int32_t square = 0; square < 64; square++) {
        int32_t points = getPiecePoints(board[square]);
        if (getPieceColor(board[square]) == CHESS_WHITE) {
            whitePoints += points;
        } else {
            blackPoints += points;
        }
    }
    if (turn == CHESS_WHITE) {
        return whitePoints - blackPoints;
    }
    return blackPoints - whitePoints;
}

/* point + position-based heuristic */
double positionHeuristic(char *board, chess_color_t turn) {
    double whitePosition = 0;
    double blackPosition = 0;
    for (int32_t square = 0; square < 64; square++) {
        double multiplier = ((24.5 - ((square / 8 - 3.5) * (square / 8 - 3.5) + (square % 8 - 3.5) * (square % 8 - 3.5))) / 24.5) * 0.2 + 0.8; // euclidean distance from center of board (scaled from 0.8 to 1.0)
        int32_t points = getPiecePoints(board[square]);
        if (getPieceColor(board[square]) == CHESS_WHITE) {
            whitePosition += points * multiplier;
        } else {
            blackPosition += points * multiplier;
        }
    }
    if (turn == CHESS_WHITE) {
        return whitePosition - blackPosition;
    }
    return blackPosition - whitePosition;
}

/* hybrid heuristic based on number of total points on the board */
double hybridHeuristic(char *board, chess_color_t turn) {
    double cutoffValue = 462;
    double whitePoints = 0;
    double blackPoints = 0;
    double whitePosition = 0;
    double blackPosition = 0;
    double totalPoints = 0;
    for (int32_t square = 0; square < 64; square++) {
        double multiplier = ((24.5 - ((square / 8 - 3.5) * (square / 8 - 3.5) + (square % 8 - 3.5) * (square % 8 - 3.5))) / 24.5) * 0.2 + 0.8; // euclidean distance from center of board (scaled from 0.8 to 1.0)
        double points = getPiecePoints(board[square]);
        if (getPieceColor(board[square]) == CHESS_WHITE) {
            whitePoints += points;
            whitePosition += points * multiplier;
        } else {
            blackPoints += points;
            blackPosition += points * multiplier;
        }
        totalPoints += points;
    }
    if (totalPoints >= cutoffValue) {
        /* use point + position-based heuristic (early game) */
        if (turn == CHESS_WHITE) {
            return whitePosition - blackPosition;
        }
        return blackPosition - whitePosition;
    }
    /* use point-based heuristic (late game) */
    if (turn == CHESS_WHITE) {
        return whitePoints - blackPoints;
    }
    return blackPoints - whitePoints;
}

/* calculates best move based on highest point value for all possible moves */
int32_t engineStrategyNaiveHeuristic(char *board, chess_color_t turn, list_t *moves, double (*heuristic)(char *, chess_color_t)) {
    /* single move heuristic */
    list_t *points = list_init();
    double maxPoints = -1000000;
    for (int32_t move = 0; move < moves -> length; move++) {
        double value = heuristic(moves -> data[move].s + 1, turn);
        if (value > maxPoints) {
            maxPoints = value;
        }
        list_append(points, (unitype) value, 'd');
    }
    /* determine all moves that result in maxPoints */
    list_t *moveIndex = list_init();
    for (int32_t move = 0; move < points -> length; move++) {
        if (points -> data[move].d == maxPoints) {
            list_append(moveIndex, (unitype) move, 'i');
        }
    }
    list_free(points);
    int32_t randomIndex = randomInt(0, moveIndex -> length - 1);
    int32_t pick = moveIndex -> data[randomIndex].i;
    list_free(moveIndex);
    return pick;
}

/* calculates best move based on highest point value after two moves, assuming opponent makes a move according to engineStrategyNaiveHeuristic */
int32_t engineStrategyHeuristic(char *board, chess_color_t turn, list_t *moves, double (*heuristic)(char *, chess_color_t)) {
    int32_t lookAhead = 2; // cycles to look ahead (1 cycle is a move from each player)
    for (int32_t cycle = 0; cycle < lookAhead; cycle++) {
        
    }
    /* determine opponent's best move */
    list_t *opponentPoints = list_init();
    double minPoints = 100000;
    for (int32_t move = 0; move < moves -> length; move++) {
        list_t *possibleOpponentMoves = generateAllMoves(moves -> data[move].s + 1, !turn);
        double maxPoints = -1000000;
        for (int32_t opponentMove = 0; opponentMove < possibleOpponentMoves -> length; opponentMove += MOVES_NUMBER_OF_FIELDS) {
            double value = heuristic(possibleOpponentMoves -> data[opponentMove + MOVES_STRING].s + 1, !turn);
            if (value > maxPoints) {
                maxPoints = value;
            }
        }
        list_free(possibleOpponentMoves);
        list_append(opponentPoints, (unitype) maxPoints, 'd');
        if (opponentPoints -> data[move].d < minPoints) {
            minPoints = opponentPoints -> data[move].d;
        }
    }
    /* determine all moves that result in opponent minPoints */
    list_t *moveIndex = list_init();
    for (int32_t move = 0; move < opponentPoints -> length; move++) {
        if (opponentPoints -> data[move].d == minPoints) {
            list_append(moveIndex, (unitype) move, 'i');
        }
    }
    list_print(opponentPoints);
    list_free(opponentPoints);
    int32_t randomIndex = randomInt(0, moveIndex -> length - 1);
    int32_t pick = moveIndex -> data[randomIndex].i;
    list_free(moveIndex);
    return pick;
}

/* hybrid approach that also looks specifically for checkmate */
int32_t engineStrategyHeuristicHybrid(char *board, chess_color_t turn, list_t *moves, double (*heuristic)(char *, chess_color_t)) {
    return 0;
}