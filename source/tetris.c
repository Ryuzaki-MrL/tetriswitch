#include <stdio.h>
#include <string.h>

#ifdef SWITCH
    #include <switch.h>
#else
    #include "switchdefs.h"
#endif

#include "draw.h"
#include "tetris.h"

#define TETROMINO_SIZE  36
#define NEXT_COUNT      3

#define GRID_XOFFSET    260
#define GRID_YOFFSET    0

#define TEXT_COLOR      RGBA8_MAXALPHA(255,255,255)
#define BG_COLOR        RGBA8_MAXALPHA(0,0,0x4F)

#define GRAVITY_TICKS   54
#define LEVEL_TICKS     ((level < 11) ? (GRAVITY_TICKS - (4 * level)) : 10)
#define LINECLEARTICKS  30
#define GLUE_TICKS      (GRAVITY_TICKS >> 1)

enum {
    TETRO_I, TETRO_J, TETRO_L, TETRO_O,
    TETRO_S, TETRO_T, TETRO_Z
};

typedef struct {
    u32 color;
    u8 width;
    u8 height;
    s8 offs;
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

static u8 tetrisgrid[22][10];
static u8 linefull[22];
static u32 gridcolor = 0xFFC0C0C0;
static u32 lines;
static u32 score;
static u32 highscore;
static u8 level;
static int gravityticks;
static int glueticks;
static int lineclearticks;
static u8 pause;
static u8 gameover;
static u8 didswap;
static u8 softdrop;
static Tetromino current;
static Tetromino onhold = { 0, 3, 0, 0, 1 };
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
        .color = RGBA8_MAXALPHA(0x00,0xFF,0xFF), .width = 5, .height = 5, .offs = -18,
        {
            0, 0, 0, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,
            0, 1, 1, 1, 1,/**/ 0, 0, 1, 0, 0,/**/ 1, 1, 1, 1, 0,/**/ 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0,/**/ 0, 0, 1, 0, 0,/**/ 0, 0, 0, 0, 0,/**/ 0, 0, 0, 0, 0
        }
    }, { // J
        .color = RGBA8_MAXALPHA(0x00,0x00,0xFF), .width = 3, .height = 3, .offs = 36,
        {
            2, 0, 0,/**/ 0, 2, 2,/**/ 0, 0, 0,/**/ 0, 2, 0,
            2, 2, 2,/**/ 0, 2, 0,/**/ 2, 2, 2,/**/ 0, 2, 0,
            0, 0, 0,/**/ 0, 2, 0,/**/ 0, 0, 2,/**/ 2, 2, 0
        }
    }, { // L
        .color = RGBA8_MAXALPHA(0xFF,0x7F,0x00), .width = 3, .height = 3, .offs = 36,
        {
            0, 0, 3,/**/ 0, 3, 0,/**/ 0, 0, 0,/**/ 3, 3, 0,
            3, 3, 3,/**/ 0, 3, 0,/**/ 3, 3, 3,/**/ 0, 3, 0,
            0, 0, 0,/**/ 0, 3, 3,/**/ 3, 0, 0,/**/ 0, 3, 0
        }
    }, { // O
        .color = RGBA8_MAXALPHA(0xFF,0xFF,0x00), .width = 2, .height = 2, .offs = 54,
        {
            4, 4,/**/ 4, 4,/**/ 4, 4,/**/ 4, 4,
            4, 4,/**/ 4, 4,/**/ 4, 4,/**/ 4, 4
        }
    }, { // S
        .color = RGBA8_MAXALPHA(0x00,0xFF,0x00), .width = 3, .height = 3, .offs = 36,
        {
            0, 5, 5,/**/ 0, 5, 0,/**/ 0, 0, 0,/**/ 5, 0, 0,
            5, 5, 0,/**/ 0, 5, 5,/**/ 0, 5, 5,/**/ 5, 5, 0,
            0, 0, 0,/**/ 0, 0, 5,/**/ 5, 5, 0,/**/ 0, 5, 0
        }
    }, { // T
        .color = RGBA8_MAXALPHA(0xFF,0x00,0xFF), .width = 3, .height = 3, .offs = 36,
        {
            0, 6, 0,/**/ 0, 6, 0,/**/ 0, 0, 0,/**/ 0, 6, 0,
            6, 6, 6,/**/ 0, 6, 6,/**/ 6, 6, 6,/**/ 6, 6, 0,
            0, 0, 0,/**/ 0, 6, 0,/**/ 0, 6, 0,/**/ 0, 6, 0
        }
    }, { // Z
        .color = RGBA8_MAXALPHA(0xFF,0x00,0x00), .width = 3, .height = 3, .offs = 36,
        {
            7, 7, 0,/**/ 0, 0, 7,/**/ 0, 0, 0,/**/ 0, 7, 0,
            0, 7, 7,/**/ 0, 7, 7,/**/ 7, 7, 0,/**/ 7, 7, 0,
            0, 0, 0,/**/ 0, 7, 0,/**/ 0, 7, 7,/**/ 7, 0, 0
        }
    }
};

