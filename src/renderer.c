#include "graphxml.h"
#include <math.h>
#include <string.h>

static void draw_line(SDL_Renderer* r, int x1,int y1,int x2,int y2, SDL_Color c){
    SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);
    SDL_RenderDrawLine(r,x1,y1,x2,y2);
}

static void draw_rect(SDL_Renderer* r, int x,int y,int w,int h, SDL_Color c){
    SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);
    SDL_Rect rr = {x,y,w,h};
    SDL_RenderFillRect(r,&rr);
}

static void draw_circle(SDL_Renderer* r, int cx, int cy, int rad, SDL_Color c){
    SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);
    for (int dy=-rad; dy<=rad; ++dy) {
        int dx = (int)sqrt(rad*rad - dy*dy);
        SDL_RenderDrawLine(r, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}

static void draw_text(SDL_Renderer* r, TTF_Font* font, const char* text, int x, int y, SDL_Color c){
    if (!font || !text) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, c);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_FreeSurface(surf);
    if (tex) {
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
}

int render_graph(const Graph* g, SDL_Renderer* r, TTF_Font* font, int width, int height) {
    if (!g || !r) return -1;

    // background
    SDL_SetRenderDrawColor(r, 250,250,250,255);
    SDL_RenderClear(r);

    // margins
    const int L = 80, R = 30, T = 60, B = 70;

    // axes area
    int plot_x = L, plot_y = T;
    int plot_w = width - L - R;
    int plot_h = height - T - B;

    // axes
    SDL_Color axisCol = {40,40,40,255};
    draw_line(r, plot_x, plot_y+plot_h, plot_x+plot_w, plot_y+plot_h, axisCol); // X
    draw_line(r, plot_x, plot_y, plot_x, plot_y+plot_h, axisCol);               // Y

    // title
    SDL_Color textCol = {20,20,20,255};
    if (g->title) {
        draw_text(r, font, g->title, plot_x, 15, textCol);
    }
    if (g->x_axis.label) {
        draw_text(r, font, g->x_axis.label, plot_x + plot_w/2 - 30, plot_y+plot_h+35, textCol);
    }
    if (g->y_axis.label) {
        // simple: draw near top-left (no rotation for MVP)
        draw_text(r, font, g->y_axis.label, 10, plot_y-25, textCol);
    }

    // Y ticks
    int ticks = 5;
    double y0 = g->y_min, y1 = g->y_max;
    double step = (y1 - y0) / ticks;
    for (int i=0;i<=ticks;i++){
        double v = y0 + i*step;
        int y = (int)(plot_y + plot_h - (v - y0) * plot_h / (y1 - y0));
        SDL_Color grid = {220,220,220,255};
        draw_line(r, plot_x, y, plot_x+plot_w, y, grid);
        char lab[64]; snprintf(lab, sizeof(lab), "%.2f", v);
        draw_text(r, font, lab, 10, y-8, textCol);
    }

    // map x labels
    // count max points
    int max_points = 0;
    for (int i=0;i<g->dataset_count;i++)
        if (g->datasets[i].point_count > max_points) max_points = g->datasets[i].point_count;
    if (max_points < 2) max_points = 2;
    // draw x labels and tick marks based on first dataset (MVP)
    int xs_y = plot_y+plot_h;
    for (int i=0;i<max_points;i++){
        int x = plot_x + (int)((double)i * (plot_w) / (double)(max_points-1));
        SDL_Color tickc = {120,120,120,255};
        draw_line(r, x, xs_y, x, xs_y+5, tickc);
        // try pick label from first dataset if exists
        const char* lab = NULL;
        if (g->dataset_count>0 && i < g->datasets[0].point_count)
            lab = g->datasets[0].points[i].x_label;
        if (lab) draw_text(r, font, lab, x-10, xs_y+10, textCol);
    }

    // datasets lines
    for (int d=0; d<g->dataset_count; d++) {
        GraphDataset* ds = &((GraphDataset*)g->datasets)[d];
        if (ds->point_count < 1) continue;
        // polyline
        for (int i=0;i<ds->point_count;i++){
            int x = plot_x + (int)((double)i * (plot_w) / (double)(max_points-1));
            double v = ds->points[i].y_value;
            int y = (int)(plot_y + plot_h - (v - y0) * plot_h / (y1 - y0));
            if (i>0){
                int px = plot_x + (int)((double)(i-1) * (plot_w) / (double)(max_points-1));
                double pv = ds->points[i-1].y_value;
                int py = (int)(plot_y + plot_h - (pv - y0) * plot_h / (y1 - y0));
                draw_line(r, px, py, x, y, ds->color);
            }
            draw_circle(r, x, y, 3, ds->color);
        }
    }

    // legend
    int lx = plot_x + plot_w - 140;
    int ly = plot_y + 10;
    SDL_Color legendBg = {255,255,255,230};
    draw_rect(r, lx-10, ly-10, 150, 20 * (g->dataset_count>0?g->dataset_count:1) + 20, legendBg);
    SDL_Color border = {200,200,200,255};
    SDL_Rect lb = {lx-10, ly-10, 150, 20 * (g->dataset_count>0?g->dataset_count:1) + 20};
    SDL_SetRenderDrawColor(r, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(r, &lb);

    for (int d=0; d<g->dataset_count; d++) {
        draw_rect(r, lx, ly + d*20, 12, 12, g->datasets[d].color);
        if (g->datasets[d].name)
            draw_text(r, font, g->datasets[d].name, lx+20, ly + d*20 - 3, textCol);
    }

    SDL_RenderPresent(r);
    return 0;
}

int gx_show_window(const Graph* g, const char* font_path, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return -1;
    }
    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init: %s", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window* win = SDL_CreateWindow(
        g && g->title ? g->title : "GraphXML",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN);
    if (!win) { SDL_Log("CreateWindow: %s", SDL_GetError()); TTF_Quit(); SDL_Quit(); return -1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) { SDL_Log("CreateRenderer: %s", SDL_GetError()); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit(); return -1; }

    TTF_Font* font = gx_load_font(font_path, 16);

    render_graph(g, ren, font, width, height);

    int running=1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running=0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running=0;
        }
        SDL_Delay(10);
    }

    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}