#include "turtle.h"

int32_t movePiece(char *board, int8_t positionFrom, int8_t positionTo);

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
    CHESS_PIECE_PAWN = 0,
    CHESS_PIECE_ROOK = 1,
    CHESS_PIECE_KNIGHT = 2,
    CHESS_PIECE_BISHOP = 3,
    CHESS_PIECE_QUEEN = 4,
    CHESS_PIECE_KING = 5,
} chess_piece_t;

typedef enum {
    CHESS_WHITE = 0,
    CHESS_BLACK = 1,
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

/* return code for checkBoardState */
enum {
    BOARD_STATE_NONE = 0,
    BOARD_STATE_CHECK = 1,
    BOARD_STATE_STALEMATE = 2,
    BOARD_STATE_CHECKMATE = 3,
};

void printBoard(char *board) {
    for (int32_t j = 0; j < 8; j++) {
        for (int32_t i = 0; i < 8; i++) {
            printf("%c", board[j * 8 + i]);
        }
        printf("\n");
    }
}

void printValid(list_t *valid) {
    printf("[");
    for (int32_t i = 0; i < valid -> length; i++) {
        printf("%d ", valid -> data[i].c);
    }
    printf("]\n");
}

chess_piece_t getPieceType(char code) {
    switch (code) {
        case WHITE_PAWN:
        case WHITE_PAWN_EN_PASSANT:
        case BLACK_PAWN:
        case BLACK_PAWN_EN_PASSANT:
        return CHESS_PIECE_PAWN;
        case WHITE_ROOK:
        case WHITE_ROOK_NO_MOVE:
        case BLACK_ROOK:
        case BLACK_ROOK_NO_MOVE:
        return CHESS_PIECE_ROOK;
        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
        return CHESS_PIECE_KNIGHT;
        case WHITE_BISHOP:
        case BLACK_BISHOP:
        return CHESS_PIECE_BISHOP;
        case WHITE_QUEEN:
        case BLACK_QUEEN:
        return CHESS_PIECE_QUEEN;
        case WHITE_KING:
        case WHITE_KING_NO_MOVE:
        case BLACK_KING:
        case BLACK_KING_NO_MOVE:
        return CHESS_PIECE_KING;
        default:
        return -1;
    }
}

chess_color_t getPieceColor(char code) {
    switch (code) {
        case WHITE_PAWN:
        case WHITE_ROOK:
        case WHITE_KNIGHT:
        case WHITE_BISHOP:
        case WHITE_QUEEN:
        case WHITE_KING:
        case WHITE_PAWN_EN_PASSANT:
        case WHITE_ROOK_NO_MOVE:
        case WHITE_KING_NO_MOVE:
        return CHESS_WHITE;
        case BLACK_PAWN:
        case BLACK_ROOK:
        case BLACK_KNIGHT:
        case BLACK_BISHOP:
        case BLACK_QUEEN:
        case BLACK_KING:
        case BLACK_PAWN_EN_PASSANT:
        case BLACK_ROOK_NO_MOVE:
        case BLACK_KING_NO_MOVE:
        return CHESS_BLACK;
        default:
        return -1;
    }
}

static inline int8_t up(int8_t position) {
    if (position == -1) {
        return -1;
    }
    if (position - 8 < 0) {
        return -1;
    }
    return position - 8;
}

static inline int8_t down(int8_t position) {
    if (position == -1) {
        return -1;
    }
    if (position + 8 >= 64) {
        return -1;
    }
    return position + 8;
}

static inline int8_t left(int8_t position) {
    if (position == -1) {
        return -1;
    }
    if (position % 8 == 0) {
        return -1;
    }
    return position - 1;
}

static inline int8_t right(int8_t position) {
    if (position == -1) {
        return -1;
    }
    if (position % 8 == 7) {
        return -1;
    }
    return position + 1;
}

static inline int8_t upLeft(int8_t position) {
    return up(left(position));
}

static inline int8_t upRight(int8_t position) {
    return up(right(position));
}

static inline int8_t downLeft(int8_t position) {
    return down(left(position));
}

static inline int8_t downRight(int8_t position) {
    return down(right(position));
}

/* returns a list of positions where this piece can move to. Naive generation - does not cull moves that would put the king in check */
list_t *generateNaiveMoves(char *board, int8_t position, int8_t turn) {
    /* initialisation */
    list_t *output = list_init();
    /* validation */
    if (position == -1) {
        return output;
    }
    chess_piece_t type = getPieceType(board[position]);
    chess_color_t color = getPieceColor(board[position]);
    if (type == -1 || color == -1) {
        return output;
    }
    if (color != turn) {
        return output;
    }
    /* computation */
    if (type == CHESS_PIECE_PAWN) {
        /* pawn */
        int8_t (*direction[3]) (int8_t position);
        int8_t pawnPowerRow;
        if (color == CHESS_WHITE) {
            int8_t (*directionCopy[3]) (int8_t position) = {
                up, upLeft, upRight,
            };
            memcpy(direction, directionCopy, sizeof(void *) * 3);
            pawnPowerRow = 6;
        } else {
            int8_t (*directionCopy[3]) (int8_t position) = {
                down, downLeft, downRight,
            };
            memcpy(direction, directionCopy, sizeof(void *) * 3);
            pawnPowerRow = 1;
        }
        /* go up */
        int8_t spaceEmpty = 0; // ensure that pawn cannot move twice if a piece is in the way
        int8_t wanderingPosition = direction[0](position);
        if (wanderingPosition != -1) {
            if (board[wanderingPosition] == BLANK_SPACE) {
                list_append(output, (unitype) wanderingPosition, 'c');
                spaceEmpty = 1;
            }
        }
        /* possible: go up again */
        if (spaceEmpty && position / 8 == pawnPowerRow) {
            wanderingPosition = direction[0](wanderingPosition);
            if (wanderingPosition != -1) {
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                }
            }
        }
        /* check for capture (and en passant) */
        wanderingPosition = direction[1](position);
        int8_t enpassantPosition = left(position);
        if (wanderingPosition != -1) {
            if ((board[wanderingPosition] != BLANK_SPACE && getPieceColor(board[wanderingPosition]) != color) || (board[wanderingPosition] == BLANK_SPACE && getPieceColor(board[enpassantPosition]) != color && (board[enpassantPosition] == WHITE_PAWN_EN_PASSANT || board[enpassantPosition] == BLACK_PAWN_EN_PASSANT))) {
                list_append(output, (unitype) wanderingPosition, 'c');
            }
        }
        wanderingPosition = direction[2](position);
        enpassantPosition = right(position);
        if (wanderingPosition != -1) {
            if ((board[wanderingPosition] != BLANK_SPACE && getPieceColor(board[wanderingPosition]) != color) || (board[wanderingPosition] == BLANK_SPACE && getPieceColor(board[enpassantPosition]) != color && (board[enpassantPosition] == WHITE_PAWN_EN_PASSANT || board[enpassantPosition] == BLACK_PAWN_EN_PASSANT))) {
                list_append(output, (unitype) wanderingPosition, 'c');
            }
        }
        /* Note: pawn promotion handled in movePiece */
    } else if (type == CHESS_PIECE_ROOK) {
        int8_t (*direction[4]) (int8_t position) = {
            up, right, down, left,
        };
        for (int32_t j = 0; j < 4; j++) {
            int8_t wanderingPosition = position;
            while (1) {
                wanderingPosition = direction[j](wanderingPosition);
                if (wanderingPosition == -1) {
                    break;
                }
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(output, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
            }
        }
    } else if (type == CHESS_PIECE_KNIGHT) {
        int8_t (*direction[12]) (int8_t position) = {
            up, upLeft, upRight, right, upRight, downRight, down, downRight, downLeft, left, downLeft, upLeft,
        };
        for (int32_t j = 0; j < 12; j += 3) {
            int8_t wanderingPosition = position;
            wanderingPosition = direction[j](wanderingPosition);
            int8_t checkPosition = direction[j + 1](wanderingPosition);
            if (checkPosition != -1 && (board[checkPosition] == BLANK_SPACE || getPieceColor(board[checkPosition]) != color)) {
                list_append(output, (unitype) checkPosition, 'c');
            }
            checkPosition = direction[j + 2](wanderingPosition);
            if (checkPosition != -1 && (board[checkPosition] == BLANK_SPACE || getPieceColor(board[checkPosition]) != color)) {
                list_append(output, (unitype) checkPosition, 'c');
            }
        }
    } else if (type == CHESS_PIECE_BISHOP) {
        int8_t (*direction[4]) (int8_t position) = {
            upRight, downRight, downLeft, upLeft,
        };
        for (int32_t j = 0; j < 4; j++) {
            int8_t wanderingPosition = position;
            while (1) {
                wanderingPosition = direction[j](wanderingPosition);
                if (wanderingPosition == -1) {
                    break;
                }
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(output, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
            }
        }
    } else if (type == CHESS_PIECE_QUEEN) {
        int8_t (*direction[8]) (int8_t position) = {
            up, upRight, right, downRight, down, downLeft, left, upLeft,
        };
        for (int32_t j = 0; j < 8; j++) {
            int8_t wanderingPosition = position;
            while (1) {
                wanderingPosition = direction[j](wanderingPosition);
                if (wanderingPosition == -1) {
                    break;
                }
                if (board[wanderingPosition] == BLANK_SPACE) {
                    list_append(output, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(output, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
            }
        }
    } else if (type == CHESS_PIECE_KING) {
        int8_t (*direction[8]) (int8_t position) = {
            up, upRight, right, downRight, down, downLeft, left, upLeft,
        };
        for (int32_t j = 0; j < 8; j++) {
            int8_t wanderingPosition = position;
            wanderingPosition = direction[j](wanderingPosition);
            if (wanderingPosition != -1 && (board[wanderingPosition] == BLANK_SPACE || getPieceColor(board[wanderingPosition]) != color)) {
                list_append(output, (unitype) wanderingPosition, 'c');
            }
        }
        /* castling */
        if (board[position] == WHITE_KING_NO_MOVE && board[63] == WHITE_ROOK_NO_MOVE && board[62] == BLANK_SPACE && board[61] == BLANK_SPACE) {
            /* white right castle */
            list_append(output, (unitype) 62, 'c');
        }
        if (board[position] == WHITE_KING_NO_MOVE && board[56] == WHITE_ROOK_NO_MOVE && board[57] == BLANK_SPACE && board[58] == BLANK_SPACE && board[59] == BLANK_SPACE) {
            /* white left castle */
            list_append(output, (unitype) 57, 'c');
        }
        if (board[position] == BLACK_KING_NO_MOVE && board[7] == BLACK_ROOK_NO_MOVE && board[6] == BLANK_SPACE && board[5] == BLANK_SPACE) {
            /* black right castle */
            list_append(output, (unitype) 6, 'c');
        }
        if (board[position] == BLACK_KING_NO_MOVE && board[0] == BLACK_ROOK_NO_MOVE && board[1] == BLANK_SPACE && board[2] == BLANK_SPACE && board[3] == BLANK_SPACE) {
            /* black left castle */
            list_append(output, (unitype) 1, 'c');
        }
    }
    return output;
}

/* returns a list of positions where this piece can move to */
list_t *generateLegalMoves(char *board, int8_t position, int8_t turn) {
    /* initialisation */
    list_t *output = list_init();
    /* computation */
    list_t *naive = generateNaiveMoves(board, position, turn);
    list_t *moves = list_init();
    char boardCopy[66];
    boardCopy[65] = '\0';
    for (int32_t i = 0; i < naive -> length; i++) {
        if (turn == CHESS_WHITE) {
            boardCopy[0] = 'b';
        } else {
            boardCopy[0] = 'w';
        }
        memcpy(boardCopy + 1, board, 64);
        int32_t move = movePiece(boardCopy + 1, position, naive -> data[i].c);
        /* check results */
        if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
            char promotionOptions[4] = {WHITE_QUEEN, WHITE_KNIGHT, WHITE_ROOK, WHITE_BISHOP};
            for (int32_t j = 0; j < 4; j++) {
                boardCopy[naive -> data[i].c + 1] = promotionOptions[j];
                list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
            }
        } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
            char promotionOptions[4] = {BLACK_QUEEN, BLACK_KNIGHT, BLACK_ROOK, BLACK_BISHOP};
            for (int32_t j = 0; j < 4; j++) {
                boardCopy[naive -> data[i].c + 1] = promotionOptions[j];
                list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
            }
        } else if (move == MOVE_PIECE_CASTLE_WHITE) {
            list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
            list_append(moves, (unitype) position, 'c'); // MOVES_FROM
            list_append(moves, naive -> data[i], 'c'); // MOVES_TO
            list_t *extraChecks = list_init();
            if (naive -> data[i].c == 62) {
                /* white right castle */
                boardCopy[61 + 1] = BLANK_SPACE; // revert rook
                boardCopy[63 + 1] = WHITE_ROOK; // revert rook
                boardCopy[62 + 1] = BLANK_SPACE; // revert king
                boardCopy[60 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[60 + 1] = BLANK_SPACE; // revert king
                boardCopy[61 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            } else {
                /* white left castle */
                boardCopy[58 + 1] = BLANK_SPACE; // revert rook
                boardCopy[56 + 1] = WHITE_ROOK; // revert rook
                boardCopy[57 + 1] = BLANK_SPACE; // revert king
                boardCopy[60 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[60 + 1] = BLANK_SPACE; // revert king
                boardCopy[59 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[59 + 1] = BLANK_SPACE; // revert king
                boardCopy[58 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            }
            list_append(moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
        } else if (move == MOVE_PIECE_CASTLE_BLACK) {
            list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
            list_append(moves, (unitype) position, 'c'); // MOVES_FROM
            list_append(moves, naive -> data[i], 'c'); // MOVES_TO
            list_t *extraChecks = list_init();
            if (naive -> data[i].c == 6) {
                /* black right castle */
                boardCopy[5 + 1] = BLANK_SPACE; // revert rook
                boardCopy[7 + 1] = BLACK_ROOK; // revert rook
                boardCopy[6 + 1] = BLANK_SPACE; // revert king
                boardCopy[4 + 1] = BLACK_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[4 + 1] = BLANK_SPACE; // revert king
                boardCopy[5 + 1] = BLACK_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            } else {
                /* black left castle */
                boardCopy[2 + 1] = BLANK_SPACE; // revert rook
                boardCopy[0 + 1] = BLACK_ROOK; // revert rook
                boardCopy[1 + 1] = BLANK_SPACE; // revert king
                boardCopy[4 + 1] = BLACK_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[4 + 1] = BLANK_SPACE; // revert king
                boardCopy[3 + 1] = BLACK_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[3 + 1] = BLANK_SPACE; // revert king
                boardCopy[2 + 1] = BLACK_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            }
            list_append(moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
        } else if (move == MOVE_PIECE_SUCCESSFUL) {
            list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
            list_append(moves, (unitype) position, 'c'); // MOVES_FROM
            list_append(moves, naive -> data[i], 'c'); // MOVES_TO
            list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
        } else {
            printf("ERROR: movePiece returned MOVE_PIECE_ERROR\n");
        }
    }
    list_free(naive);
    /* second pass - validate legal moves are legal (don't put king in check, etc) */
    for (int32_t movesIndex = 0; movesIndex < moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        for (int32_t positionIndex = 0; positionIndex < 64; positionIndex++) {
            if (moves -> data[movesIndex + MOVES_STRING].s[positionIndex + 1] == BLANK_SPACE) {
                continue;
            }
            list_t *simulated = generateNaiveMoves(moves -> data[movesIndex + MOVES_STRING].s + 1, positionIndex, !turn);
            for (int32_t i = 0; i < simulated -> length; i++) {
                char capturedPiece = moves -> data[movesIndex + MOVES_STRING].s[simulated -> data[i].c + 1];
                if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                    /* this moves captures the king, invalidate the legal move */
                    for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                        list_delete(moves, movesIndex);
                    }
                    movesIndex -= MOVES_NUMBER_OF_FIELDS;
                    list_free(simulated);
                    goto SECOND_PASS_NEXT;
                }
            }
            list_free(simulated);
        }
        /* extra moves to validate - castling cannot have the king "walk" through check */
        if (moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r != NULL) {
            for (int32_t extra = 0; extra < moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> length; extra++) {
                for (int32_t positionIndex = 0; positionIndex < 64; positionIndex++) {
                    if (moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[positionIndex + 1] == BLANK_SPACE) {
                        continue;
                    }
                    list_t *simulated = generateNaiveMoves(moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s + 1, positionIndex, !turn);
                    for (int32_t i = 0; i < simulated -> length; i++) {
                        char capturedPiece = moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[simulated -> data[i].c + 1];
                        if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                            /* this moves captures the king, invalidate the legal move */
                            for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                                list_delete(moves, movesIndex);
                            }
                            movesIndex -= MOVES_NUMBER_OF_FIELDS;
                            list_free(simulated);
                            goto SECOND_PASS_NEXT;
                        }
                    }
                    list_free(simulated);
                }
            }
        }
        SECOND_PASS_NEXT:;
    }
    for (int32_t movesIndex = 0; movesIndex < moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        list_append(output, moves -> data[movesIndex + MOVES_TO], 'c');
    }
    list_free(moves);
    return output;
}

/* simulate a move - returns one of MOVE_PIECE_X */
int32_t movePiece(char *board, int8_t positionFrom, int8_t positionTo) {
    if (positionFrom < 0 || positionTo < 0) {
        return MOVE_PIECE_ERROR;
    }
    char piece = board[positionFrom];
    /* check if piece is pawn (with or without en passant), rook (that hasn't moved), or king (that hasn't moved) */
    if (piece == WHITE_PAWN && positionFrom / 8 == 6 && positionTo / 8 == 4) {
        /* change normal pawn to en passant pawn */
        piece = WHITE_PAWN_EN_PASSANT;
    } else if (piece == WHITE_PAWN_EN_PASSANT) {
        /* revert en passant to normal pawn */
        piece = WHITE_PAWN;
    } else if (piece == WHITE_ROOK_NO_MOVE) {
        /* revert rook to has moved */
        piece = WHITE_ROOK;
    } else if (piece == WHITE_KING_NO_MOVE) {
        /* revert king to has moved */
        piece = WHITE_KING;
    } else if (piece == BLACK_PAWN && positionFrom / 8 == 1 && positionTo / 8 == 3) {
        /* change normal pawn to en passant pawn */
        piece = BLACK_PAWN_EN_PASSANT;
    } else if (piece == BLACK_PAWN_EN_PASSANT) {
        /* revert en passant to normal pawn */
        piece = BLACK_PAWN;
    } else if (piece == BLACK_ROOK_NO_MOVE) {
        /* revert rook to has moved */
        piece = BLACK_ROOK;
    } else if (piece == BLACK_KING_NO_MOVE) {
        /* revert king to has moved */
        piece = BLACK_KING;
    }
    /* check: pawn promotion */
    if (piece == WHITE_PAWN && positionTo / 8 == 0) {
        board[positionFrom] = BLANK_SPACE;
        board[positionTo] = piece;
        return MOVE_PIECE_PAWN_PROMOTION_WHITE;
    } else if (piece == BLACK_PAWN && positionTo / 8 == 7) {
        board[positionFrom] = BLANK_SPACE;
        board[positionTo] = piece;
        return MOVE_PIECE_PAWN_PROMOTION_BLACK;
    } else {
        /* simulate piece moving */
        board[positionFrom] = BLANK_SPACE;
        board[positionTo] = piece;
        /* check en passant */
        if (piece == WHITE_PAWN) {
            int8_t enpassant = down(positionTo);
            if (enpassant != -1 && board[enpassant] == BLACK_PAWN_EN_PASSANT) {
                board[enpassant] = BLANK_SPACE;
            }
        } else if (piece == BLACK_ROOK) {
            int8_t enpassant = up(positionTo);
            if (enpassant != -1 && board[enpassant] == WHITE_PAWN_EN_PASSANT) {
                board[enpassant] = BLANK_SPACE;
            }
        }
        /* check castle */
        if (piece == WHITE_KING && positionFrom == 60 && positionTo == 62) {
            /* white right castle */
            board[63] = BLANK_SPACE;
            board[61] = WHITE_ROOK;
            return MOVE_PIECE_CASTLE_WHITE;
        } else if (piece == WHITE_KING && positionFrom == 60 && positionTo == 57) {
            /* white left castle */
            board[56] = BLANK_SPACE;
            board[58] = WHITE_ROOK;
            return MOVE_PIECE_CASTLE_WHITE;
        } else if (piece == BLACK_KING && positionFrom == 4 && positionTo == 6) {
            /* black right castle */
            board[7] = BLANK_SPACE;
            board[5] = BLACK_ROOK;
            return MOVE_PIECE_CASTLE_BLACK;
        } else if (piece == BLACK_KING && positionFrom == 4 && positionTo == 1) {
            /* black left castle */
            board[0] = BLANK_SPACE;
            board[2] = BLACK_ROOK;
            return MOVE_PIECE_CASTLE_BLACK;
        }
    }
    return MOVE_PIECE_SUCCESSFUL;
}

/* generate all moves and augmented board positions, returns a list in the MOVES_X format */
list_t *generateAllMoves(char *board, int8_t turn) {
    /* initialisation */
    list_t *moves = list_init();
    /* computation */
    for (int32_t position = 0; position < 64; position++) {
        if (board[position] == BLANK_SPACE) {
            continue;
        }
        list_t *naive = generateNaiveMoves(board, position, turn);
        char boardCopy[66];
        boardCopy[65] = '\0';
        for (int32_t i = 0; i < naive -> length; i++) {
            if (turn == CHESS_WHITE) {
                boardCopy[0] = 'b';
            } else {
                boardCopy[0] = 'w';
            }
            memcpy(boardCopy + 1, board, 64);
            int32_t move = movePiece(boardCopy + 1, position, naive -> data[i].c);
            /* check results */
            if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
                char promotionOptions[4] = {WHITE_QUEEN, WHITE_KNIGHT, WHITE_ROOK, WHITE_BISHOP};
                for (int32_t j = 0; j < 4; j++) {
                    boardCopy[naive -> data[i].c + 1] = promotionOptions[j];
                    list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                    list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                    list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                    list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
                }
            } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                char promotionOptions[4] = {BLACK_QUEEN, BLACK_KNIGHT, BLACK_ROOK, BLACK_BISHOP};
                for (int32_t j = 0; j < 4; j++) {
                    boardCopy[naive -> data[i].c + 1] = promotionOptions[j];
                    list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                    list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                    list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                    list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
                }
            } else if (move == MOVE_PIECE_CASTLE_WHITE) {
                list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                list_t *extraChecks = list_init();
                if (naive -> data[i].c == 62) {
                    /* white right castle */
                    boardCopy[61 + 1] = BLANK_SPACE; // revert rook
                    boardCopy[63 + 1] = WHITE_ROOK; // revert rook
                    boardCopy[62 + 1] = BLANK_SPACE; // revert king
                    boardCopy[60 + 1] = WHITE_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                    boardCopy[60 + 1] = BLANK_SPACE; // revert king
                    boardCopy[61 + 1] = WHITE_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                } else {
                    /* white left castle */
                    boardCopy[58 + 1] = BLANK_SPACE; // revert rook
                    boardCopy[56 + 1] = WHITE_ROOK; // revert rook
                    boardCopy[57 + 1] = BLANK_SPACE; // revert king
                    boardCopy[60 + 1] = WHITE_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                    boardCopy[60 + 1] = BLANK_SPACE; // revert king
                    boardCopy[59 + 1] = WHITE_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                    boardCopy[59 + 1] = BLANK_SPACE; // revert king
                    boardCopy[58 + 1] = WHITE_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                }
                list_append(moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
            } else if (move == MOVE_PIECE_CASTLE_BLACK) {
                list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                list_t *extraChecks = list_init();
                if (naive -> data[i].c == 6) {
                    /* black right castle */
                    boardCopy[5 + 1] = BLANK_SPACE; // revert rook
                    boardCopy[7 + 1] = BLACK_ROOK; // revert rook
                    boardCopy[6 + 1] = BLANK_SPACE; // revert king
                    boardCopy[4 + 1] = BLACK_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                    boardCopy[4 + 1] = BLANK_SPACE; // revert king
                    boardCopy[5 + 1] = BLACK_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                } else {
                    /* black left castle */
                    boardCopy[2 + 1] = BLANK_SPACE; // revert rook
                    boardCopy[0 + 1] = BLACK_ROOK; // revert rook
                    boardCopy[1 + 1] = BLANK_SPACE; // revert king
                    boardCopy[4 + 1] = BLACK_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                    boardCopy[4 + 1] = BLANK_SPACE; // revert king
                    boardCopy[3 + 1] = BLACK_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                    boardCopy[3 + 1] = BLANK_SPACE; // revert king
                    boardCopy[2 + 1] = BLACK_KING; // king walk
                    list_append(extraChecks, (unitype) boardCopy, 's');
                }
                list_append(moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
            } else if (move == MOVE_PIECE_SUCCESSFUL) {
                list_append(moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(moves, naive -> data[i], 'c'); // MOVES_TO
                list_append(moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
            } else {
                printf("ERROR: movePiece returned MOVE_PIECE_ERROR\n");
            }
        }
        list_free(naive);
    }
    /* second pass - validate legal moves are legal (don't put king in check, etc) */
    for (int32_t movesIndex = 0; movesIndex < moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        for (int32_t position = 0; position < 64; position++) {
            if (moves -> data[movesIndex + MOVES_STRING].s[position + 1] == BLANK_SPACE) {
                continue;
            }
            list_t *simulated = generateNaiveMoves(moves -> data[movesIndex + MOVES_STRING].s + 1, position, !turn);
            for (int32_t i = 0; i < simulated -> length; i++) {
                char capturedPiece = moves -> data[movesIndex + MOVES_STRING].s[simulated -> data[i].c + 1];
                if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                    /* this moves captures the king, invalidate the legal move */
                    for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                        list_delete(moves, movesIndex);
                    }
                    movesIndex -= MOVES_NUMBER_OF_FIELDS;
                    list_free(simulated);
                    goto SECOND_PASS_NEXT;
                }
            }
            list_free(simulated);
        }
        /* extra moves to validate - castling cannot have the king "walk" through check */
        if (moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r != NULL) {
            for (int32_t extra = 0; extra < moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> length; extra++) {
                for (int32_t position = 0; position < 64; position++) {
                    if (moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[position + 1] == BLANK_SPACE) {
                        continue;
                    }
                    list_t *simulated = generateNaiveMoves(moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s + 1, position, !turn);
                    for (int32_t i = 0; i < simulated -> length; i++) {
                        char capturedPiece = moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[simulated -> data[i].c + 1];
                        if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                            /* this moves captures the king, invalidate the legal move */
                            for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                                list_delete(moves, movesIndex);
                            }
                            movesIndex -= MOVES_NUMBER_OF_FIELDS;
                            list_free(simulated);
                            goto SECOND_PASS_NEXT;
                        }
                    }
                    list_free(simulated);
                }
            }
        }
        SECOND_PASS_NEXT:;
    }
    return moves;
}

/* check for checkmate, stalemate, check, or none */
int32_t checkBoardState(char *board, int8_t turn, int32_t movesSinceCapture) {
    /* check for check */
    int8_t check = 0;
    for (int32_t position = 0; position < 64; position++) {
        if (board[position] == BLANK_SPACE) {
            continue;
        }
        list_t *naive = generateNaiveMoves(board, position, !turn);
        for (int32_t i = 0; i < naive -> length; i++) {
            char capturedPiece = board[naive -> data[i].i];
            if ((turn == CHESS_WHITE && (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE)) || (turn == CHESS_BLACK && (capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE))) {
                check = 1;
                break;
            }
        }
    }
    /* check for stalemate */
    int8_t stalemate = 0;
    list_t *moves = generateAllMoves(board, turn);
    if (moves -> length == 0) {
        stalemate = 1;
    }
    list_free(moves);
    if (stalemate && check) {
        return BOARD_STATE_CHECKMATE;
    } else if (stalemate) {
        return BOARD_STATE_STALEMATE;
    } else if (check) {
        return BOARD_STATE_CHECK;
    } else if (movesSinceCapture >= 50) {
        return BOARD_STATE_STALEMATE;
    }
    return BOARD_STATE_NONE;
}