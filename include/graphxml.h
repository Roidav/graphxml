#ifndef GRAPHXML_H
#define GRAPHXML_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------- Data structures ----------

typedef struct {
    char* x_label;   // e.g., "Mon"
    double y_value;  // numeric value
} GraphPoint;

typedef struct {
    char* name;        // e.g., "City A"
    SDL_Color color;   // resolved color
    GraphPoint* points;
    int point_count;
} GraphDataset;

typedef struct {
    char* label; // axis label
} GraphAxis;

typedef struct {
    char* type;        // "line" (MVP)
    char* title;       // chart title
    GraphAxis x_axis;
    GraphAxis y_axis;
    GraphDataset* datasets;
    int dataset_count;

    // derived (computed after parsing)
    double y_min, y_max;
} Graph;

// ---------- Public API ----------

// Parse .graphxml into Graph (allocates strings/arrays). Returns 0 on success.
int parse_graphxml(const char* filename, Graph* out);

// Free all allocated memory within Graph
void free_graph(Graph* g);

// Render to an SDL_Renderer (caller owns window/renderer). Returns 0 on success.
int render_graph(const Graph* g, SDL_Renderer* r, TTF_Font* font, int width, int height);

// Utility to load a TTF font at size; returns NULL if not found.
TTF_Font* gx_load_font(const char* path, int pt);

// Convert a CSS-like name or #RRGGBB into SDL_Color. Returns 0 on success.
int gx_parse_color(const char* s, SDL_Color* out);

// Clamp and nice tick generation for Y axis; exposed for testing (optional).
void gx_compute_y_range(Graph* g);

// Convenience: draw a single graph in its own SDL window (blocks until quit).
// Returns 0 on success. If font_path is NULL, tries a default; if not found, draws without text.
int gx_show_window(const Graph* g, const char* font_path, int width, int height);

#ifdef __cplusplus
}
#endif

#endif // GRAPHXML_H