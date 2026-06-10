/*
 * 2D Graphics Editor in C
 * Uses '*' for drawing and '_' for background
 * Supports: circle, rectangle, line, triangle
 * Operations: add, delete, modify objects
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ─── Canvas Configuration ─────────────────────────────────────────── */
#define ROWS      30
#define COLS      60
#define DRAW_CHAR '*'
#define BG_CHAR   '_'
#define MAX_OBJ   50

/* ─── Shape Types ───────────────────────────────────────────────────── */
typedef enum { SHAPE_CIRCLE = 1, SHAPE_RECT, SHAPE_LINE, SHAPE_TRIANGLE } ShapeType;

/* ─── Shape Descriptors ─────────────────────────────────────────────── */
typedef struct {
    int cx, cy, r;          /* circle: centre x/y, radius */
} Circle;

typedef struct {
    int x, y, w, h;         /* rectangle: top-left corner, width, height */
} Rect;

typedef struct {
    int x1, y1, x2, y2;    /* line: two endpoints */
} Line;

typedef struct {
    int x1, y1;             /* vertex A */
    int x2, y2;             /* vertex B */
    int x3, y3;             /* vertex C */
} Triangle;

typedef struct {
    int       id;
    ShapeType type;
    union {
        Circle   circle;
        Rect     rect;
        Line     line;
        Triangle triangle;
    } data;
} Object;

/* ─── Global State ──────────────────────────────────────────────────── */
char   canvas[ROWS][COLS + 1];   /* +1 for null-terminator per row      */
Object objects[MAX_OBJ];
int    obj_count = 0;
int    next_id   = 1;

/* ═══════════════════════════════════════════════════════════════════════
 *  CANVAS HELPERS
 * ═══════════════════════════════════════════════════════════════════════ */

void canvas_clear(void) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = BG_CHAR;
        canvas[r][COLS] = '\0';
    }
}

static inline void plot(int x, int y) {
    /* x = column, y = row */
    if (y >= 0 && y < ROWS && x >= 0 && x < COLS)
        canvas[y][x] = DRAW_CHAR;
}

/* ═══════════════════════════════════════════════════════════════════════
 *  PRIMITIVE DRAW FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════ */

