#include <iostream>
#include "thread"
#include "chrono"
#include "SDL.h"
#include "chip8.hpp"
using namespace std;

const int SCREEN_WIDTH =  640;
const int SCREEN_HEIGHT = 320;

uint8_t keymap[16] = {
        SDLK_x,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_q,
        SDLK_w,
        SDLK_e,
        SDLK_a,
        SDLK_s,
        SDLK_d,
        SDLK_z,
        SDLK_c,
        SDLK_4,
        SDLK_r,
        SDLK_f,
        SDLK_v,
};

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "ROM path not specified" << endl;
        exit(EXIT_FAILURE);
    }

    // Starts up SDL and creates window
    SDL_Window* window = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    } else {
        window = SDL_CreateWindow( "Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == nullptr ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }

    // Creates render for window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (sdlTexture == nullptr) {
        printf( "Texture could not be created! SDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    Chip8 chip8 = Chip8();
    if (!chip8.loadROM(argv[1])) {
        cout << "ROM could not be loaded" << endl;
        exit(2);
    }

    while (true) {
        chip8.emulateCycle();
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(EXIT_SUCCESS);
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    exit(EXIT_SUCCESS);
                }

                for (int i=0; i<16; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 1;
                    }
                }
            }

            if (e.type == SDL_KEYUP) {
                for (int i=0; i<16; i++) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 0;
                    }
                }
            }
        }
        uint32_t pixels[2048];
        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            for (int i=0; i<2048; i++) {
                uint8_t pixel = chip8.gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            SDL_UpdateTexture(sdlTexture, nullptr, pixels, 64 * sizeof(Uint32));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        this_thread::sleep_for(chrono::microseconds (1200));
    }
    return 0;
}