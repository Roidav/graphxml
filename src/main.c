#include "graphxml.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <graphxml_file> [output.png]\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = (argc >= 3) ? argv[2] : "output.png";

    // Parse GraphXML
    Graph g;
    if (parse_graphxml(input_file, &g) != 0) {
        fprintf(stderr, "Failed to parse: %s\n", input_file);
        return 1;
    }

    // Initialize SDL and TTF
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Window & Renderer (hidden for headless)
    int width = 800;
    int height = 600;
    SDL_Window* window = SDL_CreateWindow("", 0, 0, width, height, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    const char* font_path = "assets/fonts/DejaVuSans.ttf";
    TTF_Font* font = TTF_OpenFont(font_path, 16);
    if (!font) {
        fprintf(stderr, "Failed to load font %s: %s\n", font_path, TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Render graph
    if (render_graph(&g, renderer, font, width, height) != 0) {
        fprintf(stderr, "Failed to render graph.\n");
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        free_graph(&g);
        return 1;
    }

    // Copy pixels to SDL_Surface
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        fprintf(stderr, "Failed to create SDL_Surface: %s\n", SDL_GetError());
    } else {
        SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch);
        // Save as PNG
        if (stbi_write_png(output_file, surface->w, surface->h, 4, surface->pixels, surface->pitch)) {
            printf("Graph saved to %s\n", output_file);
        } else {
            fprintf(stderr, "Failed to save PNG: %s\n", output_file);
        }
        SDL_FreeSurface(surface);
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    free_graph(&g);

    return 0;
}