/* Bresenham's line algorithm */
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        plot(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

/* Midpoint circle algorithm */
void draw_circle(int cx, int cy, int r) {
    int x = 0, y = r;
    int d = 1 - r;

    while (x <= y) {
        plot(cx + x, cy + y); plot(cx - x, cy + y);
        plot(cx + x, cy - y); plot(cx - x, cy - y);
        plot(cx + y, cy + x); plot(cx - y, cy + x);
        plot(cx + y, cy - x); plot(cx - y, cy - x);

        if (d < 0)      d += 2 * x + 3;
        else          { d += 2 * (x - y) + 5; y--; }
        x++;
    }
}

/* Rectangle: four sides */
void draw_rect(int x, int y, int w, int h) {
    draw_line(x,         y,         x + w - 1, y        );  /* top    */
    draw_line(x,         y + h - 1, x + w - 1, y + h - 1);  /* bottom */
    draw_line(x,         y,         x,         y + h - 1);  /* left   */
    draw_line(x + w - 1, y,         x + w - 1, y + h - 1);  /* right  */
}

/* Triangle: three sides */
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  RENDER AN OBJECT ONTO THE CANVAS
 * ═══════════════════════════════════════════════════════════════════════ */

void render_object(const Object *o) {
    switch (o->type) {
        case SHAPE_CIRCLE:
            draw_circle(o->data.circle.cx, o->data.circle.cy, o->data.circle.r);
            break;
        case SHAPE_RECT:
            draw_rect(o->data.rect.x, o->data.rect.y,
                      o->data.rect.w, o->data.rect.h);
            break;
        case SHAPE_LINE:
            draw_line(o->data.line.x1, o->data.line.y1,
                      o->data.line.x2, o->data.line.y2);
            break;
        case SHAPE_TRIANGLE:
            draw_triangle(o->data.triangle.x1, o->data.triangle.y1,
                          o->data.triangle.x2, o->data.triangle.y2,
                          o->data.triangle.x3, o->data.triangle.y3);
            break;
    }
}

/* Re-render every object (used after delete / modify) */
void redraw_all(void) {
    canvas_clear();
    for (int i = 0; i < obj_count; i++)
        render_object(&objects[i]);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  DISPLAY
 * ═══════════════════════════════════════════════════════════════════════ */

void display_picture(void) {
    printf("\n");
    /* top border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("|%s|\n", canvas[r]);
    }

    /* bottom border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n\n");
}

/* ═══════════════════════════════════════════════════════════════════════
 *  LIST OBJECTS
 * ═══════════════════════════════════════════════════════════════════════ */

void list_objects(void) {
    if (obj_count == 0) {
        printf("  (no objects)\n");
        return;
    }
    printf("  %-4s %-10s  Details\n", "ID", "Shape");
    printf("  %-4s %-10s  -------\n", "--", "-----");
    for (int i = 0; i < obj_count; i++) {
        const Object *o = &objects[i];
        printf("  %-4d ", o->id);
        switch (o->type) {
            case SHAPE_CIRCLE:
                printf("%-10s  cx=%d cy=%d r=%d\n", "Circle",
                       o->data.circle.cx, o->data.circle.cy, o->data.circle.r);
                break;
            case SHAPE_RECT:
                printf("%-10s  x=%d y=%d w=%d h=%d\n", "Rectangle",
                       o->data.rect.x, o->data.rect.y,
                       o->data.rect.w, o->data.rect.h);
                break;
            case SHAPE_LINE:
                printf("%-10s  (%d,%d)->(%d,%d)\n", "Line",
                       o->data.line.x1, o->data.line.y1,
                       o->data.line.x2, o->data.line.y2);
                break;
            case SHAPE_TRIANGLE:
                printf("%-10s  (%d,%d) (%d,%d) (%d,%d)\n", "Triangle",
                       o->data.triangle.x1, o->data.triangle.y1,
                       o->data.triangle.x2, o->data.triangle.y2,
                       o->data.triangle.x3, o->data.triangle.y3);
                break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════
 *  FIND OBJECT BY ID
 * ═══════════════════════════════════════════════════════════════════════ */

int find_object(int id) {
    for (int i = 0; i < obj_count; i++)
        if (objects[i].id == id) return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════════════════
 *  ADD OBJECTS
 * ═══════════════════════════════════════════════════════════════════════ */

void add_circle(void) {
    if (obj_count >= MAX_OBJ) { printf("  Object limit reached.\n"); return; }
    int cx, cy, r;
    printf("  Centre x y, radius: "); scanf("%d %d %d", &cx, &cy, &r);
    Object *o = &objects[obj_count++];
    o->id   = next_id++;
    o->type = SHAPE_CIRCLE;
    o->data.circle = (Circle){ cx, cy, r };
    render_object(o);
    printf("  Circle added (id=%d).\n", o->id);
}

void add_rect(void) {
    if (obj_count >= MAX_OBJ) { printf("  Object limit reached.\n"); return; }
    int x, y, w, h;
    printf("  Top-left x y, width height: "); scanf("%d %d %d %d", &x, &y, &w, &h);
    Object *o = &objects[obj_count++];
    o->id   = next_id++;
    o->type = SHAPE_RECT;
    o->data.rect = (Rect){ x, y, w, h };
    render_object(o);
    printf("  Rectangle added (id=%d).\n", o->id);
}

void add_line(void) {
    if (obj_count >= MAX_OBJ) { printf("  Object limit reached.\n"); return; }
    int x1, y1, x2, y2;
    printf("  Start x1 y1, end x2 y2: "); scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
    Object *o = &objects[obj_count++];
    o->id   = next_id++;
    o->type = SHAPE_LINE;
    o->data.line = (Line){ x1, y1, x2, y2 };
    render_object(o);
    printf("  Line added (id=%d).\n", o->id);
}

void add_triangle(void) {
    if (obj_count >= MAX_OBJ) { printf("  Object limit reached.\n"); return; }
    int x1, y1, x2, y2, x3, y3;
    printf("  Vertex 1 (x y): "); scanf("%d %d", &x1, &y1);
    printf("  Vertex 2 (x y): "); scanf("%d %d", &x2, &y2);
    printf("  Vertex 3 (x y): "); scanf("%d %d", &x3, &y3);
    Object *o = &objects[obj_count++];
    o->id   = next_id++;
    o->type = SHAPE_TRIANGLE;
    o->data.triangle = (Triangle){ x1, y1, x2, y2, x3, y3 };
    render_object(o);
    printf("  Triangle added (id=%d).\n", o->id);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  DELETE OBJECT
 * ═══════════════════════════════════════════════════════════════════════ */

void delete_object(void) {
    int id;
    printf("  Enter object ID to delete: "); scanf("%d", &id);
    int idx = find_object(id);
    if (idx < 0) { printf("  ID %d not found.\n", id); return; }

    /* Shift array left */
    for (int i = idx; i < obj_count - 1; i++)
        objects[i] = objects[i + 1];
    obj_count--;

    redraw_all();
    printf("  Object %d deleted.\n", id);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  MODIFY OBJECT
 * ═══════════════════════════════════════════════════════════════════════ */

void modify_object(void) {
    int id;
    printf("  Enter object ID to modify: "); scanf("%d", &id);
    int idx = find_object(id);
    if (idx < 0) { printf("  ID %d not found.\n", id); return; }

    Object *o = &objects[idx];
    switch (o->type) {
        case SHAPE_CIRCLE: {
            int cx, cy, r;
            printf("  New centre x y, radius (current %d %d %d): ",
                   o->data.circle.cx, o->data.circle.cy, o->data.circle.r);
            scanf("%d %d %d", &cx, &cy, &r);
            o->data.circle = (Circle){ cx, cy, r };
            break;
        }
        case SHAPE_RECT: {
            int x, y, w, h;
            printf("  New top-left x y, width height (current %d %d %d %d): ",
                   o->data.rect.x, o->data.rect.y, o->data.rect.w, o->data.rect.h);
            scanf("%d %d %d %d", &x, &y, &w, &h);
            o->data.rect = (Rect){ x, y, w, h };
            break;
        }
        case SHAPE_LINE: {
            int x1, y1, x2, y2;
            printf("  New start x1 y1, end x2 y2 (current %d %d %d %d): ",
                   o->data.line.x1, o->data.line.y1, o->data.line.x2, o->data.line.y2);
            scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
            o->data.line = (Line){ x1, y1, x2, y2 };
            break;
        }
        case SHAPE_TRIANGLE: {
            int x1, y1, x2, y2, x3, y3;
            printf("  New vertex 1 (x y): "); scanf("%d %d", &x1, &y1);
            printf("  New vertex 2 (x y): "); scanf("%d %d", &x2, &y2);
            printf("  New vertex 3 (x y): "); scanf("%d %d", &x3, &y3);
            o->data.triangle = (Triangle){ x1, y1, x2, y2, x3, y3 };
            break;
        }
    }

    redraw_all();
    printf("  Object %d modified.\n", id);
}

/* ═══════════════════════════════════════════════════════════════════════
 *  CLEAR CANVAS
 * ═══════════════════════════════════════════════════════════════════════ */

void clear_all(void) {
    obj_count = 0;
    canvas_clear();
    printf("  Canvas and object list cleared.\n");
}

/* ═══════════════════════════════════════════════════════════════════════
 *  MAIN MENU
 * ═══════════════════════════════════════════════════════════════════════ */

void print_menu(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║     2D ASCII Graphics Editor         ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  ADD       1-Circle  2-Rect          ║\n");
    printf("║            3-Line    4-Triangle      ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  5  Delete object                    ║\n");
    printf("║  6  Modify object                    ║\n");
    printf("║  7  List objects                     ║\n");
    printf("║  8  Display picture                  ║\n");
    printf("║  9  Clear all                        ║\n");
    printf("║  0  Quit                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("  Choice: ");
}

int main(void) {
    canvas_clear();

    int choice;
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) break;
        printf("\n");
        switch (choice) {
            case 1: add_circle();   break;
            case 2: add_rect();     break;
            case 3: add_line();     break;
            case 4: add_triangle(); break;
            case 5: delete_object(); break;
            case 6: modify_object(); break;
            case 7: list_objects();  break;
            case 8: display_picture(); break;
            case 9: clear_all();     break;
            case 0: printf("  Goodbye!\n"); break;
            default: printf("  Invalid choice.\n");
        }
        printf("\n");
    } while (choice != 0);

    return 0;
}
