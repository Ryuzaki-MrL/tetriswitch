#ifndef SWITCHDEFS_H
#define SWITCHDEFS_H

#include <stdint.h>

#define CONTROLLER_P1_AUTO 0

#define BIT(n) (1U<<(n))

#define RGBA8(r,g,b,a)  (((r)&0xff)|(((g)&0xff)<<8)|(((b)&0xff)<<16)|(((a)&0xff)<<24))
#define RGBA8_MAXALPHA(r,g,b) RGBA8(r,g,b,0xff)

typedef enum
{
    KEY_A            = BIT(0),       ///< A
    KEY_B            = BIT(1),       ///< B
    KEY_X            = BIT(2),       ///< X
    KEY_Y            = BIT(3),       ///< Y
    KEY_LSTICK       = BIT(4),       ///< Left Stick Button
    KEY_RSTICK       = BIT(5),       ///< Right Stick Button
    KEY_L            = BIT(6),       ///< L
    KEY_R            = BIT(7),       ///< R
    KEY_ZL           = BIT(8),       ///< ZL
    KEY_ZR           = BIT(9),       ///< ZR
    KEY_PLUS         = BIT(10),      ///< Plus
    KEY_MINUS        = BIT(11),      ///< Minus
    KEY_DLEFT        = BIT(12),      ///< D-Pad Left
    KEY_DUP          = BIT(13),      ///< D-Pad Up
    KEY_DRIGHT       = BIT(14),      ///< D-Pad Right
    KEY_DDOWN        = BIT(15),      ///< D-Pad Down
    KEY_LSTICK_LEFT  = BIT(16),      ///< Left Stick Left
    KEY_LSTICK_UP    = BIT(17),      ///< Left Stick Up
    KEY_LSTICK_RIGHT = BIT(18),      ///< Left Stick Right
    KEY_LSTICK_DOWN  = BIT(19),      ///< Left Stick Down
    KEY_RSTICK_LEFT  = BIT(20),      ///< Right Stick Left
    KEY_RSTICK_UP    = BIT(21),      ///< Right Stick Up
    KEY_RSTICK_RIGHT = BIT(22),      ///< Right Stick Right
    KEY_RSTICK_DOWN  = BIT(23),      ///< Right Stick Down
    KEY_SL           = BIT(24),      ///< SL
    KEY_SR           = BIT(25),      ///< SR

    // Pseudo-key for at least one finger on the touch screen
    KEY_TOUCH       = BIT(26),

    // Buttons by orientation (for single Joy-Con), also works with Joy-Con pairs, Pro Controller
    KEY_JOYCON_RIGHT = BIT(0),
    KEY_JOYCON_DOWN  = BIT(1),
    KEY_JOYCON_UP    = BIT(2),
    KEY_JOYCON_LEFT  = BIT(3),

    // Generic catch-all directions, also works for single Joy-Con
    KEY_UP    = KEY_DUP    | KEY_LSTICK_UP    | KEY_RSTICK_UP,    ///< D-Pad Up or Sticks Up
    KEY_DOWN  = KEY_DDOWN  | KEY_LSTICK_DOWN  | KEY_RSTICK_DOWN,  ///< D-Pad Down or Sticks Down
    KEY_LEFT  = KEY_DLEFT  | KEY_LSTICK_LEFT  | KEY_RSTICK_LEFT,  ///< D-Pad Left or Sticks Left
    KEY_RIGHT = KEY_DRIGHT | KEY_LSTICK_RIGHT | KEY_RSTICK_RIGHT, ///< D-Pad Right or Sticks Right
} HidControllerKeys;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;
typedef int8_t s8;

u64 randomGet64();

u64 hidKeysDown(int n);
u64 hidKeysHeld(int n);

u8* gfxGetFramebuffer(u32* width, u32* height);
void gfxFlushBuffers();
void gfxSwapBuffers();
void gfxWaitForVsync();

#endif /* SWITCHDEFS_H */