static const int scoretable[] = {
    0, 100, 300, 500, 800
};

static void saveHighscore() {
    FILE* f = fopen("highscore.bin", "wb");
    fwrite(&highscore, sizeof(highscore), 1, f);
    fclose(f);
}

static void loadHighscore() {
    FILE* f = fopen("highscore.bin", "rb");
    fread(&highscore, sizeof(highscore), 1, f);
    fclose(f);
}

static int checkCollision(Tetromino* t, s8 x, s8 y) {
    u32 i, j, w, h, r;
    w = tetrodata[t->type].width;
    h = tetrodata[t->type].height;
    r = t->rot * w;
    for (i = 0; i < w; ++i) {
        for (j = 0; j < h; ++j) {
            if  (tetrodata[t->type].blockdata[j * w * 4 + r + i] &&
                ((x + i < 0) || (x + i >= 10) || (y + j >= 22) || (y + j < 0) || tetrisgrid[y + j][x + i])
            ) {
                return 1;
            }
        }
    }
    return 0;
}

static int checkLines(u8 start, u8 end) {
    u8 i, j, ln = 0;
    for (j = start; j < end && j < 22; ++j) {
        for (i = 0; i < 10; ++i) {
            if (!tetrisgrid[j][i]) break;
        }
        if (i == 10) {
            ++ln;
            linefull[j] = 1;
        }
    }
    return ln;
}

static void addScore(int value) {
    score += value;
    if (score > 99999999) {
        score = 99999999;
    }
    if (score > highscore) {
        highscore = score;
    }
}

static void doGameOver() {
    pause = gameover = 1;
    saveHighscore();
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
        doGameOver();
    } else {
        generateGhost(&current);
    }
    didswap = 0;
}

static void clearLines() {
    u8 i, j, k;
    for (j = 0; j < 22; ++j) {
        // If the line is full
        if (linefull[j]) {
            // Move everything down one level
            for (i = j; i > 0; --i) {
                memcpy(&tetrisgrid[i][0], &tetrisgrid[i-1][0], sizeof(tetrisgrid[i]));
            }
            // Clear the top row
            for (k = 0; k < 10; ++k) {
                tetrisgrid[i][k] = 0;
            }
            linefull[j] = 0;
        }
    }
    level = ((lines / 10) + 1);
    nextTetromino();
}

static void doGlue() {
    u32 i, j, w, h, r;
    w = tetrodata[current.type].width;
    h = tetrodata[current.type].height;
    r = current.rot * w;
    // Copy block data to grid
    for (j = 0; j < h; ++j) {
        for (i = 0; i < w; ++i) {
            u32 offs = j * w * 4 + r + i;
            if (tetrodata[current.type].blockdata[offs]) {
                tetrisgrid[current.y + j][current.x + i] = tetrodata[current.type].blockdata[offs];
            }
        }
    }
    u8 nclear = checkLines(current.y, current.y + h);
    if (nclear) {
        addScore(scoretable[nclear] * level);
        lines += nclear;
        lineclearticks = LINECLEARTICKS;
    } else {
        nextTetromino();
    }
    glueticks = 0;
    gravityticks = LEVEL_TICKS;
}

static void doHardDrop() {
    u8 tmpy = current.y;
    while(!checkCollision(&current, current.x, current.y + 1)) {
        ++current.y;
    }
    addScore((current.y - tmpy) * 2);
    doGlue();
}

