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
*/

#include "turtle.h"
#include <time.h>

enum {
    CHESS_THEME_DEFAULT = 0,
    CHESS_THEME_CHESS_COM = 1,
};

enum {
    KEYS_LMB = 0,
};

enum {
    TURN_WHITE = 0,
    TURN_BLACK = 1,
};

uint8_t colors[] = {


    234, 237, 209, // white squares
    118, 149, 86, // black squares
};

typedef struct {
    int8_t theme;
    int8_t keys[8];

    /* chess */
    int8_t turn;
    char board[64];
    turtle_texture_t pieces[12];

    /* board */
    double boardX;
    double boardY;
    double boardSize;

} chess_t;

chess_t self;

void init() {
    self.theme = CHESS_THEME_DEFAULT;

    /* chess */
    self.turn = TURN_WHITE;
    memcpy(self.board, "BCDEFDCBAAAAAAAA000000000000000000000000000000001111111123456432", 64);

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

/* import a file to board */
int32_t import(char *filename) {

}

/* export the board state */
int32_t export(char *filename) {

}

/* get piece texture given board code */
turtle_texture_t getPiece(char code) {
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

void render() {
    double xpos = self.boardX - self.boardSize;
    double ypos = self.boardY + self.boardSize;
    double shift = self.boardSize / 8;
    /* render board */
    int32_t color = 1;
    for (int32_t i = 0; i < 64; i++) {
        if (color) {
            tt_setColor(TT_COLOR_WHITE);
        } else {
            tt_setColor(TT_COLOR_BLACK);
        }
        color = !color;
        turtleRectangle(xpos - shift, ypos - shift, xpos + shift, ypos + shift);
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
        if (self.board[i] != 0) {
            turtle_texture_t texture = getPiece(self.board[i]);
            if (texture != -1) {
                turtleTexture(texture, xpos - shift, ypos - shift, xpos + shift, ypos + shift, 0);
            }
        }
        xpos += shift * 2;
        if (i % 8 == 7) {
            xpos = self.boardX - self.boardSize;
            ypos -= shift * 2;
        }
    }
}

void mouse() {
    if (turtleMouseDown()) {
        if (self.keys[KEYS_LMB] == 0) {
            /* first tick */
            self.keys[KEYS_LMB] = 1;
        } else {
            /* mouse held */
        }
    } else {
        if (self.keys[KEYS_LMB] == 1) {
            self.keys[KEYS_LMB] = 0;
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
    GLFWwindow *window = turtleCreateWindowIcon(TURTLE_WINDOW_DEFAULT_WIDTH, TURTLE_WINDOW_DEFAULT_HEIGHT, "turtle demo", "images/thumbnail.png");
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
