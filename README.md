# 2D Graphics Editor in C

A simple console-based 2D Graphics Editor developed in C that allows users to draw and manage geometric shapes on an ASCII canvas. The project demonstrates fundamental concepts of Computer Graphics, Data Structures, and C Programming through the implementation of drawing algorithms and object management techniques.

---

## Project Overview

This application provides a text-based graphics environment where users can create and manipulate graphical objects on a 2D canvas.

The editor supports:

* Drawing Circles
* Drawing Rectangles
* Drawing Lines
* Drawing Triangles
* Modifying Existing Objects
* Deleting Existing Objects
* Listing All Objects
* Displaying the Canvas
* Clearing the Canvas

The canvas is represented using a 2D character array and rendered directly in the terminal using ASCII characters.

---

## Features

### Shape Creation

Users can create:

* Circle
* Rectangle
* Line
* Triangle

Each object is assigned a unique ID for future operations.

### Object Management

* Modify object parameters
* Delete objects
* List all active objects
* Track objects using unique IDs

### Canvas Operations

* Display current canvas
* Rebuild canvas after modifications
* Clear all objects and reset the workspace

### Graphics Algorithms

The project implements classic computer graphics algorithms:

#### Bresenham's Line Algorithm

Used for efficient line drawing.

#### Midpoint Circle Algorithm

Used for accurate circle generation.

---

## Technologies Used

* Programming Language: C
* Concepts:

  * Structures
  * Unions
  * Arrays
  * Functions
  * Graphics Algorithms
  * Object Management
  * ASCII Rendering

---

## Project Structure

```text
main()
│
├── Canvas Management
│   ├── canvas_init()
│   ├── canvas_display()
│   ├── canvas_rebuild()
│   └── put_pixel()
│
├── Drawing Algorithms
│   ├── draw_line_algo()
│   ├── draw_circle_algo()
│   ├── draw_rectangle_algo()
│   └── draw_triangle_algo()
│
├── Object Operations
│   ├── add_circle()
│   ├── add_rectangle()
│   ├── add_line()
│   ├── add_triangle()
│   ├── delete_object()
│   ├── modify_object()
│   └── list_objects()
│
└── Helper Functions
    ├── find_object()
    └── draw_object()
```

---

## Data Structures Used

### Circle

```c
typedef struct {
    int cx, cy, radius;
} Circle;
```

### Rectangle

```c
typedef struct {
    int x1, y1, x2, y2;
} Rectangle;
```

### Line

```c
typedef struct {
    int x1, y1, x2, y2;
} Line;
```

### Triangle

```c
typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} Triangle;
```

### Union for Shape Storage

```c
typedef union {
    Circle circle;
    Rectangle rect;
    Line line;
    Triangle tri;
} ShapeData;
```

### Object Structure

```c
typedef struct {
    int id;
    int type;
    ShapeData data;
    int active;
} Object;
```

---

## How It Works

1. User selects an operation from the menu.
2. Shape information is collected.
3. An object is created and stored.
4. The appropriate drawing algorithm is executed.
5. Pixels are placed on the canvas using `put_pixel()`.
6. The canvas is displayed in ASCII format.
7. Any modification or deletion triggers a canvas rebuild.

---

## Compilation

Using GCC:

```bash
gcc graphics_editor.c -o graphics_editor -lm
```

---

## Run

```bash
./graphics_editor
```

---

## Sample Menu

```text
1. Draw Circle
2. Draw Rectangle
3. Draw Line
4. Draw Triangle
5. Delete Object
6. Modify Object
7. List Objects
8. Display Canvas
9. Clear Canvas
0. Exit
```

---

## Learning Outcomes

This project helps in understanding:

* C Programming Fundamentals
* Structures and Unions
* Function-Based Modular Design
* Computer Graphics Algorithms
* Memory Management Concepts
* Object Tracking Systems
* ASCII-Based Rendering Techniques

---

## Future Improvements

Potential enhancements include:

* Filled Shapes
* Color Support
* Shape Movement
* Save and Load Canvas
* Undo and Redo Functionality
* Mouse-Based GUI Version
* Dynamic Memory Allocation
* Linked List Based Object Storage

---

## Conclusion

The 2D Graphics Editor is a practical implementation of fundamental C programming and computer graphics concepts. Through the use of structures, unions, arrays, functions, and classic drawing algorithms such as Bresenham's Line Algorithm and the Midpoint Circle Algorithm, the project demonstrates how graphical objects can be created and managed in a console-based environment.

This project serves as an excellent learning platform for understanding object management, coordinate systems, canvas rendering, and algorithm-based drawing techniques. It also provides a strong foundation for developing more advanced graphics applications and GUI-based drawing tools in the future.

---

## Author

**Mohammed Ayaan**

First Year Engineering Student.

##
