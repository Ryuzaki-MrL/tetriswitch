#include <string.h>
#ifdef SWITCH
    #include <switch.h>
#else
    #include <stdio.h>
    #include "switchdefs.h"
#endif

#include "draw.h"
#include "tetris.h"

#define TETROMINO_SIZE  36
#define NEXT_COUNT      3

#define GRID_XOFFSET    360
#define GRID_YOFFSET    0

#define DEFAULT_TICKS   50
#define LEVEL_TICKS     (DEFAULT_TICKS - (5 * level))

enum {
    TETRO_I, TETRO_J, TETRO_L, TETRO_O,
    TETRO_S, TETRO_T, TETRO_Z
};

typedef struct {
    u32 color;
    u8 width;
    u8 height;
    u8 blockdata[5*5*4];
} TetrominoData;

typedef struct {
    u8 type;
    s8 x;
    s8 y;
    u8 rot;
    u8 ghost;
} Tetromino;

typedef struct {
    s8 x;
    s8 y;
} Pair;

static u8 tetrisgrid[10][22];
static u32 gridcolor;
static int lines;
static int score;
static int highscore;
static int level;
static int gravityticks;
static int glueticks;
static int lineclearticks;
static Tetromino current;
static Tetromino onhold;
static Tetromino ghost;
static u8 currentbag[] = { 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6 };
static u8 currentbagpos;

static const Pair rotationdataJLSTZ[4][5] = {
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}}, // 0
    {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}}, // R
    {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}}, // 2
    {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}}  // L
};

static const Pair rotationdataI[4][5] = {
    {{ 0, 0}, {-1, 0}, { 2, 0}, {-1, 0}, { 2, 0}}, // 0
    {{-1, 0}, { 0, 0}, { 0, 0}, { 0,-1}, { 0, 2}}, // R
    {{-1,-1}, { 1,-1}, {-2, 1}, { 1, 0}, {-2, 0}}, // 2
    {{ 0,-1}, { 0,-1}, { 0,-1}, { 0, 1}, { 0,-2}}  // L
};

static const TetrominoData tetrodata[7] = {
    { // I
        .color = RGBA8_MAXALPHA(0x00,0xFF,0xFF), .width = 5, .height = 5,
        {
            0, 0, 0, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,
            0, 1, 1, 1, 1,/**/ 0, 0, 1, 0, 0,/**/ 1, 1, 1, 1, 0,/**/ 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 0, 0, 0
        }
    }, { // J
        .color = RGBA8_MAXALPHA(0x00,0x00,0xFF), .width = 3, .height = 3,
        {
            2, 0, 0,/**/ 0, 2, 2,/**/ 0, 0, 0,/**/ 0, 2, 0,
            2, 2, 2,/**/ 0, 2, 0,/**/ 2, 2, 2,/**/ 0, 2, 0,
            0, 0, 0,/**/ 0, 2, 0,/**/ 0, 0, 2,/**/ 2, 2, 0
        }
    }, { // L
        .color = RGBA8_MAXALPHA(0xFF,0x7F,0x00), .width = 3, .height = 3,
        {
            0, 0, 3,/**/ 0, 3, 0,/**/ 0, 0, 0,/**/ 3, 3, 0,
            3, 3, 3,/**/ 0, 3, 0,/**/ 3, 3, 3,/**/ 0, 3, 0,
            0, 0, 0,/**/ 0, 3, 3,/**/ 3, 0, 0,/**/ 0, 3, 0
        }
    }, { // O
        .color = RGBA8_MAXALPHA(0xFF,0xFF,0x00), .width = 2, .height = 2,
        {
            4, 4,/**/ 4, 4,/**/ 4, 4,/**/ 4, 4,
            4, 4,/**/ 4, 4,/**/ 4, 4,/**/ 4, 4
        }
    }, { // S
        .color = RGBA8_MAXALPHA(0x00,0xFF,0x00), .width = 3, .height = 3,
        {
            0, 5, 5,/**/ 0, 5, 0,/**/ 0, 0, 0,/**/ 5, 0, 0,
            5, 5, 0,/**/ 0, 5, 5,/**/ 0, 5, 5,/**/ 5, 5, 0,
            0, 0, 0,/**/ 0, 0, 5,/**/ 5, 5, 0,/**/ 0, 5, 0
        }
    }, { // T
        .color = RGBA8_MAXALPHA(0xFF,0x00,0xFF), .width = 3, .height = 3,
        {
            0, 6, 0,/**/ 0, 6, 0,/**/ 0, 0, 0,/**/ 0, 6, 0,
            6, 6, 6,/**/ 0, 6, 6,/**/ 6, 6, 6,/**/ 6, 6, 0,
            0, 0, 0,/**/ 0, 6, 0,/**/ 0, 6, 0,/**/ 0, 6, 0
        }
    }, { // Z
        .color = RGBA8_MAXALPHA(0xFF,0x00,0x00), .width = 3, .height = 3,
        {
            7, 7, 0,/**/ 0, 0, 7,/**/ 0, 0, 0,/**/ 0, 7, 0,
            0, 7, 7,/**/ 0, 7, 7,/**/ 7, 7, 0,/**/ 7, 7, 0,
            0, 0, 0,/**/ 0, 7, 0,/**/ 0, 7, 7,/**/ 7, 0, 0
        }
    }
};

