/*
 * ============================================================
 *   2D GRAPHICS EDITOR — C Implementation
 *   Canvas : 2D character array  (ROWS x COLS)
 *   Background fill : '_'  (underscore)
 *   Drawing character : '*' (asterisk)
 *
 *   Supported Objects : Circle, Rectangle, Line, Triangle
 *   Operations        : Add, Delete, Modify, Display
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─── Canvas dimensions ───────────────────────────────────── */
#define ROWS  30
#define COLS  70
#define BLANK '_'
#define DRAW  '*'

/* ─── Object type codes ───────────────────────────────────── */
#define TYPE_CIRCLE    1
#define TYPE_RECT      2
#define TYPE_LINE      3
#define TYPE_TRIANGLE  4

/* ─── Maximum objects stored ──────────────────────────────── */
#define MAX_OBJECTS 50

/* ═══════════════════════════════════════════════════════════
 *  Data structures
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int cx, cy, radius;          /* Circle  */
} Circle;

typedef struct {
    int x1, y1, x2, y2;         /* Rectangle top-left / bottom-right */
} Rectangle;

typedef struct {
    int x1, y1, x2, y2;         /* Line endpoints */
} Line;

typedef struct {
    int x1, y1;                  /* Vertex A */
    int x2, y2;                  /* Vertex B */
    int x3, y3;                  /* Vertex C */
} Triangle;

typedef union {
    Circle    circle;
    Rectangle rect;
    Line      line;
    Triangle  tri;
} ShapeData;

typedef struct {
    int       id;
    int       type;
    ShapeData data;
    int       active;            /* 1 = visible, 0 = deleted */
} Object;

/* ─── Global state ─────────────────────────────────────────── */
static char    canvas[ROWS][COLS];
static Object  objects[MAX_OBJECTS];
static int     obj_count  = 0;
static int     next_id    = 1;

/* ═══════════════════════════════════════════════════════════
 *  Canvas helpers
 * ═══════════════════════════════════════════════════════════ */

void canvas_init(void)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = BLANK;
}

/* Safe pixel set — ignores out-of-bounds coordinates */
void put_pixel(int row, int col, char ch)
{
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        canvas[row][col] = ch;
}

void canvas_display(void)
{
    printf("\n");
    /* Top border */
    printf("  +");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("%2d|", r);
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    /* Bottom border */
    printf("  +");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");

    /* Column ruler (tens) */
    printf("   ");
    for (int c = 0; c < COLS; c++)
        printf("%c", (c % 10 == 0) ? ('0' + (c / 10)) : ' ');
    printf("\n");

    /* Column ruler (units) */
    printf("   ");
    for (int c = 0; c < COLS; c++)
        printf("%d", c % 10);
    printf("\n\n");
}

/* Rebuild canvas from scratch using all active objects */
void canvas_rebuild(void);   /* forward declaration */

/* ═══════════════════════════════════════════════════════════
 *  Drawing algorithms
 * ═══════════════════════════════════════════════════════════ */

