#include "graphxml.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static int hex2i(char c) {
    if (c>='0' && c<='9') return c-'0';
    if (c>='a' && c<='f') return 10 + (c-'a');
    if (c>='A' && c<='F') return 10 + (c-'A');
    return -1;
}

static int parse_hex_color(const char* s, SDL_Color* out) {
    // Expect #RRGGBB
    if (!s || s[0] != '#' || strlen(s) != 7) return -1;
    int r1 = hex2i(s[1]), r2 = hex2i(s[2]);
    int g1 = hex2i(s[3]), g2 = hex2i(s[4]);
    int b1 = hex2i(s[5]), b2 = hex2i(s[6]);
    if (r1<0||r2<0||g1<0||g2<0||b1<0||b2<0) return -1;
    out->r = (Uint8)(r1*16 + r2);
    out->g = (Uint8)(g1*16 + g2);
    out->b = (Uint8)(b1*16 + b2);
    out->a = 255;
    return 0;
}

typedef struct { const char* name; Uint8 r,g,b; } NamedColor;

static const NamedColor NAMED[] = {
    {"black",0,0,0},{"white",255,255,255},{"red",255,0,0},{"green",0,128,0},
    {"blue",0,0,255},{"yellow",255,255,0},{"magenta",255,0,255},{"purple",128,0,128},
    {"orange",255,165,0},{"gray",128,128,128},{"grey",128,128,128},{"cyan",0,255,255}
};

int gx_parse_color(const char* s, SDL_Color* out) {
    if (!s || !out) return -1;
    if (parse_hex_color(s, out) == 0) return 0;
    // lowercase compare
    char buf[64]; size_t n = strlen(s);
    if (n >= sizeof(buf)) n = sizeof(buf)-1;
    for (size_t i=0;i<n;i++) buf[i] = (char)tolower((unsigned char)s[i]);
    buf[n]=0;
    for (size_t i=0;i<sizeof(NAMED)/sizeof(NAMED[0]);++i) {
        if (strcmp(buf, NAMED[i].name)==0) {
            out->r = NAMED[i].r; out->g = NAMED[i].g; out->b = NAMED[i].b; out->a = 255;
            return 0;
        }
    }
    // default
    out->r = 0; out->g = 0; out->b = 0; out->a = 255;
    return -1;
}

TTF_Font* gx_load_font(const char* path, int pt) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0) {
            fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
            return NULL;
        }
    }
    if (!path) return NULL;
    return TTF_OpenFont(path, pt);
}

static double nice_num(double x, int round_) {
    int expv = (int)floor(log10(x));
    double f = x / pow(10.0, expv);
    double nf;
    if (round_) {
        if (f < 1.5) nf = 1;
        else if (f < 3) nf = 2;
        else if (f < 7) nf = 5;
        else nf = 10;
    } else {
        if (f <= 1) nf = 1;
        else if (f <= 2) nf = 2;
        else if (f <= 5) nf = 5;
        else nf = 10;
    }
    return nf * pow(10.0, expv);
}

void gx_compute_y_range(Graph* g) {
    if (!g || g->dataset_count==0) { g->y_min=0; g->y_max=1; return; }
    double mn = 1e300, mx = -1e300;
    for (int i=0;i<g->dataset_count;i++) {
        for (int j=0;j<g->datasets[i].point_count;j++) {
            double v = g->datasets[i].points[j].y_value;
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
    }
    if (mn > mx) { mn=0; mx=1; }
    if (mx==mn) { mx = mn + 1.0; }
    double range = mx - mn;
    double nice_range = nice_num(range, 0);
    double tick = nice_num(nice_range/5.0, 1);
    double y_min = floor(mn / tick) * tick;
    double y_max = ceil(mx / tick) * tick;
    g->y_min = y_min;
    g->y_max = y_max;
}