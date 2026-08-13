/*
Created by Ryan Srichai 02.08.26

Piece reference:
0 - blank space                                (0x30)
1 - white pawn                                 (0x31)
2 - white rook                                 (0x32)
3 - white knight                               (0x33)
4 - white bishop                               (0x34)
5 - white queen                                (0x35)
6 - white king                                 (0x36)
7 - white pawn that has just moved two squares (0x37)
8 - white rook that hasn't moved               (0x38)
9 - white king that hasn't moved               (0x39)
A - black pawn                                 (0x41)
B - black rook                                 (0x42)
C - black knight                               (0x43)
D - black bishop                               (0x44)
E - black queen                                (0x45)
F - black king                                 (0x46)
G - black pawn that has just moved two squares (0x47)
H - black rook that hasn't moved               (0x48)
I - black king that hasn't moved               (0x49)

TODO:
- Implement special stalemate conditions:
  - No capture in 50 moves
  - Threefold repetition
*/

#include "turtle.h"
#include <time.h>

#define MUTEX_ACQUIRE(mutex) while (mutex) {asm("nop");} mutex = 1
#define MUTEX_RELEASE(mutex) mutex = 0

int32_t movePiece(char *board, int8_t positionFrom, int8_t positionTo);
int32_t engineMove(char *engineName);

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

enum {
    CHESS_THEME_DEFAULT = 0,
    CHESS_THEME_CHESS_COM = 1,
};

enum {
    CHESS_COLOR_WHITE_SQUARE = 0,
    CHESS_COLOR_BLACK_SQUARE = 1,
    CHESS_COLOR_WHITE_SQUARE_HIGHLIGHT = 2,
    CHESS_COLOR_BLACK_SQUARE_HIGHLIGHT = 3,
    CHESS_COLOR_WHITE_SQUARE_HIGHLIGHT_BOX = 4,
    CHESS_COLOR_BLACK_SQUARE_HIGHLIGHT_BOX = 5,
    CHESS_COLOR_SQUARE_DOT = 6,
    CHESS_COLOR_PAWN_PROMOTION = 7,
    CHESS_COLOR_NUMBER = 8,
};

enum {
    KEYS_LMB = 0,
    KEYS_SPACE = 1,
};

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

uint8_t colors[] = {
    238, 238, 238, 0,   // white square
    113, 134, 184, 0,   // black square
    245, 246, 130, 0,   // highlighted white square
    185, 202, 67, 0,    // highlighted black square
    252, 252, 211, 0,   // box highlight white square
    206, 218, 195, 0,   // box highlight black square
    60, 60, 60, 200,    // white square dot
    255, 255, 255, 0,   // pawn promotion

    234, 237, 209, 0,   // white square
    118, 149, 86, 0,    // black square
    245, 246, 130, 0,   // highlighted white square
    185, 202, 67, 0,    // highlighted black square
    252, 252, 211, 0,   // box highlight white square
    206, 218, 195, 0,   // box highlight black square
    60, 60, 60, 200,    // white square dot
    255, 255, 255, 0,   // pawn promotion
};

typedef struct {
    int8_t theme;
    int8_t keys[8];

    /* chess */
    chess_color_t turn;
    char board[64];
    turtle_texture_t pieces[12];
    int8_t mouseSquare; // square that the mouse is hovering over
    int8_t mousePiece; // index 0-63 of piece of the board that the mouse is holding
    int8_t highlightedSquare[3]; // selected with color CHESS_COLOR_X_SQUARE_HIGHLIGHT (0 is editing piece, 1 and 2 are last move)
    int8_t highlightedSquareBox; // selected with color CHESS_COLOR_X_SQUARE_HIGHLIGHT_BOX
    int8_t highlightUnselect;
    list_t *valid; // list of squares marked with a dot (for valid)
    int8_t pawnPromotionWhite;
    int8_t pawnPromotionBlack;
    int8_t pawnPromotionIndex;
    volatile int8_t boardMutex; // data access on self.board
    volatile int8_t validMutex; // data access on self.valid

    /* board */
    double boardX;
    double boardY;
    double boardSize;

    /* engines */
    char inputFilename[4096];
    char outputFilename[4096];
    tt_button_t *mohamed;
    tt_button_t *ryan;
} chess_t;

