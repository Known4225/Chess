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
  - Threefold repetition
- Graphical updates
  - Speech bubbles
  - Color lock engine options
*/

#include "chess.h"
#include "turtle.h"
#include <time.h>

#define MUTEX_ACQUIRE(mutex) while (mutex) {asm("nop");} mutex = 1
#define MUTEX_RELEASE(mutex) mutex = 0

int32_t engineMove(char *engineName);

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
    SPEECH_BUBBLE_CENTER = 0,
    SPEECH_BUBBLE_BOTTOM_LEFT = 1,
    SPEECH_BUBBLE_BOTTOM_RIGHT = 2,
    SPEECH_BUBBLE_TOP_LEFT = 3,
    SPEECH_BUBBLE_TOP_RIGHT = 4,
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

    /* board */
    double boardX;
    double boardY;
    double boardSize;

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
    int8_t state; // BOARD_STATE_X
    tt_button_t *newGame; // new game button
    int32_t movesTotal;
    int32_t movesSinceCapture;

    /* engines */
    char inputFilename[4096];
    char outputFilename[4096];
    int8_t mohamedButtonEnabled;
    int8_t ryanButtonEnabled;
    int8_t mohamedButtonHover;
    int8_t ryanButtonHover;
    turtle_texture_t mohamedImage;
    turtle_texture_t ryanImage;
    int8_t speech;
    char speechContent[4096];
} chess_t;

chess_t self;

void init() {
    /* theme */
    self.theme = CHESS_THEME_CHESS_COM;

    /* board */
    self.boardX = -20;
    self.boardY = -5;
    self.boardSize = 160;

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
    self.state = BOARD_STATE_NONE;
    self.newGame = tt_buttonInit("New Game", NULL, self.boardX - 230, self.boardY - 55, 10);
    self.newGame -> color[TT_COLOR_SLOT_BUTTON] = TT_COLOR_BLACK_ALTERNATE;
    self.newGame -> color[TT_COLOR_SLOT_BUTTON_SELECT] = TT_COLOR_DARK_GREY;
    self.newGame -> color[TT_COLOR_SLOT_BUTTON_CLICKED] = TT_COLOR_BLACK_ALTERNATE;
    self.newGame -> color[TT_COLOR_SLOT_BUTTON_TEXT] = TT_COLOR_WHITE;
    self.newGame -> color[TT_COLOR_SLOT_BUTTON_SELECTED_TEXT] = TT_COLOR_WHITE;
    self.movesTotal = 0;
    self.movesSinceCapture = 0;

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


    /* engines */
    strcpy(self.inputFilename, osToolsFileDialog.executableFilepath);
    strcat(self.inputFilename, "input.txt");
    strcpy(self.outputFilename, osToolsFileDialog.executableFilepath);
    strcat(self.outputFilename, "output.txt");

    // self.mohamedButton = tt_buttonInit("Mohamed", NULL, self.boardX + self.boardSize + 30, self.boardY - 30, 10);
    // self.mohamedButton -> align = TT_BUTTON_ALIGN_LEFT;
    // self.ryanButton = tt_buttonInit("Ryan", NULL, self.boardX + self.boardSize + 100, self.boardY - 30, 10);
    // self.ryanButton -> align = TT_BUTTON_ALIGN_LEFT;

    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "images/anonymous.jpg");
    self.mohamedImage = turtleTextureLoad(constructedFilepath);
    strcpy(constructedFilepath, osToolsFileDialog.executableFilepath);
    strcat(constructedFilepath, "images/anonymous.jpg");
    self.ryanImage = turtleTextureLoad(constructedFilepath);
    free(constructedFilepath);
    self.speech = 0;
}

void setColor(int32_t color) {
    turtlePenColorAlpha(colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 0], colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 1], colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 2], colors[self.theme * CHESS_COLOR_NUMBER * 4 + color * 4 + 3]);
}

void turtleRoundedRectangle(double x1, double y1, double x2, double y2, double radius) {
    if (x1 > x2) {
        double temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        double temp = y1;
        y1 = y2;
        y2 = temp;
    }
    turtlePenSize(radius * 2);
    turtleGoto(x1 + radius, y1 + radius);
    turtlePenDown();
    turtleGoto(x1 + radius, y2 - radius);
    turtleGoto(x2 - radius, y2 - radius);
    turtleGoto(x2 - radius, y1 + radius);
    turtleGoto(x1 + radius, y1 + radius);
    turtlePenUp();
    turtleRectangle(x1 + radius, y1 + radius, x2 - radius, y2 - radius);
}

