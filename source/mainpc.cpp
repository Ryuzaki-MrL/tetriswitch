#ifndef SWITCH

#include <SFML/Graphics.hpp>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define TICKS_PER_SECOND 60
#define SKIP_TICKS (CLOCKS_PER_SEC / TICKS_PER_SECOND)
#define MAX_FRAMESKIP 5

#include "tetris.h"
#include "switchdefs.h"

void gfxFlushBuffers() {}
void gfxSwapBuffers() {}
void gfxWaitForVsync() {}

u64 randomGet64() {
    return rand();
}

u32 pixels[720 * 1280];

int running = 1;

u8* gfxGetFramebuffer(u32* width, u32* height) {
    *width = 1280;
    *height = 720;
    return (u8*)pixels;
}

static u64 kDown = 0;
static u64 kHeld = 0;

u64 hidKeysDown(int n) {
    return kDown;
}

u64 hidKeysHeld(int n) {
    return kHeld;
}

int main()
{
    srand(time(0));
    tetrisInit();

    sf::RenderWindow window(sf::VideoMode(1280, 720), "Test");
    window.setFramerateLimit(60);

    // Clear the screen with a dark blue color
    for (int i = 0; i < 720 * 1280; ++i) {
        pixels[i] = 0xFF4F0000;
    }

    while (window.isOpen() && running)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                break;
            }
            if (event.type == sf::Event::KeyPressed)
            {
                switch(event.key.code) {
                    case sf::Keyboard::Z: kDown |= 1; break;
                    case sf::Keyboard::X: kDown |= 2; break;
                    case sf::Keyboard::C: kDown |= 4; break;
                    case sf::Keyboard::V: kDown |= 8; break;
                    case sf::Keyboard::Q: kDown |= 16; break;
                    case sf::Keyboard::W: kDown |= 32; break;
                    case sf::Keyboard::A: kDown |= 64; break;
                    case sf::Keyboard::S: kDown |= 128; break;
                    case sf::Keyboard::E: kDown |= 256; break;
                    case sf::Keyboard::R: kDown |= 512; break;
                    case sf::Keyboard::Return: kDown |= 1024; break;
                    case sf::Keyboard::Space: kDown |= 2048; break;
                    case sf::Keyboard::Left: kDown |= 4096; break;
                    case sf::Keyboard::Up: kDown |= 8192; break;
                    case sf::Keyboard::Right: kDown |= 16384; break;
                    case sf::Keyboard::Down: kDown |= 32768; break;
                }
                kHeld = kDown;
            }
            if (event.type == sf::Event::KeyReleased)
            {
                switch(event.key.code) {
                    case sf::Keyboard::Z: kDown &= ~1; break;
                    case sf::Keyboard::X: kDown &= ~2; break;
                    case sf::Keyboard::C: kDown &= ~4; break;
                    case sf::Keyboard::V: kDown &= ~8; break;
                    case sf::Keyboard::Q: kDown &= ~16; break;
                    case sf::Keyboard::W: kDown &= ~32; break;
                    case sf::Keyboard::A: kDown &= ~64; break;
                    case sf::Keyboard::S: kDown &= ~128; break;
                    case sf::Keyboard::E: kDown &= ~256; break;
                    case sf::Keyboard::R: kDown &= ~512; break;
                    case sf::Keyboard::Return: kDown &= ~1024; break;
                    case sf::Keyboard::Space: kDown &= ~2048; break;
                    case sf::Keyboard::Left: kDown &= ~4096; break;
                    case sf::Keyboard::Up: kDown &= ~8192; break;
                    case sf::Keyboard::Right: kDown &= ~16384; break;
                    case sf::Keyboard::Down: kDown &= ~32768; break;
                }
                kHeld = kDown;
            }
        }

        update();
        kDown = 0;
        render();

        window.clear();

        sf::Image image;
        image.create(1280, 720, (const unsigned char*)pixels);

        sf::Texture texture;
        texture.loadFromImage(image);

        sf::Sprite sprite;
        sprite.setTexture(texture);

        window.draw(sprite);
        window.display();
    }

    return 0;
}

#endif
