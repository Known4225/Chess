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
- Pawn promotion
- Implement bot API
- Formal notion of checkmate & stalemate
*/

#include "turtle.h"
#include <time.h>

void generateAllMoves(char *filename);
int32_t movePiece(char *board, int8_t positionFrom, int8_t positionTo);

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

enum {
    MOVES_STRING = 0,
    MOVES_FROM = 1,
    MOVES_TO = 2,
    MOVES_EXTRA_CHECKS = 3,
    MOVES_NUMBER_OF_FIELDS = 4,
};

enum {
    MOVE_PIECE_ERROR = -1,
    MOVE_PIECE_SUCCESSFUL = 0,
    MOVE_PIECE_PAWN_PROMOTION_WHITE = 1,
    MOVE_PIECE_PAWN_PROMOTION_BLACK = 2,
    MOVE_PIECE_CASTLE_WHITE = 3,
    MOVE_PIECE_CASTLE_BLACK = 4,
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
    list_t *dotSquares; // list of squares marked with a dot (for valid)
    int8_t pawnPromotionWhite;
    int8_t pawnPromotionBlack;
    int8_t pawnPromotionIndex;

    /* board */
    double boardX;
    double boardY;
    double boardSize;

    /* engines */
    list_t *moves; // see MOVES_X
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
    self.dotSquares = list_init();

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
    self.moves = list_init();
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

/* import a file to board */
int32_t import(char *filename) {

}

/* export the board state */
int32_t export(char *filename) {

}

void printBoard(char *board) {
    for (int32_t j = 0; j < 8; j++) {
        for (int32_t i = 0; i < 8; i++) {
            printf("%c", board[j * 8 + i]);
        }
        printf("\n");
    }
}

void printDotSquares() {
    printf("self.dotSquares = [");
    for (int32_t i = 0; i < self.dotSquares -> length; i++) {
        printf("%d ", self.dotSquares -> data[i].c);
    }
    printf("]\n");
}

/* get piece texture given board code */
turtle_texture_t getPieceTexture(char code) {
    switch (code) {
        case '1':
        case '7':
        return self.pieces[0];
        case '2':
        case '8':
        return self.pieces[1];
        case '3':
        return self.pieces[2];
        case '4':
        return self.pieces[3];
        case '5':
        return self.pieces[4];
        case '6':
        case '9':
        return self.pieces[5];
        case 'A':
        case 'G':
        return self.pieces[6];
        case 'B':
        case 'H':
        return self.pieces[7];
        case 'C':
        return self.pieces[8];
        case 'D':
        return self.pieces[9];
        case 'E':
        return self.pieces[10];
        case 'F':
        case 'I':
        return self.pieces[11];
        default:
        return -1;
    }
}

chess_piece_t getPieceType(char code) {
    switch (code) {
        case '1':
        case '7':
        case 'A':
        case 'G':
        return CHESS_PIECE_PAWN;
        case '2':
        case '8':
        case 'B':
        case 'H':
        return CHESS_PIECE_ROOK;
        case '3':
        case 'C':
        return CHESS_PIECE_KNIGHT;
        case '4':
        case 'D':
        return CHESS_PIECE_BISHOP;
        case '5':
        case 'E':
        return CHESS_PIECE_QUEEN;
        case '6':
        case '9':
        case 'F':
        case 'I':
        return CHESS_PIECE_KING;
        default:
        return -1;
    }
}

chess_color_t getPieceColor(char code) {
    switch (code) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        return CHESS_WHITE;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
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
            turtleTextWriteStringf(xpos + shift * 0.85, ypos - shift * 0.66, shift * 0.32, 100, "%c", 'a' + i % 8);
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
    for (int32_t i = 0; i < 64; i++) {
        if (self.board[i] != '0') {
            turtle_texture_t texture = getPieceTexture(self.board[i]);
            if (texture != -1 && self.mousePiece != i) {
                turtleTexture(texture, xpos - shift, ypos - shift, xpos + shift, ypos + shift, 0);
            }
        }
        if (list_find(self.dotSquares, (unitype) i, 'c') != -1) {
            setColor(CHESS_COLOR_SQUARE_DOT);
            if (self.board[i] == '0') {
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
        char promotionOptions[4] = {'5', '3', '2', '4'};
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
        char promotionOptions[4] = {'E', 'C', 'B', 'D'};
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
        generateAllMoves(self.inputFilename);
    }
    if (self.ryan -> value) {
        self.ryan -> value = 0;
        generateAllMoves(self.inputFilename);
        char *command = malloc(8192);
        // sprintf(command, "\"%sryan.exe\"", osToolsFileDialog.executableFilepath);
        sprintf(command, "\"\"%sryan.exe\" \"%s\" \"%s\"\"", osToolsFileDialog.executableFilepath, self.inputFilename, self.outputFilename); // idk why it needs to be double quoted
        // printf("%s\n", command);
        int32_t status = system(command);
        free(command);
        if (status != 0) {
            printf("ERROR: ryan.exe returned error code %d\n", status);
            return;
        }
        /* check board */
        FILE *fp = fopen(self.outputFilename, "r");
        if (fp == NULL) {
            printf("ERROR: Could not open file %s\n", self.outputFilename);
            return;
        }
        char line[1024];
        fgets(line, 1024, fp);
        fclose(fp);
        for (int32_t i = 0; i < 2; i++) {
            if (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r') {
                line[strlen(line) - 1] = '\0';
            }
        }
        int32_t found = -1;
        for (int32_t movesIndex = 0; movesIndex < self.moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
            if (strcmp(self.moves -> data[movesIndex + MOVES_STRING].s, line) == 0) {
                found = movesIndex;
                break;
            }
        }
        if (found == -1) {
            printf("ERROR: ryan.exe output (%s) is invalid\n", line);
            return;
        }
        /* make move */
        movePiece(self.board, self.moves -> data[found + MOVES_FROM].c, self.moves -> data[found + MOVES_TO].c);
        list_clear(self.dotSquares);
        self.highlightedSquare[1] = self.moves -> data[found + MOVES_FROM].c;
        self.highlightedSquare[2] = self.moves -> data[found + MOVES_TO].c;
        self.highlightedSquare[0] = -1;
        self.mousePiece = -1;
        self.turn = !self.turn;
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

/* naive generation - does not cull moves that would put the king in check, returns moves in self.dotSquares */
void generateNaiveMoves(char *board, int8_t position, int8_t turn) {
    list_clear(self.dotSquares);
    if (position == -1) {
        return;
    }
    chess_piece_t type = getPieceType(board[position]);
    chess_color_t color = getPieceColor(board[position]);
    if (type == -1 || color == -1) {
        return;
    }
    if (color != turn) {
        return;
    }
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
            if (board[wanderingPosition] == '0') {
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                spaceEmpty = 1;
            }
        }
        /* possible: go up again */
        if (spaceEmpty && position / 8 == pawnPowerRow) {
            wanderingPosition = direction[0](wanderingPosition);
            if (wanderingPosition != -1) {
                if (board[wanderingPosition] == '0') {
                    list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                }
            }
        }
        /* check for capture (and en passant) */
        wanderingPosition = direction[1](position);
        int8_t enpassantPosition = left(position);
        if (wanderingPosition != -1) {
            if ((board[wanderingPosition] != '0' && getPieceColor(board[wanderingPosition]) != color) || (board[wanderingPosition] == '0' && getPieceColor(board[enpassantPosition]) != color && (board[enpassantPosition] == '7' || board[enpassantPosition] == 'G'))) {
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
            }
        }
        wanderingPosition = direction[2](position);
        enpassantPosition = right(position);
        if (wanderingPosition != -1) {
            if ((board[wanderingPosition] != '0' && getPieceColor(board[wanderingPosition]) != color) || (board[wanderingPosition] == '0' && getPieceColor(board[enpassantPosition]) != color && (board[enpassantPosition] == '7' || board[enpassantPosition] == 'G'))) {
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
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
                if (board[wanderingPosition] == '0') {
                    list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
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
            if (checkPosition != -1 && (board[checkPosition] == '0' || getPieceColor(board[checkPosition]) != color)) {
                list_append(self.dotSquares, (unitype) checkPosition, 'c');
            }
            checkPosition = direction[j + 2](wanderingPosition);
            if (checkPosition != -1 && (board[checkPosition] == '0' || getPieceColor(board[checkPosition]) != color)) {
                list_append(self.dotSquares, (unitype) checkPosition, 'c');
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
                if (board[wanderingPosition] == '0') {
                    list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
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
                if (board[wanderingPosition] == '0') {
                    list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                } else {
                    if (getPieceColor(board[wanderingPosition]) != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
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
            if (wanderingPosition != -1 && (board[wanderingPosition] == '0' || getPieceColor(board[wanderingPosition]) != color)) {
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
            }
        }
        /* castling */
        if (board[position] == '9' && board[63] == '8' && board[62] == '0' && board[61] == '0') {
            /* white right castle */
            list_append(self.dotSquares, (unitype) 62, 'c');
        }
        if (board[position] == '9' && board[56] == '8' && board[57] == '0' && board[58] == '0' && board[59] == '0') {
            /* white left castle */
            list_append(self.dotSquares, (unitype) 57, 'c');
        }
        if (board[position] == 'I' && board[7] == 'H' && board[6] == '0' && board[5] == '0') {
            /* black right castle */
            list_append(self.dotSquares, (unitype) 6, 'c');
        }
        if (board[position] == 'I' && board[0] == 'H' && board[1] == '0' && board[2] == '0' && board[3] == '0') {
            /* black left castle */
            list_append(self.dotSquares, (unitype) 1, 'c');
        }
    }
}

/* generates moves */
void generateLegalMoves(char *board, int8_t position, int8_t turn) {
    generateNaiveMoves(self.board, position, self.turn);
    list_clear(self.moves);
    char boardCopy[66];
    boardCopy[65] = '\0';
    for (int32_t i = 0; i < self.dotSquares -> length; i++) {
        if (self.turn == CHESS_WHITE) {
            boardCopy[0] = 'b';
        } else {
            boardCopy[0] = 'w';
        }
        memcpy(boardCopy + 1, self.board, 64);
        int32_t move = movePiece(boardCopy + 1, position, self.dotSquares -> data[i].c);
        /* check results */
        if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
            char promotionOptions[4] = {'5', '3', '2', '4'};
            for (int32_t j = 0; j < 4; j++) {
                boardCopy[self.dotSquares -> data[i].c + 1] = promotionOptions[j];
                list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                list_append(self.moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
            }
        } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
            char promotionOptions[4] = {'E', 'C', 'B', 'D'};
            for (int32_t j = 0; j < 4; j++) {
                boardCopy[self.dotSquares -> data[i].c + 1] = promotionOptions[j];
                list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                list_append(self.moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
            }
        } else if (move == MOVE_PIECE_CASTLE_WHITE) {
            list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
            list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
            list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
            list_t *extraChecks = list_init();
            if (self.dotSquares -> data[i].c == 62) {
                /* white right castle */
                boardCopy[63 + 1] = '2'; // revert rook
                boardCopy[62 + 1] = '0'; // revert king
                boardCopy[60 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[60 + 1] = '0'; // revert king
                boardCopy[61 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            } else {
                /* white left castle */
                boardCopy[56 + 1] = '2'; // revert rook
                boardCopy[57 + 1] = '0'; // revert king
                boardCopy[60 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[60 + 1] = '0'; // revert king
                boardCopy[59 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[59 + 1] = '0'; // revert king
                boardCopy[58 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            }
            list_append(self.moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
        } else if (move == MOVE_PIECE_CASTLE_BLACK) {
            list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
            list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
            list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
            list_t *extraChecks = list_init();
            if (self.dotSquares -> data[i].c == 6) {
                /* black right castle */
                boardCopy[7 + 1] = '2'; // revert rook
                boardCopy[6 + 1] = '0'; // revert king
                boardCopy[4 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[4 + 1] = '0'; // revert king
                boardCopy[5 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            } else {
                /* black left castle */
                boardCopy[0 + 1] = '2'; // revert rook
                boardCopy[1 + 1] = '0'; // revert king
                boardCopy[4 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[4 + 1] = '0'; // revert king
                boardCopy[3 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
                boardCopy[3 + 1] = '0'; // revert king
                boardCopy[2 + 1] = '6'; // king walk
                list_append(extraChecks, (unitype) boardCopy, 's');
            }
            list_append(self.moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
        } else if (move == MOVE_PIECE_SUCCESSFUL) {
            list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
            list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
            list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
            list_append(self.moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
        } else {
            printf("ERROR: movePiece returned MOVE_PIECE_ERROR\n");
        }
    }
    /* second pass - validate legal moves are legal (don't put king in check, etc) */
    for (int32_t movesIndex = 0; movesIndex < self.moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        for (int32_t positionIndex = 0; positionIndex < 64; positionIndex++) {
            generateNaiveMoves(self.moves -> data[movesIndex + MOVES_STRING].s + 1, positionIndex, !self.turn);
            for (int32_t i = 0; i < self.dotSquares -> length; i++) {
                char capturedPiece = self.moves -> data[movesIndex + MOVES_STRING].s[self.dotSquares -> data[i].c + 1];
                if ((self.turn == CHESS_WHITE && (capturedPiece == '6' || capturedPiece == '9')) || (self.turn == CHESS_BLACK && (capturedPiece == 'F' || capturedPiece == 'I'))) {
                    /* this moves captures the king, invalidate the legal move */
                    // printf("invalidate due to %d to %d\n", positionIndex, self.dotSquares -> data[i].c);
                    // printDotSquares();
                    // printBoard(self.moves -> data[movesIndex + MOVES_STRING].s + 1);
                    for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                        list_delete(self.moves, movesIndex);
                    }
                    movesIndex -= MOVES_NUMBER_OF_FIELDS;
                    goto SECOND_PASS_NEXT;
                }
            }
        }
        /* extra moves to validate - castling cannot have the king "walk" through check */
        if (self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r != NULL) {
            for (int32_t extra = 0; extra < self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> length; extra++) {
                for (int32_t positionIndex = 0; positionIndex < 64; positionIndex++) {
                    generateNaiveMoves(self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s + 1, positionIndex, !self.turn);
                    for (int32_t i = 0; i < self.dotSquares -> length; i++) {
                        char capturedPiece = self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[self.dotSquares -> data[i].c + 1];
                        if ((self.turn == CHESS_WHITE && (capturedPiece == '6' || capturedPiece == '9')) || (self.turn == CHESS_BLACK && (capturedPiece == 'F' || capturedPiece == 'I'))) {
                            /* this moves captures the king, invalidate the legal move */
                            for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                                list_delete(self.moves, movesIndex);
                            }
                            movesIndex -= MOVES_NUMBER_OF_FIELDS;
                            goto SECOND_PASS_NEXT;
                        }
                    }
                }
            }
        }
        SECOND_PASS_NEXT:
    }
    list_clear(self.dotSquares);
    for (int32_t movesIndex = 0; movesIndex < self.moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        list_append(self.dotSquares, self.moves -> data[movesIndex + MOVES_TO], 'c');
    }
}

/* simulate a move - returns one of MOVE_PIECE_X */
int32_t movePiece(char *board, int8_t positionFrom, int8_t positionTo) {
    if (positionFrom < 0 || positionTo < 0) {
        return MOVE_PIECE_ERROR;
    }
    char piece = board[positionFrom];
    /* check if piece is pawn (with or without en passant), rook (that hasn't moved), or king (that hasn't moved) */
    if (piece == '1' && positionFrom / 8 == 6 && positionTo / 8 == 4) {
        /* change normal pawn to en passant pawn */
        piece = '7';
    } else if (piece == '7') {
        /* revert en passant to normal pawn */
        piece = '1';
    } else if (piece == '8') {
        /* revert rook to has moved */
        piece = '2';
    } else if (piece == '9') {
        /* revert king to has moved */
        piece = '6';
    } else if (piece == 'A' && positionFrom / 8 == 1 && positionTo / 8 == 3) {
        /* change normal pawn to en passant pawn */
        piece = 'G';
    } else if (piece == 'G') {
        /* revert en passant to normal pawn */
        piece = 'A';
    } else if (piece == 'H') {
        /* revert rook to has moved */
        piece = 'B';
    } else if (piece == 'I') {
        /* revert king to has moved */
        piece = 'F';
    }
    /* check: pawn promotion */
    if (piece == '1' && positionTo / 8 == 0) {
        board[positionFrom] = '0';
        board[positionTo] = piece;
        return MOVE_PIECE_PAWN_PROMOTION_WHITE;
    } else if (piece == 'A' && positionTo / 8 == 7) {
        board[positionFrom] = '0';
        board[positionTo] = piece;
        return MOVE_PIECE_PAWN_PROMOTION_BLACK;
    } else {
        /* simulate piece moving */
        board[positionFrom] = '0';
        board[positionTo] = piece;
        /* check en passant */
        if (piece == '1') {
            int8_t enpassant = down(positionTo);
            if (enpassant != -1 && self.board[enpassant] == 'G') {
                board[enpassant] = '0';
            }
        } else if (piece == 'b') {
            int8_t enpassant = up(positionTo);
            if (enpassant != -1 && self.board[enpassant] == '7') {
                board[enpassant] = '0';
            }
        }
        /* check castle */
        if (piece == '6' && positionFrom == 60 && positionTo == 62) {
            /* white right castle */
            board[63] = '0';
            board[61] = '2';
            return MOVE_PIECE_CASTLE_WHITE;
        } else if (piece == '6' && positionFrom == 60 && positionTo == 57) {
            /* white left castle */
            board[56] = '0';
            board[58] = '2';
            return MOVE_PIECE_CASTLE_WHITE;
        } else if (piece == 'F' && positionFrom == 4 && positionTo == 6) {
            /* black right castle */
            board[7] = '0';
            board[5] = 'B';
            return MOVE_PIECE_CASTLE_BLACK;
        } else if (piece == 'F' && positionFrom == 4 && positionTo == 1) {
            /* black left castle */
            board[0] = '0';
            board[2] = 'B';
            return MOVE_PIECE_CASTLE_BLACK;
        }
    }
    return MOVE_PIECE_SUCCESSFUL;
}

/* generate all moves and augmented board positions and place them in a file */
void generateAllMoves(char *filename) {
    list_clear(self.moves);
    for (int32_t position = 0; position < 64; position++) {
        if (self.board[position] != '0') {
            generateNaiveMoves(self.board, position, self.turn);
            char boardCopy[66];
            boardCopy[65] = '\0';
            for (int32_t i = 0; i < self.dotSquares -> length; i++) {
                if (self.turn == CHESS_WHITE) {
                    boardCopy[0] = 'b';
                } else {
                    boardCopy[0] = 'w';
                }
                memcpy(boardCopy + 1, self.board, 64);
                int32_t move = movePiece(boardCopy + 1, position, self.dotSquares -> data[i].c);
                /* check results */
                if (move == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
                    char promotionOptions[4] = {'5', '3', '2', '4'};
                    for (int32_t j = 0; j < 4; j++) {
                        boardCopy[self.dotSquares -> data[i].c + 1] = promotionOptions[j];
                        list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                        list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                        list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                        list_append(self.moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
                    }
                } else if (move == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    char promotionOptions[4] = {'E', 'C', 'B', 'D'};
                    for (int32_t j = 0; j < 4; j++) {
                        boardCopy[self.dotSquares -> data[i].c + 1] = promotionOptions[j];
                        list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                        list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                        list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                        list_append(self.moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
                    }
                } else if (move == MOVE_PIECE_CASTLE_WHITE) {
                    list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                    list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                    list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                    list_t *extraChecks = list_init();
                    if (self.dotSquares -> data[i].c == 62) {
                        /* white right castle */
                        boardCopy[63 + 1] = '2'; // revert rook
                        boardCopy[62 + 1] = '0'; // revert king
                        boardCopy[61 + 1] = '6'; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                    } else {
                        /* white left castle */
                        boardCopy[56 + 1] = '2'; // revert rook
                        boardCopy[57 + 1] = '0'; // revert king
                        boardCopy[59 + 1] = '6'; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                        boardCopy[59 + 1] = '0'; // revert king
                        boardCopy[58 + 1] = '6'; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                    }
                    list_append(self.moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
                } else if (move == MOVE_PIECE_CASTLE_BLACK) {
                    list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                    list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                    list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                    list_t *extraChecks = list_init();
                    if (self.dotSquares -> data[i].c == 6) {
                        /* black right castle */
                        boardCopy[7 + 1] = '2'; // revert rook
                        boardCopy[6 + 1] = '0'; // revert king
                        boardCopy[5 + 1] = '6'; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                    } else {
                        /* black left castle */
                        boardCopy[0 + 1] = '2'; // revert rook
                        boardCopy[1 + 1] = '0'; // revert king
                        boardCopy[3 + 1] = '6'; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                        boardCopy[3 + 1] = '0'; // revert king
                        boardCopy[2 + 1] = '6'; // king walk
                        list_append(extraChecks, (unitype) boardCopy, 's');
                    }
                    list_append(self.moves, (unitype) extraChecks, 'r'); // MOVES_EXTRA_CHECKS
                } else if (move == MOVE_PIECE_SUCCESSFUL) {
                    list_append(self.moves, (unitype) boardCopy, 's'); // MOVES_STRING
                    list_append(self.moves, (unitype) position, 'c'); // MOVES_FROM
                    list_append(self.moves, self.dotSquares -> data[i], 'c'); // MOVES_TO
                    list_append(self.moves, (unitype) NULL, 'l'); // MOVES_EXTRA_CHECKS
                } else {
                    printf("ERROR: movePiece returned MOVE_PIECE_ERROR\n");
                }
            }
        }
    }
    /* second pass - validate legal moves are legal (don't put king in check, etc) */
    for (int32_t movesIndex = 0; movesIndex < self.moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        for (int32_t position = 0; position < 64; position++) {
            generateNaiveMoves(self.moves -> data[movesIndex + MOVES_STRING].s + 1, position, !self.turn);
            for (int32_t i = 0; i < self.dotSquares -> length; i++) {
                char capturedPiece = self.moves -> data[movesIndex + MOVES_STRING].s[self.dotSquares -> data[i].c + 1];
                if (capturedPiece == '6' || capturedPiece == '9' || capturedPiece == 'F' || capturedPiece == 'I') {
                    /* this moves captures the king, invalidate the legal move */
                    for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                        list_delete(self.moves, movesIndex);
                    }
                    movesIndex -= MOVES_NUMBER_OF_FIELDS;
                    goto SECOND_PASS_NEXT;
                }
            }
        }
        /* extra moves to validate - castling cannot have the king "walk" through check */
        if (self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r != NULL) {
            for (int32_t extra = 0; extra < self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> length; extra++) {
                for (int32_t position = 0; position < 64; position++) {
                    generateNaiveMoves(self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s + 1, position, !self.turn);
                    for (int32_t i = 0; i < self.dotSquares -> length; i++) {
                        char capturedPiece = self.moves -> data[movesIndex + MOVES_EXTRA_CHECKS].r -> data[extra].s[self.dotSquares -> data[i].c + 1];
                        if (capturedPiece == '6' || capturedPiece == '9' || capturedPiece == 'F' || capturedPiece == 'I') {
                            /* this moves captures the king, invalidate the legal move */
                            for (int32_t j = 0; j < MOVES_NUMBER_OF_FIELDS; j++) {
                                list_delete(self.moves, movesIndex);
                            }
                            movesIndex -= MOVES_NUMBER_OF_FIELDS;
                            goto SECOND_PASS_NEXT;
                        }
                    }
                }
            }
        }
        SECOND_PASS_NEXT:
    }
    list_clear(self.dotSquares);
    printf("Number of moves: %d\n", self.moves -> length / MOVES_NUMBER_OF_FIELDS);
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("ERROR: Could not open file %s\n", filename);
        return;
    }
    char turnChar = 'w';
    if (self.turn == CHESS_BLACK) {
        turnChar = 'b';
    }
    fwrite(&turnChar, 1, 1, fp);
    fwrite(self.board, 1, 64, fp);
    fwrite("\n", 1, 1, fp);
    for (int32_t movesIndex = 0; movesIndex < self.moves -> length; movesIndex += MOVES_NUMBER_OF_FIELDS) {
        fwrite(self.moves -> data[movesIndex + MOVES_STRING].s, 1, 65, fp);
        fwrite("\n", 1, 1, fp);
    }
    fclose(fp);
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
                if (self.pawnPromotionWhite != -1) {
                    char promotionOptions[4] = {'5', '3', '2', '4'};
                    self.board[self.pawnPromotionWhite] = promotionOptions[self.pawnPromotionIndex];
                    self.pawnPromotionWhite = -1;
                } else {
                    char promotionOptions[4] = {'E', 'C', 'B', 'D'};
                    self.board[self.pawnPromotionBlack] = promotionOptions[self.pawnPromotionIndex];
                    self.pawnPromotionBlack = -1;
                }
                self.turn = !self.turn;
                return;
            }
            self.mousePiece = self.mouseSquare;
            if (self.mousePiece != -1 && list_find(self.dotSquares, (unitype) self.mousePiece, 'c') >= 0) {
                /* make move */
                int32_t status = movePiece(self.board, self.highlightedSquare[0], self.mousePiece);
                if (status == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
                    /* special: pawn promotion */
                    self.pawnPromotionWhite = self.mousePiece;
                } else if (status == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    self.pawnPromotionBlack = self.mousePiece;
                } else {
                    self.turn = !self.turn;
                }
                list_clear(self.dotSquares);
                self.highlightedSquare[1] = self.highlightedSquare[0];
                self.highlightedSquare[2] = self.mousePiece;
                self.highlightedSquare[0] = -1;
                self.mousePiece = -1;
            } else if (self.mousePiece == -1 || self.board[self.mousePiece] == '0') {
                list_clear(self.dotSquares);
                self.highlightedSquare[0] = -1;
                self.mousePiece = -1;
            } else {
                if (self.highlightedSquare[0] == self.mousePiece) {
                    self.highlightUnselect = 1;
                }
                self.highlightedSquare[0] = self.mousePiece;
                generateLegalMoves(self.board, self.highlightedSquare[0], self.turn);
            }
        } else {
            /* mouse held */
            if (self.mousePiece != -1) {
                self.highlightedSquareBox = self.mouseSquare;
            }
        }
    } else {
        if (self.keys[KEYS_LMB] == 1) {
            self.keys[KEYS_LMB] = 0;
            if (self.mousePiece == self.highlightedSquare[0] && list_find(self.dotSquares, (unitype) self.mouseSquare, 'c') >= 0) {
                /* make move */
                int32_t status = movePiece(self.board, self.highlightedSquare[0], self.mouseSquare);
                if (status == MOVE_PIECE_PAWN_PROMOTION_WHITE) {
                    /* special: pawn promotion */
                    self.pawnPromotionWhite = self.mouseSquare;
                } else if (status == MOVE_PIECE_PAWN_PROMOTION_BLACK) {
                    self.pawnPromotionBlack = self.mouseSquare;
                } else {
                    self.turn = !self.turn;
                }
                list_clear(self.dotSquares);
                self.highlightedSquare[1] = self.highlightedSquare[0];
                self.highlightedSquare[2] = self.mouseSquare;
                self.highlightedSquare[0] = -1;
            } else if (self.mouseSquare == self.mousePiece && self.highlightUnselect) {
                list_clear(self.dotSquares);
                self.highlightedSquare[0] = -1;
            }
            self.mousePiece = -1;
            self.highlightedSquareBox = -1;
            self.highlightUnselect = 0;
        }
    }
    if (turtleKeyPressed(GLFW_KEY_SPACE)) {
        if (self.keys[KEYS_SPACE] == 0) {
            self.keys[KEYS_SPACE] = 1;
            printBoard(self.board);
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
            list_clear(self.dotSquares);
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