void speechBubble(char *text, double x, double y, double size, double originX, double originY, int8_t color, int8_t position) {
    char textCopy[4096];
    strcpy(textCopy, text);
    /* calculate width and height */
    double maxLength = 0;
    int32_t lines = 0;
    char *ptr = strtok(textCopy, "\n");
    while (ptr != NULL) {
        double textLength = turtleTextGetUnicodeLength(ptr, size);
        if (textLength > maxLength) {
            maxLength = textLength;
        }
        ptr = strtok(NULL, "\n");
        lines++;
    }
    maxLength += size / 1.3;
    if (color == CHESS_WHITE) {
        tt_setColor(TT_COLOR_WHITE);
        turtlePenColor(249, 249, 249);
    } else {
        tt_setColor(TT_COLOR_BLACK);
        // turtlePenColor(92, 89, 87);
    }
    double leading = size * 1.3;
    double centerX = x;
    double centerY = y;
    switch (position) {
        case SPEECH_BUBBLE_BOTTOM_LEFT:
            centerX += maxLength / 2;
            centerY += (lines - 1) / 2.0 * leading;
        break;
        case SPEECH_BUBBLE_BOTTOM_RIGHT:
            centerX -= maxLength / 2;
            centerY += (lines - 1) / 2.0 * leading;
        break;
        case SPEECH_BUBBLE_TOP_LEFT:
            centerX += maxLength / 2;
            centerY -= (lines - 1) / 2.0 * leading;
        break;
        case SPEECH_BUBBLE_TOP_RIGHT:
            centerX -= maxLength / 2;
            centerY -= (lines - 1) / 2.0 * leading;
        break;
        default:
        break;
    }
    double ypos = centerY + (lines - 1) / 2.0 * leading;
    double x1 = centerX - maxLength / 2;
    double y1 = ypos + leading / 1.3;
    double x2 = centerX + maxLength / 2;
    double y2 = centerY - (lines - 1) / 2.0 * leading - leading / 1.3;
    turtleRoundedRectangle(x1, y1, x2, y2, size / 1.5);
    double firstPointX = (centerY - originY) / 4 + centerX;
    double firstPointY = (originX - centerX) / 4 + centerY;
    double thirdPointX = (originY - centerY) / 4 + centerX;
    double thirdPointY = (centerX - originX) / 4 + centerY;
    /* clamp points */
    if (firstPointX < x1) {
        firstPointX = x1;
    }
    if (firstPointX > x2) {
        firstPointX = x2;
    }
    if (firstPointY > y1) {
        firstPointY = y1;
    }
    if (firstPointY < y2) {
        firstPointY = y2;
    }
    if (thirdPointX < x1) {
        thirdPointX = x1;
    }
    if (thirdPointX > x2) {
        thirdPointX = x2;
    }
    if (thirdPointY > y1) {
        thirdPointY = y1;
    }
    if (thirdPointY < y2) {
        thirdPointY = y2;
    }
    turtleTriangle(firstPointX, firstPointY, originX, originY, thirdPointX, thirdPointY);
    if (color == CHESS_WHITE) {
        tt_setColor(TT_COLOR_BLACK);
        // turtlePenColor(92, 89, 87);
    } else {
        tt_setColor(TT_COLOR_WHITE);
        // turtlePenColor(249, 249, 249);
    }
    int32_t index = 0;
    for (int32_t i = 0; i < lines; i++) {
        turtleTextWriteUnicode(textCopy + index, centerX, ypos, size, 50);
        index += strlen(textCopy + index) + 1;
        ypos -= leading;
    }
}

void newGame() {
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
    self.state = BOARD_STATE_NONE;
    self.movesTotal = 0;
    self.movesSinceCapture = 0;
    self.speech = 0;
}

/* import a file to board - TODO */
int32_t import(char *filename) {
    return -1;
}

