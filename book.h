#include <stdlib.h>

enum {
    BLANK_SPACE = '0',
    WHITE_PAWN = '1',
    WHITE_ROOK = '2',
    WHITE_KNIGHT = '3',
    WHITE_BISHOP = '4',
    WHITE_QUEEN = '5',
    WHITE_KING = '6',
    WHITE_PAWN_EN_PASSANT = '7',
    WHITE_ROOK_NO_MOVE = '8',
    WHITE_KING_NO_MOVE = '9',
    BLACK_PAWN = 'A',
    BLACK_ROOK = 'B',
    BLACK_KNIGHT = 'C',
    BLACK_BISHOP = 'D',
    BLACK_QUEEN = 'E',
    BLACK_KING = 'F',
    BLACK_PAWN_EN_PASSANT = 'G',
    BLACK_ROOK_NO_MOVE = 'H',
    BLACK_KING_NO_MOVE = 'I',
};

typedef enum {
    CHESS_BLACK = -1,
    CHESS_WHITE = 1,
} chess_color_t;

/* MOVES_X format for lists */
enum {
    MOVES_STRING = 0,
    MOVES_FROM = 1,
    MOVES_TO = 2,
    MOVES_EXTRA_CHECKS = 3,
    MOVES_NUMBER_OF_FIELDS = 4,
};

/* return code for movePiece */
enum {
    MOVE_PIECE_ERROR = -1,
    MOVE_PIECE_SUCCESSFUL = 0,
    MOVE_PIECE_PAWN_PROMOTION_WHITE = 1,
    MOVE_PIECE_PAWN_PROMOTION_BLACK = 2,
    MOVE_PIECE_CASTLE_WHITE = 3,
    MOVE_PIECE_CASTLE_BLACK = 4,
};

char queens_gambit[66] = {'b',
                          'H', 'C', 'D', 'E', 'I', 'D', 'C', 'H',
                          'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '0', '0', '0', '7', '0', '0', '0', '0',
                          '0', '0', '0', '0', '0', '0', '0', '0',
                          '1', '1', '1', '0', '1', '1', '1', '1',
                          '8', '3', '4', '5', '9', '4', '3', '8'};