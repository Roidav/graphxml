#include "graphxml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ---- minimal helpers ----

static char* gx_strdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* p = (char*)malloc(n+1);
    if (p) memcpy(p, s, n+1);
    return p;
}

static char* trim(char* s) {
    if (!s) return s;
    char* e = s + strlen(s);
    while (e>s && isspace((unsigned char)e[-1])) --e;
    *e = 0;
    while (*s && isspace((unsigned char)*s)) ++s;
    return s;
}

static char* read_file(const char* path, long* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(n+1);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf,1,n,f)!=(size_t)n) { fclose(f); free(buf); return NULL; }
    fclose(f);
    buf[n]=0;
    if (out_len) *out_len = n;
    return buf;
}

// extract attribute value attr="value" from tag text
static int attr_val(const char* tag, const char* attr, char* out, size_t outsz) {
    // very naive but sufficient for controlled input
    const char* p = strstr(tag, attr);
    if (!p) return -1;
    p += strlen(attr);
    while (*p && isspace((unsigned char)*p)) ++p;
    if (*p!='=') return -1;
    ++p;
    while (*p && isspace((unsigned char)*p)) ++p;
    if (*p!='\"' && *p!='\'') return -1;
    char quote = *p++;
    size_t i=0;
    while (*p && *p!=quote && i+1<outsz) out[i++] = *p++;
    out[i]=0;
    return (*p==quote) ? 0 : -1;
}

// --- parsing ---

static int parse_graph_header(const char* tag, Graph* g) {
    char val[256];
    if (attr_val(tag, "type", val, sizeof(val))==0) g->type = gx_strdup(val);
    if (attr_val(tag, "title", val, sizeof(val))==0) g->title = gx_strdup(val);
    return 0;
}

static int parse_axis(const char* tag, GraphAxis* axis) {
    char val[256];
    if (attr_val(tag, "label", val, sizeof(val))==0) axis->label = gx_strdup(val);
    return 0;
}

static int parse_dataset_open(const char* tag, GraphDataset* ds) {
    char val[256];
    memset(ds, 0, sizeof(*ds));
    if (attr_val(tag, "name", val, sizeof(val))==0) ds->name = gx_strdup(val);
    if (attr_val(tag, "color", val, sizeof(val))==0) {
        gx_parse_color(val, &ds->color);
    } else {
        ds->color.r=0; ds->color.g=0; ds->color.b=255; ds->color.a=255;
    }
    return 0;
}

static int parse_point(const char* tag, GraphPoint* pt) {
    char val[256];
    if (attr_val(tag, "x", val, sizeof(val))==0) pt->x_label = gx_strdup(val);
    if (attr_val(tag, "y", val, sizeof(val))==0) pt->y_value = atof(val);
    return 0;
}

static void free_dataset(GraphDataset* ds) {
    if (!ds) return;
    free(ds->name);
    for (int i=0;i<ds->point_count;i++) free(ds->points[i].x_label);
    free(ds->points);
}

static int push_point(GraphDataset* ds, const GraphPoint* pt) {
    GraphPoint* np = (GraphPoint*)realloc(ds->points, sizeof(GraphPoint)*(ds->point_count+1));
    if (!np) return -1;
    ds->points = np;
    ds->points[ds->point_count++] = *pt;
    return 0;
}

static int push_dataset(Graph* g, const GraphDataset* ds) {
    GraphDataset* nd = (GraphDataset*)realloc(g->datasets, sizeof(GraphDataset)*(g->dataset_count+1));
    if (!nd) return -1;
    g->datasets = nd;
    g->datasets[g->dataset_count++] = *ds;
    return 0;
}

int parse_graphxml(const char* filename, Graph* out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    long n=0;
    char* xml = read_file(filename, &n);
    if (!xml) {
        fprintf(stderr, "Failed to read %s\n", filename);
        return -1;
    }
    // very simple tokenization over tags
    char* p = xml;
    GraphDataset cur_ds; int ds_open = 0;
    memset(&cur_ds, 0, sizeof(cur_ds));

    while (*p) {
        char* lt = strchr(p, '<');
        if (!lt) break;
        char* gt = strchr(lt, '>');
        if (!gt) break;
        size_t len = (size_t)(gt - lt + 1);
        char* tag = (char*)malloc(len+1);
        memcpy(tag, lt, len); tag[len]=0;

        if (strncmp(tag, "<graph", 6)==0 && tag[6]!='/' ) {
            parse_graph_header(tag, out);
        } else if (strncmp(tag, "<x-axis", 7)==0) {
            parse_axis(tag, &out->x_axis);
        } else if (strncmp(tag, "<y-axis", 7)==0) {
            parse_axis(tag, &out->y_axis);
        } else if (strncmp(tag, "<dataset", 8)==0 && tag[8]!='/' ) {
            if (ds_open) { free(tag); free(xml); return -1; }
            parse_dataset_open(tag, &cur_ds);
            ds_open = 1;
        } else if (strncmp(tag, "</dataset", 9)==0) {
            if (!ds_open) { free(tag); free(xml); return -1; }
            push_dataset(out, &cur_ds);
            memset(&cur_ds, 0, sizeof(cur_ds));
            ds_open = 0;
        } else if (strncmp(tag, "<point", 6)==0) {
            if (!ds_open) { free(tag); free(xml); return -1; }
            GraphPoint pt = {0};
            parse_point(tag, &pt);
            push_point(&cur_ds, &pt);
        }
        free(tag);
        p = gt + 1;
    }

    free(xml);

    gx_compute_y_range(out);
    return 0;
}

void free_graph(Graph* g) {
    if (!g) return;
    free(g->type);
    free(g->title);
    free(g->x_axis.label);
    free(g->y_axis.label);
    for (int i=0;i<g->dataset_count;i++) {
        free(g->datasets[i].name);
        for (int j=0;j<g->datasets[i].point_count;j++) {
            free(g->datasets[i].points[j].x_label);
        }
        free(g->datasets[i].points);
    }
    free(g->datasets);
    memset(g, 0, sizeof(*g));
}