static void doGravity() {
    if (!checkCollision(&current, current.x, current.y + 1)) {
        ++current.y;
        gravityticks = LEVEL_TICKS;
        addScore(softdrop); // +1 per row when soft dropping
    } else {
        if ((++glueticks) >= GLUE_TICKS) {
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
            current.x = tmpx;
            current.y = tmpy;
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

void doHold() {
    if (!didswap) {
        Tetromino t = onhold;
        onhold.type = current.type;
        if (t.ghost) {
            onhold.ghost = 0;
            nextTetromino();
        } else {
            current = t;
            generateGhost(&current);
        }
        didswap = 1;
    }
}

void tetrisInit() {
    lines = score = 0;
    loadHighscore();
    memset(tetrisgrid, 0, sizeof(tetrisgrid));
    memset(linefull, 0, sizeof(linefull));
    currentbagpos = 0;
    level = 1;
    pause = gameover = 0;
    gravityticks = LEVEL_TICKS;
    lineclearticks = 0;
    didswap = 0;
    onhold.ghost = 1;
    generateBag(currentbag);
    nextTetromino();
}

void update() {
    u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
    u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    // Pause the game
    if (kDown & KEY_PLUS) {
        pause ^= 1;
        if (gameover) {
            tetrisInit();
        }
    }
    // Switch between dark and light grid
    if (kDown & KEY_X) {
        gridcolor ^= 0xE0E0E0;
    }

    if (pause) {
        // Quit the game
        if (kDown & KEY_MINUS) {
            running = 0;
        }
        return;
    }

    // Wait for line clear animation to end
    if (lineclearticks > 0) {
        if ((--lineclearticks) == 0) {
            clearLines();
        }
        return;
    }

    // Hold
    if (kDown & (KEY_L | KEY_R)) {
        doHold();
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
    // Hard drop
    if (kDown & KEY_UP) {
        doHardDrop();
    }

    // Soft drop and gravity
    softdrop = ((kHeld & KEY_DOWN) > 0);
    gravityticks -= 1 + ((LEVEL_TICKS / 2) * softdrop);
    if (gravityticks <= 0) {
        doGravity();
    }
}

static void renderGrid() {
    int i, j;
    for (j = 0; j < 20; ++j) {
        int y = GRID_YOFFSET + TETROMINO_SIZE * j;
        if (((lineclearticks & 7) < 4) && linefull[j + 2]) {
            drawFillRect(GRID_XOFFSET, y, GRID_XOFFSET + 360, y + TETROMINO_SIZE, gridcolor);
        } else for (i = 0; i < 10; ++i) {
            int x = GRID_XOFFSET + TETROMINO_SIZE * i;
            if (tetrisgrid[j + 2][i]) {
                drawFillRect(x, y, x + TETROMINO_SIZE, y + TETROMINO_SIZE, tetrodata[tetrisgrid[j + 2][i] - 1].color);
                drawRectangle(x, y, x + TETROMINO_SIZE, y + TETROMINO_SIZE, RGBA8_MAXALPHA(0,0,0));
            } else {
                drawFillRect(x + (i > 0), y + 1, x + TETROMINO_SIZE, y + TETROMINO_SIZE, gridcolor);
            }
        }
    }
}

static void renderBlockData(int x, int y, u8 type, u8 rot, u8 ghost) {
    u32 i, j, w, h, r, xx, yy, outline;
    w = tetrodata[type].width;
    h = tetrodata[type].height;
    r = rot * w;
    for (i = 0; i < w; ++i) {
        for (j = 0; j < h; ++j) {
            if (tetrodata[type].blockdata[j * w * 4 + r + i]) {
                xx = x + i * TETROMINO_SIZE;
                yy = y + j * TETROMINO_SIZE;
                if (ghost) {
                    outline = tetrodata[type].color;
                } else {
                    drawFillRect(xx, yy, xx + TETROMINO_SIZE, yy + TETROMINO_SIZE, tetrodata[type].color);
                    outline = RGBA8_MAXALPHA(0,0,0);
                }
                drawRectangle(xx, yy, xx + TETROMINO_SIZE, yy + TETROMINO_SIZE, outline);
            }
        }
    }
}

static void renderTetromino(Tetromino* t) {
    int x, y;
    x = GRID_XOFFSET + (t->x * TETROMINO_SIZE);
    y = GRID_YOFFSET + ((t->y - 2) * TETROMINO_SIZE);
    renderBlockData(x, y, t->type, t->rot, t->ghost);
}

void render() {
    drawStart();
    renderGrid();
    if (!lineclearticks) {
        renderTetromino(&ghost);
        renderTetromino(&current);
    }
    if (pause) {
        drawFillRect(GRID_XOFFSET + 1, 320, GRID_XOFFSET + 360, 360, RGBA8_MAXALPHA(255,255,255));
        if (gameover) {
            drawText(tahoma24, GRID_XOFFSET + 90, 320, RGBA8_MAXALPHA(0,0,0), "GAME OVER");
        } else {
            drawText(tahoma24, GRID_XOFFSET + 132, 320, RGBA8_MAXALPHA(0,0,0), "PAUSE");
        }
    }


    drawFillRect(0, 0, GRID_XOFFSET - 1, 720, BG_COLOR);
    drawFillRect(GRID_XOFFSET + 361, 0, 1280, 720, BG_COLOR);
    drawTextFormat(tahoma24, GRID_XOFFSET + 400, 0, TEXT_COLOR, "Score: %08u    Highscore: %08u", score, highscore);
    drawTextFormat(tahoma24, 40, 0, TEXT_COLOR, "Level: %u\nLines: %u", level, lines);

    drawText(tahoma24, 40, 80, TEXT_COLOR, "HOLD");
    drawFillRect(40, 120, 40 + TETROMINO_SIZE*5, 120 + TETROMINO_SIZE*5, gridcolor);
    if (!onhold.ghost) {
        renderBlockData(40 + tetrodata[onhold.type].offs, 120 + tetrodata[onhold.type].offs, onhold.type, 0, 0);
    }

    drawText(tahoma24, GRID_XOFFSET + 400, 80, TEXT_COLOR, "NEXT");
    u8 i;
    for (i = 0; i < NEXT_COUNT; ++i) {
        int x = GRID_XOFFSET + 400 + (TETROMINO_SIZE * 5 + 20) * i;
        drawFillRect(x, 120, x + TETROMINO_SIZE*5, 120 + TETROMINO_SIZE*5, gridcolor);
        u8 type = currentbag[(currentbagpos + i) % 14];
        renderBlockData(x + tetrodata[type].offs, 120 + tetrodata[type].offs, type, 0, 0);
    }
}