static int checkCollision(Tetromino* t, s8 x, s8 y) {
    u32 i, j, w, h, r;
    w = tetrodata[t->type].width;
    h = tetrodata[t->type].height;
    r = t->rot * w;
    for (i = 0; i < w; ++i) {
        for (j = 0; j < h; ++j) {
            if  (tetrodata[t->type].blockdata[j * w * 4 + r + i] &&
                ((x + i < 0) || (x + i >= 10) || (y + j >= 22) || tetrisgrid[x + i][y + j])
            ) {
                return 1;
            }
        }
    }
    return 0;
}

static void generateBag(u8* bag) {
    int i;
    for (i = 6; i > 0; --i) {
        u64 r = randomGet64() % (i+1);
        u8 tmp = bag[i];
        bag[i] = bag[r];
        bag[r] = tmp;
    }
}

static void generateGhost(Tetromino* t) {
    ghost = *t;
    while(!checkCollision(&ghost, ghost.x, ghost.y + 1)) {
        ++ghost.y;
    }
    ghost.ghost = 1; // duh... :p
}

static void nextTetromino() {
    current.type = currentbag[currentbagpos++];
    if (currentbagpos > 13) {
        currentbagpos = 0;
    }
    if ((currentbagpos % 7) == 0) {
        generateBag(currentbag + ((currentbagpos + 7) % 14));
    }
    current.x = 3;
    current.y = 0;
    current.rot = current.ghost = 0;
    if (checkCollision(&current, current.x, current.y)) {
        //doGameOver();
    } else {
        generateGhost(&current);
    }
}

static void doGlue() {
    u32 i, j, w, h, r;
    w = tetrodata[current.type].width;
    h = tetrodata[current.type].height;
    r = current.rot * w;
    for (i = 0; i < w; ++i) {
        for (j = 0; j < h; ++j) {
            u32 offs = j * w * 4 + r + i;
            if (tetrodata[current.type].blockdata[offs]) {
                tetrisgrid[current.x + i][current.y + j] = tetrodata[current.type].blockdata[offs];
            }
        }
    }
    glueticks = 0;
    gravityticks = LEVEL_TICKS;
    nextTetromino();
    // TODO: check lines, add score
}

static void doHardDrop() {
    while(!checkCollision(&current, current.x, current.y + 1)) {
        ++current.y;
    }
    doGlue();
}

static void doGravity() {
    if (!checkCollision(&current, current.x, current.y + 1)) {
        ++current.y;
        gravityticks = LEVEL_TICKS;
    } else {
        if ((++glueticks) >= LEVEL_TICKS) {
            doGlue();
        }
    }
}

static void doRotate(int amt) {
    u8 tmprot = current.rot;
    current.rot = (current.rot + amt + 4) % 4;
    if (current.type == TETRO_O) {
        glueticks = 0;
        return;
    }
    const Pair (*rotdata)[5] = (current.type == TETRO_I) ? rotationdataI : rotationdataJLSTZ;
    int i;
    for (i = 0; i < 5; ++i) {
        s8 tmpx = current.x + rotdata[tmprot][i].x - rotdata[current.rot][i].x;
        s8 tmpy = current.y + rotdata[tmprot][i].y - rotdata[current.rot][i].y;
        if (!checkCollision(&current, tmpx, tmpy)) {
            current.x = (u8)tmpx;
            current.y = (u8)tmpy;
            glueticks = 0;
            generateGhost(&current);
            return;
        }
    }
    current.rot = tmprot;
}