/* export the board state - TODO */
int32_t export(char *filename) {
    return -1;
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
    setColor(CHESS_COLOR_BLACK_SQUARE);
    turtleRoundedRectangle(self.boardX - self.boardSize - 30, self.boardY + self.boardSize, self.boardX - self.boardSize - 110, self.boardY + self.boardSize - 20, 5);
    if (self.state == BOARD_STATE_CHECKMATE || self.state == BOARD_STATE_STALEMATE) {
        if (self.turn == CHESS_WHITE) {
            turtlePenColor(92, 89, 87);
            tt_setColor(TT_COLOR_BLACK);
        } else {
            turtlePenColor(249, 249, 249);
            tt_setColor(TT_COLOR_WHITE);
        }
        turtleTextWriteString("Game Over", self.boardX - self.boardSize - 70, self.boardY + self.boardSize - 10, 10, 50);
    } else {
        if (self.turn == CHESS_WHITE) {
            turtlePenColor(249, 249, 249);
            tt_setColor(TT_COLOR_WHITE);
            turtleTextWriteString("Turn: White", self.boardX - self.boardSize - 70, self.boardY + self.boardSize - 10, 10, 50);
        } else {
            turtlePenColor(92, 89, 87);
            tt_setColor(TT_COLOR_BLACK);
            turtleTextWriteString("Turn: Black", self.boardX - self.boardSize - 70, self.boardY + self.boardSize - 10, 10, 50);
        }
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
        self.mohamedButtonEnabled = 0;
        self.ryanButtonEnabled = 0;
    } else {
        self.mohamedButtonEnabled = 1;
        self.ryanButtonEnabled = 1;
    }
    if (self.state == BOARD_STATE_CHECKMATE || self.state == BOARD_STATE_STALEMATE) {
        self.mohamedButtonEnabled = 0;
        self.ryanButtonEnabled = 0;
        self.newGame -> enabled = TT_ELEMENT_ENABLED;
        if (self.state == BOARD_STATE_CHECKMATE) {
            setColor(CHESS_COLOR_BLACK_SQUARE);
            turtle.penr *= 0.8;
            turtle.peng *= 0.8;
            turtle.penb *= 0.8;
            double offsetX = -230;
            turtleRoundedRectangle(self.boardX - 50 + offsetX, self.boardY - 70, self.boardX + 50 + offsetX, self.boardY + 70, 5);
            if (self.turn == CHESS_BLACK) {
                tt_setColor(TT_COLOR_WHITE);
            } else {
                tt_setColor(TT_COLOR_BLACK);
            }
            turtleTextWriteString("Checkmate", self.boardX + offsetX, self.boardY + 50, 12, 50);
            if (self.turn == CHESS_BLACK) {
                turtleTexture(self.pieces[11], self.boardX - 12 + offsetX, self.boardY - 40, self.boardX + 8 + offsetX, self.boardY, 90);
                turtleTexture(self.pieces[5], self.boardX + 20 + offsetX, self.boardY - 24, self.boardX - 20 + offsetX, self.boardY + 16, 0);
                speechBubble(":(", self.boardX - 27 + offsetX, self.boardY - 8, 8, self.boardX - 20 + offsetX, self.boardY - 18, CHESS_BLACK, SPEECH_BUBBLE_CENTER);
                speechBubble("!", self.boardX + 15 + offsetX, self.boardY + 21, 8, self.boardX + 10 + offsetX, self.boardY + 9, CHESS_WHITE, SPEECH_BUBBLE_CENTER);
            } else {
                turtleTexture(self.pieces[5], self.boardX - 8 + offsetX, self.boardY - 40, self.boardX + 12 + offsetX, self.boardY, -90);
                turtleTexture(self.pieces[11], self.boardX + 20 + offsetX, self.boardY - 24, self.boardX - 20 + offsetX, self.boardY + 16, 0);
                speechBubble(":(", self.boardX + 27 + offsetX, self.boardY - 8, 8, self.boardX + 20 + offsetX, self.boardY - 18, CHESS_WHITE, SPEECH_BUBBLE_CENTER);
                speechBubble("!", self.boardX - 15 + offsetX, self.boardY + 21, 8, self.boardX - 10 + offsetX, self.boardY + 9, CHESS_BLACK, SPEECH_BUBBLE_CENTER);
            }
        } else if (self.state == BOARD_STATE_STALEMATE) {
            setColor(CHESS_COLOR_BLACK_SQUARE);
            turtle.penr *= 0.8;
            turtle.peng *= 0.8;
            turtle.penb *= 0.8;
            double offsetX = -230;
            turtleRoundedRectangle(self.boardX - 50 + offsetX, self.boardY - 70, self.boardX + 50 + offsetX, self.boardY + 70, 5);
            tt_setColor(TT_COLOR_WHITE);
            turtleTextWriteString("Stalemate", self.boardX + offsetX, self.boardY + 50, 12, 50);
            turtleTexture(self.pieces[11], self.boardX - 45 + offsetX, self.boardY - 30, self.boardX - 5 + offsetX, self.boardY + 10, 0);
            turtleTexture(self.pieces[5], self.boardX + 45 + offsetX, self.boardY - 30, self.boardX + 5 + offsetX, self.boardY + 10, 0);
            speechBubble("?", self.boardX - 35 + offsetX, self.boardY + 20, 8, self.boardX - 30 + offsetX, self.boardY + 8, CHESS_BLACK, SPEECH_BUBBLE_CENTER);
            speechBubble("?", self.boardX + 35 + offsetX, self.boardY + 20, 8, self.boardX + 30 + offsetX, self.boardY + 8, CHESS_WHITE, SPEECH_BUBBLE_CENTER);
        }
    } else {
        self.newGame -> enabled = TT_ELEMENT_HIDE;
    }
    if (self.state == BOARD_STATE_CHECK || self.state == BOARD_STATE_CHECKMATE) {
        if (self.turn == CHESS_WHITE) {
            tt_setColor(TT_COLOR_BLACK);
            turtlePenColor(92, 89, 87);
        } else {
            tt_setColor(TT_COLOR_WHITE);
            turtlePenColor(249, 249, 249);
        }
        double x1 = self.boardX - self.boardSize - 5;
        double y1 = self.boardY + self.boardSize;
        double x2 = self.boardX - self.boardSize - 25;
        double y2 = self.boardY + self.boardSize - 20;
        turtleRectangle(x1, y1, x2, y2);
        turtlePenSize(1);
        if (self.turn == CHESS_WHITE) {
            tt_setColor(TT_COLOR_WHITE);
            turtlePenColor(249, 249, 249);
        } else {
            tt_setColor(TT_COLOR_BLACK);
            turtlePenColor(92, 89, 87);
        }
        turtleTriangle(x1, y1, x2, y2, x1, y2);
    }
    if (self.newGame -> value) {
        self.newGame -> value = 0;
        newGame();
    }

    /* engine buttons */
    double buttonWidth = 72;
    double buttonY = self.boardY + 30;
    double mohamedX = self.boardX + self.boardSize + 10;
    double ryanX = mohamedX + 87;
    uint8_t greyOut = 255;
    if (turtle.mouseX > mohamedX && turtle.mouseX < mohamedX + buttonWidth && turtle.mouseY > buttonY - 12 && turtle.mouseY < buttonY + buttonWidth * 1.25 && self.mohamedButtonEnabled) {
        self.mohamedButtonHover = 1;
    } else {
        self.mohamedButtonHover = 0;
    }
    if (turtle.mouseX > ryanX && turtle.mouseX < ryanX + buttonWidth && turtle.mouseY > buttonY - 12 && turtle.mouseY < buttonY + buttonWidth * 1.25 && self.ryanButtonEnabled) {
        self.ryanButtonHover = 1;
    } else {
        self.ryanButtonHover = 0;
    }
    tt_setColor(TT_COLOR_COMPONENT);
    if (self.mohamedButtonEnabled) {
        if (self.mohamedButtonHover) {
            tt_setColor(TT_COLOR_COMPONENT_HIGHLIGHT);
        }
    } else {
        greyOut = 100;
    }
    turtleRoundedRectangle(mohamedX, buttonY - 12, mohamedX + buttonWidth, buttonY + 10, buttonWidth / 10);
    turtleTextureColor(self.mohamedImage, mohamedX, buttonY, mohamedX + buttonWidth, buttonY + buttonWidth * 1.25, 0, greyOut, greyOut, greyOut); // TODO - find a way to grey out button
    tt_setColor(TT_COLOR_TEXT);
    turtleTextWriteString("Mohamed", mohamedX + buttonWidth / 2, buttonY - 6, 6, 50);
    tt_setColor(TT_COLOR_COMPONENT);
    if (self.ryanButtonEnabled) {
        if (self.ryanButtonHover) {
            tt_setColor(TT_COLOR_COMPONENT_HIGHLIGHT);
        }
    } else {
        greyOut = 100;
    }
    turtleRoundedRectangle(ryanX, buttonY - 12, ryanX + buttonWidth, buttonY + 10, buttonWidth / 10);
    turtleTextureColor(self.ryanImage, ryanX, buttonY, ryanX + buttonWidth, buttonY + buttonWidth * 1.25, 0, greyOut, greyOut, greyOut); // TODO - find a way to grey out button
    tt_setColor(TT_COLOR_TEXT);
    turtleTextWriteString("Ryan", ryanX + buttonWidth / 2, buttonY - 6, 6, 50);
    if (self.speech == 1) {
        speechBubble(self.speechContent, mohamedX + buttonWidth / 2 + 10, buttonY + buttonWidth * 1.25 - 40, 6, mohamedX + buttonWidth / 2 + 5, buttonY + buttonWidth * 1.25 - 50, !self.turn, SPEECH_BUBBLE_BOTTOM_LEFT);
    } else if (self.speech == 2) {
        speechBubble(self.speechContent, ryanX + buttonWidth / 2 - 10, buttonY + buttonWidth * 1.25 - 40, 6, ryanX + buttonWidth / 2 - 5, buttonY + buttonWidth * 1.25 - 50, !self.turn, SPEECH_BUBBLE_BOTTOM_RIGHT);
    }
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
    fprintf(inputfp, "%d %d\n", self.movesTotal, self.movesSinceCapture);
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
    sprintf(command, "%s%s %s %s", osToolsFileDialog.executableFilepath, engineName, self.inputFilename, self.outputFilename); // idk why it needs to be double quoted
    // sprintf(command, "\"\"%s%s\" \"%s\" \"%s\"\"", osToolsFileDialog.executableFilepath, engineName, self.inputFilename, self.outputFilename); // idk why it needs to be double quoted
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
    self.movesTotal++;
    self.movesSinceCapture++;
    if (self.board[moves -> data[found + MOVES_TO].c] != BLANK_SPACE) {
        self.movesSinceCapture = 0;
    }
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
    self.state = checkBoardState(self.board, self.turn, self.movesSinceCapture);
    MUTEX_RELEASE(self.boardMutex);
    /* update speechContent */
    self.speechContent[0] = '\0';
    while (fgets(line, 1024, outputfp) != NULL) {
        strcat(self.speechContent, line);
    }
    fclose(outputfp);    
    return 0;
}