chess_t self;

void init() {
    self.theme = CHESS_THEME_CHESS_COM;

    /* chess */
    self.turn = CHESS_WHITE;
    memcpy(self.board, "HCDEIDCHAAAAAAAA000000000000000000000000000000001111111183459438", 64);
    self.mouseSquare = -1;
    self.mousePiece = -1;
    self.highlightedSquare[0] = -1;
    self.highlightedSquare[1] = -1;
    self.highlightedSquare[2] = -1;
    self.highlightedSquareBox = -1;
    self.highlightUnselect = 0;
    self.pawnPromotionWhite = -1;
    self.pawnPromotionBlack = -1;
    self.pawnPromotionIndex = -1;
    self.valid = list_init();
    self.boardMutex = 0;
    self.validMutex = 0;

    /* textures */
    char *constructedFilepath = malloc(5120);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/white-pawn.png");
    self.pieces[0] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/white-rook.png");
    self.pieces[1] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/white-knight.png");
    self.pieces[2] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/white-bishop.png");
    self.pieces[3] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/white-queen.png");
    self.pieces[4] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/white-king.png");
    self.pieces[5] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/black-pawn.png");
    self.pieces[6] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/black-rook.png");
    self.pieces[7] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/black-knight.png");
    self.pieces[8] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/black-bishop.png");
    self.pieces[9] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/black-queen.png");
    self.pieces[10] = turtleTextureLoad(constructedFilepath);

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "pieces/black-king.png");
    self.pieces[11] = turtleTextureLoad(constructedFilepath);

    free(constructedFilepath);

    /* board */
    self.boardX = -20;
    self.boardY = -5;
    self.boardSize = 160;

    /* engines */
    strcpy(self.inputFilename, osToolsFileDialog.executableFilepath);
    strcat(self.inputFilename, "input.txt");
    strcpy(self.outputFilename, osToolsFileDialog.executableFilepath);
    strcat(self.outputFilename, "output.txt");

    self.mohamed = tt_buttonInit("Mohamed", NULL, self.boardX + self.boardSize + 10, self.boardY + self.boardSize - 9, 10);
    self.mohamed -> align = TT_BUTTON_ALIGN_LEFT;
    self.ryan = tt_buttonInit("Ryan", NULL, self.boardX + self.boardSize + 10, self.boardY + self.boardSize - 36, 10);
    self.ryan -> align = TT_BUTTON_ALIGN_LEFT;
}

void setColor(int32_t color) {
    turtlePenColorAlpha(colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 0], colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 1], colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 2], colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 3]);
}

/* import a file to board - TODO */
int32_t import(char *filename) {
    return -1;
}

/* export the board state - TODO */
int32_t export(char *filename) {
    return -1;
}

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