/* Bresenham's line algorithm */
void draw_line_algo(int x0, int y0, int x1, int y1, char ch)
{
    int dx =  abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (1) {
        put_pixel(y0, x0, ch);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/* Midpoint circle algorithm */
void draw_circle_algo(int cx, int cy, int r, char ch)
{
    int x = 0, y = r;
    int d = 1 - r;

    while (x <= y) {
        /* 8-way symmetry */
        put_pixel(cy + y, cx + x, ch);
        put_pixel(cy + y, cx - x, ch);
        put_pixel(cy - y, cx + x, ch);
        put_pixel(cy - y, cx - x, ch);
        put_pixel(cy + x, cx + y, ch);
        put_pixel(cy + x, cx - y, ch);
        put_pixel(cy - x, cx + y, ch);
        put_pixel(cy - x, cx - y, ch);

        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void draw_rectangle_algo(int x1, int y1, int x2, int y2, char ch)
{
    /* Normalise so x1<=x2, y1<=y2 */
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }

    draw_line_algo(x1, y1, x2, y1, ch); /* top    */
    draw_line_algo(x1, y2, x2, y2, ch); /* bottom */
    draw_line_algo(x1, y1, x1, y2, ch); /* left   */
    draw_line_algo(x2, y1, x2, y2, ch); /* right  */
}

void draw_triangle_algo(int x1, int y1,
                         int x2, int y2,
                         int x3, int y3, char ch)
{
    draw_line_algo(x1, y1, x2, y2, ch);
    draw_line_algo(x2, y2, x3, y3, ch);
    draw_line_algo(x3, y3, x1, y1, ch);
}

/* ─── Dispatch: draw a single object onto the canvas ────── */
void draw_object(const Object *o, char ch)
{
    switch (o->type) {
        case TYPE_CIRCLE:
            draw_circle_algo(o->data.circle.cx,
                             o->data.circle.cy,
                             o->data.circle.radius, ch);
            break;
        case TYPE_RECT:
            draw_rectangle_algo(o->data.rect.x1, o->data.rect.y1,
                                 o->data.rect.x2, o->data.rect.y2, ch);
            break;
        case TYPE_LINE:
            draw_line_algo(o->data.line.x1, o->data.line.y1,
                            o->data.line.x2, o->data.line.y2, ch);
            break;
        case TYPE_TRIANGLE:
            draw_triangle_algo(o->data.tri.x1, o->data.tri.y1,
                                o->data.tri.x2, o->data.tri.y2,
                                o->data.tri.x3, o->data.tri.y3, ch);
            break;
    }
}

/* Rebuild the entire canvas by re-drawing all active objects */
void canvas_rebuild(void)
{
    canvas_init();
    for (int i = 0; i < obj_count; i++)
        if (objects[i].active)
            draw_object(&objects[i], DRAW);
}

/* ═══════════════════════════════════════════════════════════
 *  Object list helpers
 * ═══════════════════════════════════════════════════════════ */

void list_objects(void)
{
    int any = 0;
    printf("\n  ┌─────────────────────────────────────────┐\n");
    printf("  │         OBJECTS ON CANVAS               │\n");
    printf("  ├────┬────────────┬────────────────────────┤\n");
    printf("  │ ID │ Type       │ Parameters             │\n");
    printf("  ├────┼────────────┼────────────────────────┤\n");
    for (int i = 0; i < obj_count; i++) {
        Object *o = &objects[i];
        if (!o->active) continue;
        any = 1;
        switch (o->type) {
            case TYPE_CIRCLE:
                printf("  │%3d │ Circle     │ cx=%2d cy=%2d r=%2d       │\n",
                       o->id,
                       o->data.circle.cx, o->data.circle.cy,
                       o->data.circle.radius);
                break;
            case TYPE_RECT:
                printf("  │%3d │ Rectangle  │ (%2d,%2d)->(%2d,%2d)      │\n",
                       o->id,
                       o->data.rect.x1, o->data.rect.y1,
                       o->data.rect.x2, o->data.rect.y2);
                break;
            case TYPE_LINE:
                printf("  │%3d │ Line       │ (%2d,%2d)->(%2d,%2d)      │\n",
                       o->id,
                       o->data.line.x1, o->data.line.y1,
                       o->data.line.x2, o->data.line.y2);
                break;
            case TYPE_TRIANGLE:
                printf("  │%3d │ Triangle   │ A(%2d,%2d) B(%2d,%2d) C(%2d,%2d)│\n",
                       o->id,
                       o->data.tri.x1, o->data.tri.y1,
                       o->data.tri.x2, o->data.tri.y2,
                       o->data.tri.x3, o->data.tri.y3);
                break;
        }
    }
    if (!any)
        printf("  │              (canvas is empty)          │\n");
    printf("  └────┴────────────┴────────────────────────┘\n\n");
}

/* Find active object by id; returns index or -1 */
int find_object(int id)
{
    for (int i = 0; i < obj_count; i++)
        if (objects[i].active && objects[i].id == id)
            return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════
 *  ADD operations
 * ═══════════════════════════════════════════════════════════ */

void add_circle(void)
{
    if (obj_count >= MAX_OBJECTS) { printf("  [!] Object limit reached.\n"); return; }
    int cx, cy, r;
    printf("  Enter centre (col row) and radius: ");
    scanf("%d %d %d", &cx, &cy, &r);
    if (r <= 0) { printf("  [!] Radius must be positive.\n"); return; }

    Object *o      = &objects[obj_count++];
    o->id          = next_id++;
    o->type        = TYPE_CIRCLE;
    o->active      = 1;
    o->data.circle = (Circle){ cx, cy, r };
    draw_object(o, DRAW);
    printf("  [+] Circle added (ID %d).\n", o->id);
}

void add_rectangle(void)
{
    if (obj_count >= MAX_OBJECTS) { printf("  [!] Object limit reached.\n"); return; }
    int x1, y1, x2, y2;
    printf("  Enter top-left (col row) and bottom-right (col row): ");
    scanf("%d %d %d %d", &x1, &y1, &x2, &y2);

    Object *o    = &objects[obj_count++];
    o->id        = next_id++;
    o->type      = TYPE_RECT;
    o->active    = 1;
    o->data.rect = (Rectangle){ x1, y1, x2, y2 };
    draw_object(o, DRAW);
    printf("  [+] Rectangle added (ID %d).\n", o->id);
}

void add_line(void)
{
    if (obj_count >= MAX_OBJECTS) { printf("  [!] Object limit reached.\n"); return; }
    int x1, y1, x2, y2;
    printf("  Enter start (col row) and end (col row): ");
    scanf("%d %d %d %d", &x1, &y1, &x2, &y2);

    Object *o    = &objects[obj_count++];
    o->id        = next_id++;
    o->type      = TYPE_LINE;
    o->active    = 1;
    o->data.line = (Line){ x1, y1, x2, y2 };
    draw_object(o, DRAW);
    printf("  [+] Line added (ID %d).\n", o->id);
}

void add_triangle(void)
{
    if (obj_count >= MAX_OBJECTS) { printf("  [!] Object limit reached.\n"); return; }
    int x1, y1, x2, y2, x3, y3;
    printf("  Enter vertex A (col row): ");
    scanf("%d %d", &x1, &y1);
    printf("  Enter vertex B (col row): ");
    scanf("%d %d", &x2, &y2);
    printf("  Enter vertex C (col row): ");
    scanf("%d %d", &x3, &y3);

    Object *o   = &objects[obj_count++];
    o->id       = next_id++;
    o->type     = TYPE_TRIANGLE;
    o->active   = 1;
    o->data.tri = (Triangle){ x1, y1, x2, y2, x3, y3 };
    draw_object(o, DRAW);
    printf("  [+] Triangle added (ID %d).\n", o->id);
}

/* ═══════════════════════════════════════════════════════════
 *  DELETE operation
 * ═══════════════════════════════════════════════════════════ */

void delete_object(void)
{
    list_objects();
    int id;
    printf("  Enter object ID to delete (0 to cancel): ");
    scanf("%d", &id);
    if (id == 0) return;

    int idx = find_object(id);
    if (idx == -1) { printf("  [!] Object ID %d not found.\n", id); return; }

    objects[idx].active = 0;
    canvas_rebuild();
    printf("  [-] Object ID %d deleted.\n", id);
}

/* ═══════════════════════════════════════════════════════════
 *  MODIFY operation
 * ═══════════════════════════════════════════════════════════ */

void modify_object(void)
{
    list_objects();
    int id;
    printf("  Enter object ID to modify (0 to cancel): ");
    scanf("%d", &id);
    if (id == 0) return;

    int idx = find_object(id);
    if (idx == -1) { printf("  [!] Object ID %d not found.\n", id); return; }

    Object *o = &objects[idx];

    /* Temporarily erase then re-read parameters */
    o->active = 0;
    canvas_rebuild();

    printf("  Modifying %s (ID %d) — enter new parameters:\n",
           (o->type == TYPE_CIRCLE)   ? "Circle"    :
           (o->type == TYPE_RECT)     ? "Rectangle" :
           (o->type == TYPE_LINE)     ? "Line"      : "Triangle",
           id);

    switch (o->type) {
        case TYPE_CIRCLE:
            printf("  Enter centre (col row) and radius: ");
            scanf("%d %d %d", &o->data.circle.cx,
                               &o->data.circle.cy,
                               &o->data.circle.radius);
            if (o->data.circle.radius <= 0) {
                printf("  [!] Invalid radius — modification cancelled.\n");
                o->active = 1; canvas_rebuild(); return;
            }
            break;
        case TYPE_RECT:
            printf("  Enter top-left (col row) and bottom-right (col row): ");
            scanf("%d %d %d %d", &o->data.rect.x1, &o->data.rect.y1,
                                  &o->data.rect.x2, &o->data.rect.y2);
            break;
        case TYPE_LINE:
            printf("  Enter start (col row) and end (col row): ");
            scanf("%d %d %d %d", &o->data.line.x1, &o->data.line.y1,
                                  &o->data.line.x2, &o->data.line.y2);
            break;
        case TYPE_TRIANGLE:
            printf("  Enter vertex A (col row): ");
            scanf("%d %d", &o->data.tri.x1, &o->data.tri.y1);
            printf("  Enter vertex B (col row): ");
            scanf("%d %d", &o->data.tri.x2, &o->data.tri.y2);
            printf("  Enter vertex C (col row): ");
            scanf("%d %d", &o->data.tri.x3, &o->data.tri.y3);
            break;
    }

    o->active = 1;
    canvas_rebuild();
    printf("  [*] Object ID %d modified.\n", id);
}

/* ═══════════════════════════════════════════════════════════
 *  CLEAR canvas
 * ═══════════════════════════════════════════════════════════ */

void clear_all(void)
{
    char confirm;
    printf("  Are you sure you want to clear everything? (y/n): ");
    scanf(" %c", &confirm);
    if (confirm != 'y' && confirm != 'Y') { printf("  Cancelled.\n"); return; }

    canvas_init();
    obj_count = 0;
    next_id   = 1;
    printf("  [x] Canvas cleared.\n");
}

/* ═══════════════════════════════════════════════════════════
 *  Menu & main
 * ═══════════════════════════════════════════════════════════ */

void print_banner(void)
{
    printf("\n");
    printf("  ╔══════════════════════════════════════════════════╗\n");
    printf("  ║       2D GRAPHICS EDITOR  —  C / ASCII Art       ║\n");
    printf("  ║   Canvas : %2d rows × %2d cols  |  Draw char : *   ║\n", ROWS, COLS);
    printf("  ╚══════════════════════════════════════════════════╝\n");
}

void print_menu(void)
{
    printf("\n");
    printf("  ┌─────────────────────────────────────────────────┐\n");
    printf("  │                  MAIN  MENU                     │\n");
    printf("  ├─────────────────────────────────────────────────┤\n");
    printf("  │  ADD OBJECTS                                     │\n");
    printf("  │   1. Draw Circle                                 │\n");
    printf("  │   2. Draw Rectangle                             │\n");
    printf("  │   3. Draw Line                                   │\n");
    printf("  │   4. Draw Triangle                               │\n");
    printf("  ├─────────────────────────────────────────────────┤\n");
    printf("  │  MANAGE OBJECTS                                  │\n");
    printf("  │   5. Delete Object                               │\n");
    printf("  │   6. Modify Object                               │\n");
    printf("  │   7. List Objects                                │\n");
    printf("  ├─────────────────────────────────────────────────┤\n");
    printf("  │  CANVAS                                          │\n");
    printf("  │   8. Display Canvas                              │\n");
    printf("  │   9. Clear Canvas                                │\n");
    printf("  ├─────────────────────────────────────────────────┤\n");
    printf("  │   0. Exit                                        │\n");
    printf("  └─────────────────────────────────────────────────┘\n");
    printf("  Choice: ");
}

int main(void)
{
    canvas_init();
    print_banner();

    int choice;
    do {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            /* flush bad input */
            while (getchar() != '\n');
            choice = -1;
        }

        switch (choice) {
            case 1: add_circle();    break;
            case 2: add_rectangle(); break;
            case 3: add_line();      break;
            case 4: add_triangle();  break;
            case 5: delete_object(); break;
            case 6: modify_object(); break;
            case 7: list_objects();  break;
            case 8: canvas_display(); break;
            case 9: clear_all();     break;
            case 0:
                printf("\n  Goodbye! Thanks for using 2D Graphics Editor.\n\n");
                break;
            default:
                printf("  [!] Invalid choice. Please enter 0–9.\n");
        }
    } while (choice != 0);

    return 0;
}