void mouse() {
    if (self.pawnPromotionWhite != -1 || self.pawnPromotionBlack != -1 || self.state == BOARD_STATE_CHECKMATE || self.state == BOARD_STATE_STALEMATE) {
        self.mouseSquare = -1;           
    }
    if (turtleMouseDown()) {
        if (self.keys[KEYS_LMB] == 0) {
            /* first tick */
            self.keys[KEYS_LMB] = 1;
            /* check for engine buttons */
            if (self.mohamedButtonHover) {
                int32_t status = engineMove("mohamed.exe");
                if (status != 0) {
                    sprintf(self.speechContent, "ERROR: returned %d\n", status);
                }
                self.speech = 1;
            }
            if (self.ryanButtonHover) {
                int32_t status = engineMove("ryan.exe");
                if (status != 0) {
                    sprintf(self.speechContent, "ERROR: returned %d\n", status);
                }
                self.speech = 2;

            }
            /* check for manual pawn promotion */
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
                self.state = checkBoardState(self.board, self.turn, self.movesSinceCapture);
                MUTEX_RELEASE(self.boardMutex);
                return;
            }
            /* check for board movement */
            self.mousePiece = self.mouseSquare;
            MUTEX_ACQUIRE(self.validMutex);
            MUTEX_ACQUIRE(self.boardMutex);
            if (self.mousePiece != -1 && list_find(self.valid, (unitype) self.mousePiece, 'c') >= 0) {
                /* make move */
                self.speech = 0;
                self.movesTotal++;
                self.movesSinceCapture++;
                if (self.board[self.mousePiece] != BLANK_SPACE) {
                    self.movesSinceCapture = 0;
                }
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
                    self.state = checkBoardState(self.board, self.turn, self.movesSinceCapture);
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
                self.speech = 0;
                self.movesTotal++;
                self.movesSinceCapture++;
                if (self.board[self.mouseSquare] != BLANK_SPACE) {
                    self.movesSinceCapture = 0;
                }
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
                    self.state = checkBoardState(self.board, self.turn, self.movesSinceCapture);
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
            newGame();
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
        turtleTextWriteStringf(310, -160, 5, 100, "Moves Total: %d", self.movesTotal);
        turtleTextWriteStringf(310, -170, 5, 100, "Moves since capture: %d", self.movesSinceCapture);
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
