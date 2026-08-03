/*
Created by Ryan Sricha 02.08.26

Piece reference:
0 - blank space   (0x30)
1 - white pawn    (0x31)
2 - white rook    (0x32)
3 - white knight  (0x33)
4 - white bishop  (0x34)
5 - white queen   (0x35)
6 - white king    (0x36)
A - black pawn    (0x41)
B - black rook    (0x42)
C - black knight  (0x43)
D - black bishop  (0x44)
E - black queen   (0x45)
F - black king    (0x46)

TODO:
- Generate legal moves
  - Challenges: en passant, castling
- Implement bot API
*/

#include "turtle.h"
#include <time.h>

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
    CHESS_COLOR_NUMBER = 7,
};

enum {
    KEYS_LMB = 0,
};

uint8_t colors[] = {
    238, 238, 238, 0,   // white square
    113, 134, 184, 0,   // black square
    245, 246, 130, 0,   // highlighted white square
    185, 202, 67, 0,    // highlighted black square
    252, 252, 211, 0,   // box highlight white square
    206, 218, 195, 0,   // box highlight black square
    60, 60, 60, 200,    // white square dot

    234, 237, 209, 0,   // white square
    118, 149, 86, 0,    // black square
    245, 246, 130, 0,   // highlighted white square
    185, 202, 67, 0,    // highlighted black square
    252, 252, 211, 0,   // box highlight white square
    206, 218, 195, 0,   // box highlight black square
    60, 60, 60, 200,    // white square dot
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
    int8_t whiteCastleEligible;
    int8_t blackCastleEligible;

    /* board */
    double boardX;
    double boardY;
    double boardSize;

} chess_t;

chess_t self;

void init() {
    self.theme = CHESS_THEME_CHESS_COM;

    /* chess */
    self.turn = CHESS_WHITE;
    memcpy(self.board, "BCDEFDCBAAAAAAAA000000000000500000000000000000001111111123456432", 64);
    self.mouseSquare = -1;
    self.mousePiece = -1;
    self.highlightedSquare[0] = -1;
    self.highlightedSquare[1] = -1;
    self.highlightedSquare[2] = -1;
    self.highlightedSquareBox = -1;
    self.highlightUnselect = 0;
    self.whiteCastleEligible = 1;
    self.blackCastleEligible = 1;
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
    self.boardX = 0;
    self.boardY = -25;
    self.boardSize = 160;
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

/* get piece texture given board code */
turtle_texture_t getPieceTexture(char code) {
    switch (code) {
        case '1':
        return self.pieces[0];
        case '2':
        return self.pieces[1];
        case '3':
        return self.pieces[2];
        case '4':
        return self.pieces[3];
        case '5':
        return self.pieces[4];
        case '6':
        return self.pieces[5];
        case 'A':
        return self.pieces[6];
        case 'B':
        return self.pieces[7];
        case 'C':
        return self.pieces[8];
        case 'D':
        return self.pieces[9];
        case 'E':
        return self.pieces[10];
        case 'F':
        return self.pieces[11];
        default:
        return -1;
    }
}

chess_piece_t getPieceType(char code) {
    switch (code) {
        case '1':
        case 'A':
        return CHESS_PIECE_PAWN;
        case '2':
        case 'B':
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
        case 'F':
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
        return CHESS_WHITE;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        return CHESS_BLACK;
        default:
        return -1;
    }
}

void render() {
    self.mouseSquare = -1;
    double xpos = self.boardX - self.boardSize;
    double ypos = self.boardY + self.boardSize;
    double shift = self.boardSize / 8;
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
            /* draw number */
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
            xpos = self.boardX - self.boardSize;
            ypos -= shift * 2;
            color = !color;
        }
    }
    /* render pieces */
    xpos = self.boardX - self.boardSize;
    ypos = self.boardY + self.boardSize;
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
            xpos = self.boardX - self.boardSize;
            ypos -= shift * 2;
        }
    }
    if (self.mousePiece != -1) {
        turtle_texture_t texture = getPieceTexture(self.board[self.mousePiece]);
        if (texture != -1) {
            turtleTexture(texture, turtle.mouseX - shift, turtle.mouseY - shift, turtle.mouseX + shift, turtle.mouseY + shift, 0);
        }
    }
}

static inline int8_t up(int8_t position) {
    if (position - 8 < 0) {
        return -1;
    }
    return position - 8;
}

static inline int8_t down(int8_t position) {
    if (position  + 8 >= 64) {
        return -1;
    }
    return position + 8;
}

static inline int8_t left(int8_t position) {
    if (position % 8 == 0) {
        return -1;
    }
    return position - 1;
}

static inline int8_t right(int8_t position) {
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

void generateLegalMoves(int8_t position) {
    if (position == -1) {
        return;
    }
    chess_piece_t type = getPieceType(self.board[position]);
    chess_color_t color = getPieceColor(self.board[position]);
    if (type == -1 || color == -1) {
        return;
    }
    list_clear(self.dotSquares);
    if (type == CHESS_PIECE_PAWN) {
        /* pawn */
        if (color == CHESS_WHITE) {

        } else {

        }
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
                if (self.board[wanderingPosition] != '0') {
                    chess_color_t wanderingColor = getPieceColor(self.board[wanderingPosition]);
                    if (wanderingColor != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
            }
        }
    } else if (type == CHESS_PIECE_KNIGHT) {

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
                if (self.board[wanderingPosition] != '0') {
                    chess_color_t wanderingColor = getPieceColor(self.board[wanderingPosition]);
                    if (wanderingColor != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
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
                if (self.board[wanderingPosition] != '0') {
                    chess_color_t wanderingColor = getPieceColor(self.board[wanderingPosition]);
                    if (wanderingColor != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                    }
                    break;
                }
                list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
            }
        }
    } else if (type == CHESS_PIECE_KING) {
        int8_t (*direction[8]) (int8_t position) = {
            up, upRight, right, downRight, down, downLeft, left, upLeft,
        };
        for (int32_t j = 0; j < 8; j++) {
            int8_t wanderingPosition = position;
            wanderingPosition = direction[j](wanderingPosition);
            if (wanderingPosition != -1) {
                if (self.board[wanderingPosition] != '0') {
                    chess_color_t wanderingColor = getPieceColor(self.board[wanderingPosition]);
                    if (wanderingColor != color) {
                        list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                    }
                } else {
                    list_append(self.dotSquares, (unitype) wanderingPosition, 'c');
                }
            }
        }
    }
}

void mouse() {
    if (turtleMouseDown()) {
        if (self.keys[KEYS_LMB] == 0) {
            /* first tick */
            self.keys[KEYS_LMB] = 1;
            self.mousePiece = self.mouseSquare;
            if (self.mousePiece == -1 || self.board[self.mousePiece] == '0') {
                list_clear(self.dotSquares);
                self.highlightedSquare[0] = -1;
                self.mousePiece = -1;
            } else {
                if (self.highlightedSquare[0] == self.mousePiece) {
                    self.highlightUnselect = 1;
                }
                self.highlightedSquare[0] = self.mousePiece;
                generateLegalMoves(self.highlightedSquare[0]);
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
            if (self.mouseSquare == self.mousePiece && self.highlightUnselect) {
                list_clear(self.dotSquares);
                self.highlightedSquare[0] = -1;
            }
            self.mousePiece = -1;
            self.highlightedSquareBox = -1;
            self.highlightUnselect = 0;
        }
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
            printf("New\n");
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