static void doMove(int amt) {
    if (!checkCollision(&current, current.x + amt, current.y)) {
        current.x += amt;
        generateGhost(&current);
        glueticks = 0;
    }
}

static void renderGrid() {
    int i, j;
    for (i = 0; i < 10; ++i) {
        for (j = 0; j < 20; ++j) {
            int x = GRID_XOFFSET + TETROMINO_SIZE * i;
            int y = GRID_YOFFSET + TETROMINO_SIZE * j;
            if (tetrisgrid[i][j + 2]) {
                drawFillRect(x, y, x + TETROMINO_SIZE, y + TETROMINO_SIZE, tetrodata[tetrisgrid[i][j + 2] - 1].color);
                drawRectangle(x, y, x + TETROMINO_SIZE, y + TETROMINO_SIZE, RGBA8_MAXALPHA(0,0,0));
            } else {
                drawFillRect(x + 1, y + 1, x + TETROMINO_SIZE, y + TETROMINO_SIZE, gridcolor);
            }
        }
    }
}

static void renderBlockData(int x, int y, u8 type, u8 rot, u8 ghost) {
    u32 i, j, w, h, r, xx, yy, col;
    w = tetrodata[type].width;
    h = tetrodata[type].height;
    r = rot * w;
    for (i = 0; i < w; ++i) {
        for (j = 0; j < h; ++j) {
            if (tetrodata[type].blockdata[j * w * 4 + r + i]) {
                xx = x + i * TETROMINO_SIZE;
                yy = y + j * TETROMINO_SIZE;
                col = ghost ? (tetrodata[type].color & 0x7FFFFFFF) : tetrodata[type].color;
                drawFillRect(xx, yy, xx + TETROMINO_SIZE, yy + TETROMINO_SIZE, col);
                drawRectangle(xx, yy, xx + TETROMINO_SIZE, yy + TETROMINO_SIZE, RGBA8_MAXALPHA(0,0,0));
            }
        }
    }
}

static void renderTetromino(Tetromino* t) {
    u32 x, y;
    x = GRID_XOFFSET + (t->x * TETROMINO_SIZE);
    y = GRID_YOFFSET + ((t->y - 2) * TETROMINO_SIZE);
    renderBlockData(x, y, t->type, t->rot, t->ghost);
}

void tetrisInit() {
    lines = score = 0;
    memset(tetrisgrid, 0, sizeof(tetrisgrid));
    gridcolor = RGBA8_MAXALPHA(0xC0, 0xC0, 0xC0);
    currentbagpos = 0;
    level = 1;
    gravityticks = LEVEL_TICKS;
    generateBag(currentbag);
    nextTetromino();
}

void update() {
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    // Quit the game (TODO: pause + select to quit)
    if (kDown & KEY_PLUS) {
        running = 0;
        return;
    }
    // Switch between dark and light grid
    if (kDown & KEY_X) {
        gridcolor ^= 0xE0E0E0;
    }

    // Wait for line clear animation to end
    if (lineclearticks > 0) {
        --lineclearticks;
        return;
    }

    // Rotate
    if (kDown & KEY_A) {
        doRotate(1);
    } else if (kDown & KEY_B) {
        doRotate(-1);
    }
    // Move
    if (kDown & KEY_RIGHT) {
        doMove(1);
    } else if (kDown & KEY_LEFT) {
        doMove(-1);
    }
    // Soft drop
    if (kHeld & KEY_DOWN) {
        gravityticks -= LEVEL_TICKS / 2;
    }
    // Hard drop
    if (kDown & KEY_UP) {
        doHardDrop();
    }

    if ((--gravityticks) <= 0) {
        doGravity();
    }
}

void render() {
    drawStart();
    renderGrid();
    renderTetromino(&ghost);
    renderTetromino(&current);
    drawEnd();
}
