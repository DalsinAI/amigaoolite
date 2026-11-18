/* Oolite native SDL3 boot artwork preview; NOT the Oolite game engine.
 * Uses unmodified original Resources/Images/splash.bmp from pinned Oolite.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <SDL3/SDL.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Surface *surface = NULL;
    const char *path = argc > 1 ? argv[1] : "SYS:Oolite/Resources/Images/splash.bmp";
    int rc = 1;
    int ticks = 0;
    const int width = 640, height = 480;
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("OOLITE_SPLASH SDL_INIT_FAIL %s\n", SDL_GetError());
        return 10;
    }
    if (!SDL_CreateWindowAndRenderer("Oolite - AmigaOS Native Preview", width, height, 0, &window, &renderer)) {
        printf("OOLITE_SPLASH WINDOW_FAIL %s\n", SDL_GetError());
        goto done;
    }
    surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("OOLITE_SPLASH LOAD_FAIL %s %s\n",path,SDL_GetError());
        goto done;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("OOLITE_SPLASH TEXTURE_FAIL %s\n",SDL_GetError());
        goto done;
    }
    printf("OOLITE_SPLASH SOURCE=%s WIDTH=%d HEIGHT=%d\n",path,surface->w,surface->h);
    fflush(stdout);
    for (ticks=0; ticks<240; ++ticks) {
        SDL_Event event;
        SDL_FRect dst;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_KEY_DOWN) {
                rc=0;
                goto done;
            }
        }
        dst.x=184.0f; dst.y=40.0f; dst.w=272.0f; dst.h=272.0f;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if (!SDL_RenderTexture(renderer,texture,NULL,&dst)) {
            printf("OOLITE_SPLASH RENDER_FAIL %s\n",SDL_GetError());
            goto done;
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(50);
    }
    rc=0;
done:
    if (texture) SDL_DestroyTexture(texture);
    if (surface) SDL_DestroySurface(surface);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    printf("OOLITE_SPLASH status=%s frames=%d\n",rc==0?"PASS":"FAIL",ticks);
    fflush(stdout);
    return rc;
}