/* get piece texture given board code */
turtle_texture_t getPieceTexture(char code) {
    switch (code) {
        case WHITE_PAWN:
        case WHITE_PAWN_EN_PASSANT:
        return self.pieces[0];
        case WHITE_ROOK:
        case WHITE_ROOK_NO_MOVE:
        return self.pieces[1];
        case WHITE_KNIGHT:
        return self.pieces[2];
        case WHITE_BISHOP:
        return self.pieces[3];
        case WHITE_QUEEN:
        return self.pieces[4];
        case WHITE_KING:
        case WHITE_KING_NO_MOVE:
        return self.pieces[5];
        case BLACK_PAWN:
        case BLACK_PAWN_EN_PASSANT:
        return self.pieces[6];
        case BLACK_ROOK:
        case BLACK_ROOK_NO_MOVE:
        return self.pieces[7];
        case BLACK_KNIGHT:
        return self.pieces[8];
        case BLACK_BISHOP:
        return self.pieces[9];
        case BLACK_QUEEN:
        return self.pieces[10];
        case BLACK_KING:
        case BLACK_KING_NO_MOVE:
        return self.pieces[11];
        default:
        return -1;
    }
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

void render() {
    self.mouseSquare = -1;
    double shift = self.boardSize / 8;
    double xpos = self.boardX - self.boardSize + shift;
    double ypos = self.boardY + self.boardSize - shift;
    /* render board */
    int32_t color = 1;
    for (int32_t i = 0; i < 64; i++) {
        if (self.highlightedSquareBox == i) {
            if (color) {
                setColor(CHESS_COLOR_WHITE_SQUARE_HIGHLIGHT_BOX);
            } else {
                setColor(CHESS_COLOR_BLACK_SQUARE_HIGHLIGHT_BOX);
            }
            turtleRectangle(xpos - shift, ypos - shift, xpos + shift, ypos + shift);
        }
        if (color) {
            if (self.highlightedSquare[0] == i || self.highlightedSquare[1] == i || self.highlightedSquare[2] == i) {
                setColor(CHESS_COLOR_WHITE_SQUARE_HIGHLIGHT);
            } else {
                setColor(CHESS_COLOR_WHITE_SQUARE);
            }
        } else {
            if (self.highlightedSquare[0] == i || self.highlightedSquare[1] == i || self.highlightedSquare[2] == i) {
                setColor(CHESS_COLOR_BLACK_SQUARE_HIGHLIGHT);
            } else {
                setColor(CHESS_COLOR_BLACK_SQUARE);
            }
        }
        if (self.highlightedSquareBox == i) {
            turtleRectangle(xpos - shift * 0.9, ypos - shift * 0.9, xpos + shift * 0.9, ypos + shift * 0.9);
        } else {
            turtleRectangle(xpos - shift, ypos - shift, xpos + shift, ypos + shift);
        }
        if (turtle.mouseX > xpos - shift && turtle.mouseX <= xpos + shift && turtle.mouseY > ypos - shift && turtle.mouseY <= ypos + shift) {
            self.mouseSquare = i;
        }
        if (i % 8 == 0) {
            /* draw number */
            if (color) {
                setColor(CHESS_COLOR_BLACK_SQUARE);
            } else {
                setColor(CHESS_COLOR_WHITE_SQUARE);
            }
            turtleTextWriteStringf(xpos - shift * 0.85, ypos + shift * 0.66, shift * 0.32, 0, "%d", 8 - i / 8);
        }
        if (i > 55) {
            /* draw letter */
            if (color) {
                setColor(CHESS_COLOR_BLACK_SQUARE);
            } else {
                setColor(CHESS_COLOR_WHITE_SQUARE);
            }
            turtleTextWriteStringf(xpos + shift * 0.85, ypos - shift * 0.66, shift * 0.32, 100, "%c", BLACK_PAWN + i % 8);
        }
        color = !color;
        xpos += shift * 2;
        if (i % 8 == 7) {
            xpos = self.boardX - self.boardSize + shift;
            ypos -= shift * 2;
            color = !color;
        }
    }
    /* render pieces */
    xpos = self.boardX - self.boardSize + shift;
    ypos = self.boardY + self.boardSize - shift;
    MUTEX_ACQUIRE(self.boardMutex);
    for (int32_t i = 0; i < 64; i++) {
        if (self.board[i] != BLANK_SPACE) {
            turtle_texture_t texture = getPieceTexture(self.board[i]);
            if (texture != -1 && self.mousePiece != i) {
                turtleTexture(texture, xpos - shift, ypos - shift, xpos + shift, ypos + shift, 0);
            }
        }
        MUTEX_ACQUIRE(self.validMutex);
        if (list_find(self.valid, (unitype) i, 'c') != -1) {
            setColor(CHESS_COLOR_SQUARE_DOT);
            if (self.board[i] == BLANK_SPACE) {
                turtleCircle(xpos, ypos, shift * 0.35);
            } else {
                turtlePenShape("none");
                turtlePenSize(shift * 0.16);
                turtleGoto(xpos, ypos + shift * 0.9);
                turtlePenDown();
                for (int32_t i = 0; i < 20; i++) {
                    turtleGoto(xpos + shift * sin(360 / 20 / 57.2958 * i) * 0.9, ypos + shift * cos(360 / 20 / 57.2958 * i) * 0.9);
                }
                turtleGoto(xpos, ypos + shift * 0.9);
                turtlePenUp();
                turtlePenShape("circle");
            }
        }
        MUTEX_RELEASE(self.validMutex);
        xpos += shift * 2;
        if (i % 8 == 7) {
            xpos = self.boardX - self.boardSize + shift;
            ypos -= shift * 2;
        }
    }
    if (self.mousePiece != -1) {
        turtle_texture_t texture = getPieceTexture(self.board[self.mousePiece]);
        if (texture != -1) {
            turtleTexture(texture, turtle.mouseX - shift, turtle.mouseY - shift, turtle.mouseX + shift, turtle.mouseY + shift, 0);
        }
    }
    MUTEX_RELEASE(self.boardMutex);
    /* render sidebar */
    if (self.turn == CHESS_WHITE) {
        tt_setColor(TT_COLOR_WHITE);
        turtleTextWriteString("Turn: White", self.boardX + self.boardSize + 10, self.boardY, 10, 0);
    } else {
        tt_setColor(TT_COLOR_BLACK);
        turtleTextWriteString("Turn: Black", self.boardX + self.boardSize + 10, self.boardY, 10, 0);
    }
    /* render pawn promotion */
    if (self.pawnPromotionWhite != -1) {
        self.pawnPromotionIndex = -1;
        xpos = self.boardX - self.boardSize + shift + self.pawnPromotionWhite % 8 * shift * 2;
        ypos = self.boardY + self.boardSize - shift - self.pawnPromotionWhite / 8 * shift * 2;
        char promotionOptions[4] = {WHITE_QUEEN, WHITE_KNIGHT, WHITE_ROOK, WHITE_BISHOP};
        tt_setColor(TT_COLOR_BACKGROUND);
        turtleRectangle(xpos - shift * 1.05, ypos - shift * 7.05, xpos + shift * 1.05, ypos + shift * 1.05); // simulate drop shadow
        setColor(CHESS_COLOR_PAWN_PROMOTION);
        turtleRectangle(xpos - shift, ypos - shift * 7, xpos + shift, ypos + shift);
        for (int32_t j = 0; j < 4; j++) {
            turtle_texture_t texture = getPieceTexture(promotionOptions[j]);
            turtleTexture(texture, xpos - shift, ypos - shift, xpos + shift, ypos + shift, 0);
            if (turtle.mouseX > xpos - shift && turtle.mouseX <= xpos + shift && turtle.mouseY > ypos - shift && turtle.mouseY <= ypos + shift) {
                self.pawnPromotionIndex = j;
            }
            ypos -= shift * 2;
        }
    } else if (self.pawnPromotionBlack != -1) {
        self.pawnPromotionIndex = -1;
        xpos = self.boardX - self.boardSize + shift + self.pawnPromotionBlack % 8 * shift * 2;
        ypos = self.boardY + self.boardSize - shift - self.pawnPromotionBlack / 8 * shift * 2;
        char promotionOptions[4] = {BLACK_QUEEN, BLACK_KNIGHT, BLACK_ROOK, BLACK_BISHOP};
        tt_setColor(TT_COLOR_BACKGROUND);
        turtleRectangle(xpos - shift * 1.05, ypos - shift * 1.05, xpos + shift * 1.05, ypos + shift * 7.05); // simulate drop shadow
        setColor(CHESS_COLOR_PAWN_PROMOTION);
        turtleRectangle(xpos - shift, ypos - shift, xpos + shift, ypos + shift * 7);
        for (int32_t j = 0; j < 4; j++) {
            turtle_texture_t texture = getPieceTexture(promotionOptions[j]);
            turtleTexture(texture, xpos - shift, ypos - shift, xpos + shift, ypos + shift, 0);
            if (turtle.mouseX > xpos - shift && turtle.mouseX <= xpos + shift && turtle.mouseY > ypos - shift && turtle.mouseY <= ypos + shift) {
                self.pawnPromotionIndex = j;
            }
            ypos += shift * 2;
        }
    }
    if (self.pawnPromotionWhite != -1 || self.pawnPromotionBlack != -1) {
        self.mohamed -> enabled = TT_ELEMENT_NO_MOUSE;
        self.ryan -> enabled = TT_ELEMENT_NO_MOUSE;
    } else {
        self.mohamed -> enabled = TT_ELEMENT_ENABLED;
        self.ryan -> enabled = TT_ELEMENT_ENABLED;
    }

    /* engine buttons */
    if (self.mohamed -> value) {
        self.mohamed -> value = 0;
        int32_t status = engineMove("mohamed.exe");
        if (status != 0) {

        }
    }
    if (self.ryan -> value) {
        self.ryan -> value = 0;
        int32_t status = engineMove("ryan.exe");
        if (status != 0) {

        }
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
                boardCopy[63 + 1] = WHITE_ROOK; // revert rook
                boardCopy[62 + 1] = BLANK_SPACE; // revert king
                boardCopy[60 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[60 + 1] = BLANK_SPACE; // revert king
                boardCopy[61 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            } else {
                /* white left castle */
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
                boardCopy[7 + 1] = WHITE_ROOK; // revert rook
                boardCopy[6 + 1] = BLANK_SPACE; // revert king
                boardCopy[4 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[4 + 1] = BLANK_SPACE; // revert king
                boardCopy[5 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            } else {
                /* black left castle */
                boardCopy[0 + 1] = WHITE_ROOK; // revert rook
                boardCopy[1 + 1] = BLANK_SPACE; // revert king
                boardCopy[4 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[4 + 1] = BLANK_SPACE; // revert king
                boardCopy[3 + 1] = WHITE_KING; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[3 + 1] = BLANK_SPACE; // revert king
                boardCopy[2 + 1] = WHITE_KING; // king walk
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
        if (board[position] != BLANK_SPACE) {
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
                        boardCopy[63 + 1] = WHITE_ROOK; // revert rook
                        boardCopy[62 + 1] = BLANK_SPACE; // revert king
                        boardCopy[61 + 1] = WHITE_KING; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                    } else {
                        /* white left castle */
                        boardCopy[56 + 1] = WHITE_ROOK; // revert rook
                        boardCopy[57 + 1] = BLANK_SPACE; // revert king
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
                        boardCopy[7 + 1] = WHITE_ROOK; // revert rook
                        boardCopy[6 + 1] = BLANK_SPACE; // revert king
                        boardCopy[5 + 1] = WHITE_KING; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                    } else {
                        /* black left castle */
                        boardCopy[0 + 1] = WHITE_ROOK; // revert rook
                        boardCopy[1 + 1] = BLANK_SPACE; // revert king
                        boardCopy[3 + 1] = WHITE_KING; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                        boardCopy[3 + 1] = BLANK_SPACE; // revert king
                        boardCopy[2 + 1] = WHITE_KING; // king walk
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
                if (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE || capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE) {
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
                        if (capturedPiece == WHITE_KING || capturedPiece == WHITE_KING_NO_MOVE || capturedPiece == BLACK_KING || capturedPiece == BLACK_KING_NO_MOVE) {
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
    printf("Number of moves: %d\n", moves -> length / MOVES_NUMBER_OF_FIELDS);
    return moves;
}

/* check for checkmate, stalemate, check, or none */
int32_t checkBoardState(char *board, int8_t turn) {
    /* check for check */
    int8_t check = 0;

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
    }
    return BOARD_STATE_NONE;
}

int32_t engineMove(char *engineName) {
    MUTEX_ACQUIRE(self.boardMutex);
    list_t *moves = generateAllMoves(self.board, self.turn);
    /* place moves in inputFile */
    FILE *inputfp = fopen(self.inputFilename, "wb");
    if (inputfp == NULL) {
        printf("ERROR: Could not open file %s\n", self.inputFilename);
        list_free(moves);
        MUTEX_RELEASE(self.boardMutex);
        return -1;
    }
    char turnChar = 'w';
    if (self.turn == CHESS_BLACK) {
        turnChar = 'b';
    }
    fwrite(&turnChar, 1, 1, inputfp);
    fwrite(self.board, 1, 64, inputfp);
    MUTEX_RELEASE(self.boardMutex);
    fwrite("\n", 1, 1, inputfp);
    for (int32_t movesIndex = 0; movesIndex < moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        fwrite(moves -> data[movesIndex + MOVES_STRING].s, 1, 65, inputfp);
        fwrite("\n", 1, 1, inputfp);
    }
    fclose(inputfp);
    /* run engine */
    char *command = malloc(8192);
    sprintf(command, "\"\"%s%s\" \"%s\" \"%s\"\"", osToolsFileDialog.executableFilepath, engineName, self.inputFilename, self.outputFilename); // idk why it needs to be double quoted
    // printf("%s\n", command);
    int32_t status = system(command);
    free(command);
    if (status != 0) {
        printf("ERROR: %s returned error code %d\n", engineName, status);
        list_free(moves);
        return -1;
    }
    /* check board */
    FILE *outputfp = fopen(self.outputFilename, "r");
    if (outputfp == NULL) {
        printf("ERROR: Could not open file %s\n", self.outputFilename);
        list_free(moves);
        return -1;
    }
    char line[1024];
    fgets(line, 1024, outputfp);
    fclose(outputfp);
    for (int32_t i = 0; i < 2; i++) {
        if (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r') {
            line[strlen(line) - 1] = '\0';
        }
    }
    int32_t found = -1;
    for (int32_t movesIndex = 0; movesIndex < moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        if (strcmp(moves -> data[movesIndex + MOVES_STRING].s, line) == 0) {
            found = movesIndex;
            break;
        }
    }
    if (found == -1) {
        printf("ERROR: %s output (%s) is invalid\n", engineName, line);
        list_free(moves);
        return -1;
    }
    /* make move */
    MUTEX_ACQUIRE(self.boardMutex);
    int32_t move = movePiece(self.board, moves -> data[found + MOVES_FROM].c, moves -> data[found + MOVES_TO].c);
    /* check for pawn promotion */
    if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE || move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
        self.board[moves -> data[found + MOVES_TO].c] = line[moves -> data[found + MOVES_TO].c + 1];
    }
    MUTEX_ACQUIRE(self.validMutex);
    list_clear(self.valid);
    MUTEX_RELEASE(self.validMutex);
    self.highlightedSquare[1] = moves -> data[found + MOVES_FROM].c;
    self.highlightedSquare[2] = moves -> data[found + MOVES_TO].c;
    self.highlightedSquare[0] = -1;
    self.mousePiece = -1;
    self.turn = !self.turn;
    list_free(moves);
    checkBoardState(self.board, self.turn);
    MUTEX_RELEASE(self.boardMutex);
    return 0;
}

void mouse() {
    if (self.pawnPromotionWhite != -1 || self.pawnPromotionBlack != -1) {
        self.mouseSquare = -1;           
    }
    if (turtleMouseDown()) {
        if (self.keys[KEYS_LMB] == 0) {
            /* first tick */
            self.keys[KEYS_LMB] = 1;
            if ((self.pawnPromotionWhite != -1 || self.pawnPromotionBlack != -1) && self.pawnPromotionIndex != -1) {
                MUTEX_ACQUIRE(self.boardMutex);
                if (self.pawnPromotionWhite != -1) {
                    char promotionOptions[4] = {WHITE_QUEEN, WHITE_KNIGHT, WHITE_ROOK, WHITE_BISHOP};
                    self.board[self.pawnPromotionWhite] = promotionOptions[self.pawnPromotionIndex];
                    self.pawnPromotionWhite = -1;
                } else {
                    char promotionOptions[4] = {BLACK_QUEEN, BLACK_KNIGHT, BLACK_ROOK, BLACK_BISHOP};
                    self.board[self.pawnPromotionBlack] = promotionOptions[self.pawnPromotionIndex];
                    self.pawnPromotionBlack = -1;
                }
                self.turn = !self.turn;
                checkBoardState(self.board, self.turn);
                MUTEX_RELEASE(self.boardMutex);
                return;
            }
            self.mousePiece = self.mouseSquare;
            MUTEX_ACQUIRE(self.validMutex);
            MUTEX_ACQUIRE(self.boardMutex);
            if (self.mousePiece != -1 && list_find(self.valid, (unitype) self.mousePiece, 'c') >= 0) {
                /* make move */
                int32_t move = movePiece(self.board, self.highlightedSquare[0], self.mousePiece);
                /* check for pawn promotion */
                if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
                    self.pawnPromotionWhite = self.mousePiece;
                } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    self.pawnPromotionBlack = self.mousePiece;
                } else {
                    self.turn = !self.turn;
                }
                list_clear(self.valid);
                self.highlightedSquare[1] = self.highlightedSquare[0];
                self.highlightedSquare[2] = self.mousePiece;
                self.highlightedSquare[0] = -1;
                self.mousePiece = -1;
                if (move != MOVE_PIECE_PAWN_PROMOTION_WHITE && move != MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    checkBoardState(self.board, self.turn);
                }
            } else if (self.mousePiece == -1 || self.board[self.mousePiece] == BLANK_SPACE) {
                list_clear(self.valid);
                self.highlightedSquare[0] = -1;
                self.mousePiece = -1;
            } else {
                if (self.highlightedSquare[0] == self.mousePiece) {
                    self.highlightUnselect = 1;
                }
                self.highlightedSquare[0] = self.mousePiece;
                list_free(self.valid);
                self.valid = generateLegalMoves(self.board, self.highlightedSquare[0], self.turn);
            }
            MUTEX_RELEASE(self.boardMutex);
            MUTEX_RELEASE(self.validMutex);
        } else {
            /* mouse held */
            if (self.mousePiece != -1) {
                self.highlightedSquareBox = self.mouseSquare;
            }
        }
    } else {
        if (self.keys[KEYS_LMB] == 1) {
            self.keys[KEYS_LMB] = 0;
            MUTEX_ACQUIRE(self.validMutex);
            if (self.mousePiece == self.highlightedSquare[0] && list_find(self.valid, (unitype) self.mouseSquare, 'c') >= 0) {
                /* make move */
                MUTEX_ACQUIRE(self.boardMutex);
                int32_t move = movePiece(self.board, self.highlightedSquare[0], self.mouseSquare);
                /* special: pawn promotion */
                if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
                    self.pawnPromotionWhite = self.mouseSquare;
                } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    self.pawnPromotionBlack = self.mouseSquare;
                } else {
                    self.turn = !self.turn;
                }
                list_clear(self.valid);
                self.highlightedSquare[1] = self.highlightedSquare[0];
                self.highlightedSquare[2] = self.mouseSquare;
                self.highlightedSquare[0] = -1;
                if (move != MOVE_PIECE_PAWN_PROMOTION_WHITE && move != MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    checkBoardState(self.board, self.turn);
                }
                MUTEX_RELEASE(self.boardMutex);
            } else if (self.mouseSquare == self.mousePiece && self.highlightUnselect) {
                list_clear(self.valid);
                self.highlightedSquare[0] = -1;
            }
            MUTEX_RELEASE(self.validMutex);
            self.mousePiece = -1;
            self.highlightedSquareBox = -1;
            self.highlightUnselect = 0;
        }
    }
    if (turtleKeyPressed(GLFW_KEY_SPACE)) {
        if (self.keys[KEYS_SPACE] == 0) {
            self.keys[KEYS_SPACE] = 1;
            MUTEX_ACQUIRE(self.boardMutex);
            printBoard(self.board);
            MUTEX_RELEASE(self.boardMutex);
        }
    } else {
        self.keys[KEYS_SPACE] = 0;
    }
}

void parseRibbonOutput() {
    if (tt_ribbon.output[0] == 0) {
        return;
    }
    tt_ribbon.output[0] = 0;
    if (tt_ribbon.output[1] == 0) { // File
        if (tt_ribbon.output[2] == 1) { // New
            list_clear(osToolsFileDialog.selectedFilenames);
            self.turn = CHESS_WHITE;
            MUTEX_ACQUIRE(self.boardMutex);
            memcpy(self.board, "HCDEIDCHAAAAAAAA000000000000000000000000000000001111111183459438", 64);
            MUTEX_RELEASE(self.boardMutex);
            self.mouseSquare = -1;
            self.mousePiece = -1;
            self.highlightedSquare[0] = -1;
            self.highlightedSquare[1] = -1;
            self.highlightedSquare[2] = -1;
            self.highlightedSquareBox = -1;
            self.highlightUnselect = 0;
            self.pawnPromotionWhite = -1;
            self.pawnPromotionBlack = -1;
            self.pawnPromotionIndex = -1;
            MUTEX_ACQUIRE(self.validMutex);
            list_clear(self.valid);
            MUTEX_RELEASE(self.validMutex);
        }
        if (tt_ribbon.output[2] == 2) { // Save
            if (osToolsFileDialog.selectedFilenames -> length == 0) {
                if (osToolsFileDialogSave(OSTOOLS_FILE_DIALOG_FILE, "Save.txt", NULL) != -1) {
                    printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
                }
            } else {
                printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
            }
        }
        if (tt_ribbon.output[2] == 3) { // Save As...
            list_clear(osToolsFileDialog.selectedFilenames);
            if (osToolsFileDialogSave(OSTOOLS_FILE_DIALOG_FILE, "Save.txt", NULL) != -1) {
                printf("Saved to: %s\n", osToolsFileDialog.selectedFilenames -> data[0].s);
            }
        }
        if (tt_ribbon.output[2] == 4) { // Open
            list_clear(osToolsFileDialog.selectedFilenames);
            if (osToolsFileDialogOpen(OSTOOLS_FILE_DIALOG_MULTIPLE_SELECT, OSTOOLS_FILE_DIALOG_FILE, "", NULL) != -1) {
                printf("Loaded data from: ");
                list_print(osToolsFileDialog.selectedFilenames);
            }
        }
    }
    if (tt_ribbon.output[1] == 1) { // View
        if (tt_ribbon.output[2] == 1) { // Change theme
            if (self.theme == CHESS_THEME_DEFAULT) {
                self.theme = CHESS_THEME_CHESS_COM;
            } else {
                self.theme = CHESS_THEME_DEFAULT;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* create window */
    GLFWwindow *window = turtleCreateWindowIcon(TURTLE_WINDOW_DEFAULT_WIDTH, TURTLE_WINDOW_DEFAULT_HEIGHT, "chess", "images/thumbnail.png");
    if (window == NULL) {
        return -1; // failed to create window
    }

    /* initialise turtle */
    turtleSetResizeMode(TURTLE_RESIZE_MODE_PAD); // change to TURTLE_RESIZE_MODE_STRETCH to have content stretch when resized
    turtleInit(window, -320, -180, 320, 180);
    
    /* initialise osTools */
    osToolsInit(argv[0], window); // must include argv[0] to get executableFilepath, must include GLFW window for copy paste and cursor functionality
    osToolsFileDialogAddGlobalExtension("txt"); // add txt to extension restrictions
    osToolsFileDialogAddGlobalExtension("csv"); // add csv to extension restrictions

    /* initialise turtleText */
    char *constructedFilepath = malloc(5120);
    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "config/roberto.tgl");
    turtleTextInit(constructedFilepath);
    free(constructedFilepath);

    /* initialise turtleTools ribbon */
    turtleToolsSetTheme(TT_THEME_DARK); // dark theme preset

    list_t *ribbonConfig = list_init();
    list_append(ribbonConfig, (unitype) "File, 📄 New, 📄 Save, 📄 Save As..., 📄 Open", 's');
    list_append(ribbonConfig, (unitype) "View, Change Theme", 's');
    tt_ribbonInitList(ribbonConfig);

    init();

    uint32_t tps = 120; // ticks per second (locked to fps in this case)
    clock_t start, end;
    while (turtle.close == 0) {
        start = clock();
        turtleGetMouseCoordinates();
        turtleClear();
        render();
        mouse();
        turtleToolsUpdate(); // update turtleTools
        tt_setColor(TT_COLOR_TEXT);
        turtleTextWriteStringf(-310, -170, 5, 0, "%.2lf, %.2lf", turtle.mouseX, turtle.mouseY);
        parseRibbonOutput(); // user defined function to use ribbon
        turtleUpdate(); // update the screen
        end = clock();
        while ((double) (end - start) / CLOCKS_PER_SEC < (1.0 / tps)) {
            end = clock();
        }
    }
    turtleFree();
    glfwTerminate();
    return 0;
}
