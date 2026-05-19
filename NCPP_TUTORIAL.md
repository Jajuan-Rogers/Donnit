# Notcurses C++ Bindings (ncpp) — Complete Guide

> [!abstract] About This Document
> This guide covers the **philosophy, architecture, and hands-on usage** of the C++ bindings for [Notcurses](https://github.com/dankamongmen/notcurses) via the `ncpp` namespace. It is written for developers who are comfortable with C++ basics but new to terminal UI programming. By the end, you will understand how to build rich, interactive TUIs (Terminal User Interfaces) without relying on legacy curses idioms.
>
> All code examples in this document use **only the documented `ncpp` C++ API**. Where the underlying C API differs, this is explicitly noted.

---

## Table of Contents

- [[#1. What Is Notcurses?]]
- [[#2. Prerequisites — What You Need to Know First]]
- [[#3. The Philosophy — Notcurses vs ncurses]]
- [[#4. Key Concept — RAII in C++]]
- [[#5. The Plane Model — Thinking in Layers]]
- [[#6. Application Lifecycle]]
- [[#7. Core Classes Reference]]
	- [[#7.1 ncpp::NotCurses — The Context Manager]]
	- [[#7.2 ncpp::Plane — The Canvas]]
	- [[#7.3 ncpp::Cell — The Glyph]]
- [[#8. Colours and Styling]]
- [[#9. Input Handling]]
- [[#10. A Complete Working Example]]
- [[#11. Compiling Your Program]]
- [[#12. Common Pitfalls]]
- [[#13. Quick Reference Cheatsheet]]

---

## 1. What Is Notcurses?

**Notcurses** is a modern C library (with C++ bindings) for building rich, full-featured terminal applications. Think of it as the spiritual successor to `ncurses`, rebuilt from scratch to take advantage of ==everything modern terminals can do==.

With Notcurses you can render full **24-bit truecolor** (16 million colours rather than the legacy 256), **alpha-blended, layered planes** that composite like transparent sheets of glass, **Unicode and emoji** as first-class citizens, and even **images and video** directly in the terminal via sixel or kitty pixel protocols.

> [!info] The C++ Wrapper (`ncpp`)
> The raw Notcurses API is written in C. The `ncpp` namespace provides idiomatic C++ wrappers around every major C type, giving you RAII constructors and destructors, method syntax on objects, and type safety — instead of raw pointer juggling and function prefixes like `ncplane_putstr(plane, ...)`. **This guide focuses entirely on the `ncpp` C++ API.**

---

## 2. Prerequisites — What You Need to Know First

Before reading further, make sure you are comfortable with the following C++ concepts, because they underpin how `ncpp` works at every level.

| Concept | Why It Matters Here |
|---|---|
| Stack vs. heap allocation | Planes are often stack-allocated; understanding scope is essential |
| Object scope and lifetime | The entire cleanup model relies on objects going out of scope |
| Structs and zero-initialisation | Notcurses options are configured via C-style structs |
| Pointers and references | The standard plane is returned as a raw `ncpp::Plane*` |
| `#include` and linking | You will link against `notcurses++` separately from `notcurses` |

> [!tip] You Don't Need to Know ncurses
> In fact, knowing `ncurses` might give you counterproductive habits. Notcurses is a fundamentally different mental model — start fresh rather than trying to map concepts across.

---

## 3. The Philosophy — Notcurses vs ncurses

> [!quote] "Notcurses is not curses."
> This is stated directly in the library's own documentation, and it is the single most important thing to internalise before writing a line of code.

### The Old Way (ncurses)

`ncurses` was designed in the 1980s for terminals that had severe limitations. It relies on a **single global state machine** with one `stdscr` for your whole program, a **limited colour palette** of 8 or 16 colours, and a **flat surface** with no concept of layering or compositing.

### The New Way (Notcurses)

| Feature | ncurses | Notcurses |
|---|---|---|
| State | Global singleton | Explicit context object (`ncpp::NotCurses`) |
| Colours | 8–256 | Full 24-bit RGB (16 million colours) |
| Layering | None | Unlimited `ncplane` stack along a Z-axis |
| Unicode | Limited | Full grapheme cluster support |
| Images | No | Yes (sixel, kitty, braille art) |
| Thread safety | Dangerous | Much safer — no hidden global state |
| Rendering | Per-character updates | Damage-detected frame compositing |

### The Four Core Tenets

> [!important] Design Principles You Must Internalise
> 1. **No Global State** — You create an explicit `ncpp::NotCurses` context and pass it where it is needed. There are no hidden globals.
> 2. **The Z-Axis (Planes)** — Everything is drawn onto `ncpp::Plane` objects that stack like transparent sheets of glass, composited into one image at render time.
> 3. **Lazy Rendering** — Drawing to a plane only changes an in-memory buffer. The terminal screen is only updated when you explicitly call `render()`.
> 4. **RAII Lifetime Management** — Objects clean up after themselves when they go out of scope. You do not manually destroy planes or stop the context.

---

## 4. Key Concept — RAII in C++

Because RAII is so central to how `ncpp` works, it is worth understanding clearly before proceeding.

**RAII** stands for **Resource Acquisition Is Initialization**. The principle is that when an object is **created**, it acquires its resource, and when it is **destroyed** — meaning it goes out of scope — it automatically releases that resource by running its destructor.

```cpp
// A familiar RAII example with file I/O
{
    std::ifstream file("data.txt");  // File is opened here — resource acquired
    // ... work with file ...
}  // file goes out of scope; destructor runs; file is closed automatically
```

The `ncpp` library applies exactly this pattern to your terminal session and to every plane you create:

```cpp
{
    ncpp::NotCurses nc(opts);   // Library initialised; terminal taken over
    // ... your application runs ...
}   // nc destructor calls notcurses_stop(); terminal fully restored
```

And the same applies to individual planes:

```cpp
{
    ncpp::Plane floating(std, popts);  // Plane created and registered
    // ... draw on it, render it ...
}   // floating goes out of scope; underlying ncplane automatically destroyed
```

> [!success] Why This Matters for You
> You never have to manually call `notcurses_stop()` or `ncplane_destroy()`. Just let your objects leave scope. This also means your program cleans up correctly even if an exception is thrown — the destructors still run. This is one of the most powerful advantages of the C++ bindings over using the raw C API.

---

## 5. The Plane Model — Thinking in Layers

This is the central architectural concept of Notcurses. It is worth spending time here before touching any code.

### Imagine a Stack of Glass Sheets

Picture a series of **transparent glass sheets** stacked above your terminal, each of which you can draw on independently:

```
        ┌──────────────────────────────────────────────┐
Top     │    🔴  "Alert!" plane  (small, floating)     │  ← highest z-order
        └──────────────────────────────────────────────┘
        ┌──────────────────────────────────────────────┐
Middle  │      Blue sidebar plane (partial width)      │
        └──────────────────────────────────────────────┘
        ┌──────────────────────────────────────────────┐
Bottom  │       Standard Plane  (full terminal)        │  ← always at the base
        └──────────────────────────────────────────────┘
              ↓ ↓ ↓    nc.render()    ↓ ↓ ↓
        ┌──────────────────────────────────────────────┐
Screen  │      Composited result — what you see        │
        └──────────────────────────────────────────────┘
```

Each plane is independently positioned, independently sized, and drawn into in memory. Nothing appears on screen until `render()` is called and Notcurses composites all the sheets into one image.

### The Standard Plane

The **Standard Plane** is special. It always exists, always fills the full terminal dimensions, and sits at the very bottom of the Z-stack. It is owned entirely by the `NotCurses` context and ==cannot be destroyed manually==. Think of it as the background canvas that is permanently present.

### Parent-Child Relationships

Every plane (except the standard plane) must have a **parent**. A child plane's position is always ==relative to its parent's top-left corner==, not the physical terminal screen — unless the parent happens to be the standard plane, in which case the two coordinate spaces coincide.

```
Standard Plane at terminal (0, 0) — full terminal size
    └── Sidebar Plane at .y=0, .x=0 (relative to standard plane)
            └── Scrollbar Plane at .y=0, .x=18 (relative to sidebar)
```

> [!info] Moving a Child Moves It Relative to Its Parent
> If your sub-plane's parent begins at terminal position (10, 5) and you call `move(2, 3)` on the sub-plane, it will appear at terminal position (12, 8). The coordinates you give are always offsets from the parent, not absolute screen coordinates.

---

## 6. Application Lifecycle

Every Notcurses application follows this predictable lifecycle. Understanding it upfront will save you a great deal of confusion.

```
┌─────────────────────────────────────────────────┐
│  1. INITIALIZE                                   │
│     ncpp::NotCurses nc(opts)                     │
│     Takes control of the terminal                │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────┐
│  2. ACQUIRE BASE PLANE                           │
│     ncpp::Plane* std = nc.get_stdplane()         │
│     Get a pointer to the root canvas             │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────┐
│  3. CONSTRUCT PLANES                             │
│     ncpp::Plane child(std, popts)                │
│     Create your UI surfaces, bound to a parent   │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────┐
│  4. MUTATE                                       │
│     putstr(), set_fg_rgb(), move(), erase() ...  │
│     Write text, update positions, change colours │
│     ⚠ Nothing is visible on screen yet          │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────┐
│  5. RENDER                                       │
│     nc.render()                                  │
│     Composites all planes; flushes to terminal   │
└──────────────────┬──────────────────────────────┘
                   │
         ┌─────────▼─────────┐
         │  Continue looping? │──Yes──► Back to step 4
         └─────────┬─────────┘
                   │ No
┌──────────────────▼──────────────────────────────┐
│  6. DESTROY (automatic via RAII)                 │
│     Planes destroyed as they leave scope         │
│     nc destructor calls notcurses_stop()         │
│     Terminal fully restored                      │
└─────────────────────────────────────────────────┘
```

---

## 7. Core Classes Reference

### 7.1 `ncpp::NotCurses` — The Context Manager

This is the ==root object== of your entire application. You create exactly one per terminal session, and it manages the entire lifecycle of the library.

```cpp
#include <notcurses/notcurses.hh>

int main() {
    // Zero-initialise the options struct so all fields start at safe defaults.
    // A fully zeroed struct gives you the alternate screen and clears on exit.
    notcurses_options opts = {};

    // Example: prevent Notcurses from switching to the alternate screen.
    // Your output will appear inline in the shell's scrollback buffer instead.
    opts.flags = NCOPTION_NO_ALTERNATE_SCREEN;

    ncpp::NotCurses nc(opts);

    // Your application lives here.

    return 0;
}   // nc goes out of scope; notcurses_stop() is called automatically.
    // The terminal is fully restored — no manual teardown needed.
```

#### The `notcurses_options` Flags

The `flags` field in the options struct is a bitfield. You can combine multiple flags with the bitwise OR operator (`|`).

| Flag | Effect |
|---|---|
| `NCOPTION_NO_ALTERNATE_SCREEN` | Renders inline rather than hijacking the terminal buffer |
| `NCOPTION_SUPPRESS_BANNERS` | Hides the startup and shutdown performance banners |
| `NCOPTION_INHIBIT_SETLOCALE` | Prevents the library from calling `setlocale()` automatically |
| `NCOPTION_NO_CLEAR_BITMAPS` | Skips clearing pixel/bitmap content on exit |

#### Core Methods

**`get_stdplane()`** returns a raw pointer to the Standard Plane. Use this as the parent for all of your top-level UI planes.

```cpp
ncpp::Plane* std = nc.get_stdplane();
```

> [!warning] Never `delete` the Standard Plane
> The Standard Plane is owned by the `NotCurses` context. It is not a heap-allocated object that you are responsible for. Calling `delete` on it is undefined behaviour and will corrupt or crash your program.

---

**`render()`** is the function that makes everything visible. It computes the visual result of all currently stacked planes, compares it to what is currently on screen (damage detection), and only sends the differences to the terminal. This makes rendering fast and flicker-free.

```cpp
nc.render();
```

> [!tip] Call `render()` Once Per Frame
> In a typical event loop, you mutate your planes as needed in response to input, then call `render()` exactly once to present the updated frame. Calling it after every individual draw call is wasteful and defeats the entire purpose of the staged compositing model.

---

**`getc(bool blocking, ncinput* ni)`** reads one input event. It returns a `char32_t` — a 32-bit value that is either a Unicode codepoint for printable characters, or a special `NCKEY_*` constant for function keys, arrow keys, mouse events, and terminal resize signals.

```cpp
ncinput ni;

// Pass true to block until the user does something
char32_t key = nc.getc(true, &ni);

// Pass false to return immediately with 0 if no input is waiting
char32_t key = nc.getc(false, &ni);
```

---

### 7.2 `ncpp::Plane` — The Canvas

Planes are where all actual drawing happens. Every plane except the standard plane must be explicitly bound to a parent at construction time.

#### Creating a Plane

You first describe the plane's geometry in an `ncplane_options` struct, then construct the `ncpp::Plane` object by passing the parent pointer and the options together.

```cpp
ncplane_options popts = {
    .y        = 2,            // Row offset relative to the parent plane's top-left
    .x        = 2,            // Column offset relative to the parent plane's top-left
    .rows     = 10,           // Height in terminal rows
    .cols     = 40,           // Width in terminal columns
    .userptr  = nullptr,      // Optional: attach any pointer you like (arbitrary user data)
    .name     = "my_plane",   // A debug name — useful when inspecting plane trees
    .resizecb = nullptr,      // Optional: function called automatically on terminal resize
    .flags    = 0             // Bitfield of NCPLANE_OPTION_* flags
};

// The first argument is the parent plane pointer.
// All position values in popts are interpreted relative to this parent.
ncpp::Plane my_plane(nc.get_stdplane(), popts);
```

> [!example] What `.y` and `.x` Actually Mean
> If you pass `nc.get_stdplane()` as the parent, the standard plane occupies the full terminal starting at (0, 0). Setting `.y = 2, .x = 2` means your new plane's top-left corner will appear at terminal row 2, column 2. If the parent were a smaller plane starting at terminal (5, 10), the new plane would appear at terminal (7, 12). The math is always: *terminal position = parent's terminal position + your offset*.

#### Writing Text

**`putstr(int y, int x, const char* str)`** writes a string at specific coordinates local to this plane. Remember that these coordinates are relative to the plane itself, not to the terminal.

```cpp
// Write at row 0, column 0 of THIS plane (its own top-left corner)
my_plane.putstr(0, 0, "Hello from the top-left of this plane!");

// Write further down and in from the plane's edge
my_plane.putstr(3, 4, "Indented on row 3");
```

You can also call `putstr` without coordinates, and it will write starting from the plane's current internal cursor position:

```cpp
// Writes starting wherever the cursor currently sits on this plane
my_plane.putstr("Continues from the last cursor position");
```

> [!info] Coordinates Are Always Local to the Plane
> `putstr(0, 0, ...)` writes to the top-left of *this plane*, not the terminal. If the plane is positioned at terminal row 10, column 5, writing at plane position (0, 0) causes text to appear at terminal (10, 5). You never think in absolute terminal coordinates when drawing — only in plane-local coordinates.

---

#### Setting Colours

**`set_fg_rgb(unsigned rgb)`** and **`set_bg_rgb(unsigned rgb)`** set the foreground text colour and background cell colour respectively for all subsequent write operations on this plane. The colour is specified as a `0xRRGGBB` hex value — exactly the same format as HTML/CSS colours.

```cpp
my_plane.set_fg_rgb(0xFF6600);   // Orange text
my_plane.set_bg_rgb(0x1E1E2E);   // Very dark navy background

// Both of these will be orange text on a dark navy background
my_plane.putstr(0, 0, "First line");
my_plane.putstr(1, 0, "Second line");

// Change only the foreground — background remains dark navy
my_plane.set_fg_rgb(0x00FF88);
my_plane.putstr(2, 0, "Third line — now in green");
```

> [!info] Colour State Is Sticky and Per-Plane
> Once you set a colour on a plane, it stays set until you explicitly change it. Changing the colour on one plane has absolutely no effect on any other plane's colour state. Each plane maintains its own current fg/bg colour independently.

---

#### Moving and Resizing

**`move(int y, int x)`** repositions the plane relative to its parent. This is how you implement animations, slide-in panels, and dynamic layout changes.

```cpp
// Slide a plane smoothly from left to right
for (int x = 0; x < 50; x++) {
    my_plane.move(5, x);   // New position, relative to parent
    nc.render();
    usleep(30000);          // 30ms pause between frames
}
```

**`resize(int rows, int cols)`** changes the plane's dimensions. Be aware that content outside the new bounds is lost if you shrink the plane.

```cpp
my_plane.resize(20, 60);   // Grow to 20 rows tall, 60 columns wide
```

---

#### Clearing a Plane

**`erase()`** wipes all content from a plane, replacing every cell with a fully transparent empty cell. The plane itself continues to exist and can be drawn on again immediately.

```cpp
my_plane.erase();
```

> [!warning] `erase()` Makes Cells Transparent, Not Black
> An erased cell is fully transparent in the compositing model — whatever plane lies beneath it will show through after the next `render()`. If you want a solid colour fill, you need to set your background colour with `set_bg_rgb()` and then write content to the cells (or `erase()` after setting the background, since `erase()` replaces cells with the plane's current background).

---

### 7.3 `ncpp::Cell` — The Glyph

An `ncpp::Cell` represents a **single character position** on a plane, with complete control over its Unicode grapheme cluster, foreground colour, background colour, and style attributes. While `putstr` is convenient for writing strings, `Cell` is what you reach for when you want ==precise, reusable control over individual glyphs== — particularly when working with emoji or complex Unicode sequences.

#### The Correct Ownership Model

The most important thing to get right with `Cell` is understanding *which object the methods belong to*. This trips up almost every beginner.

The `load` and `release` methods are called ==on the cell object itself==. The plane is passed in as an argument, because the plane is the entity that actually owns the backing memory for the grapheme cluster. Think of the cell as a handle and the plane as the memory manager.

```cpp
// 1. Declare an empty cell
ncpp::Cell my_cell;

// 2. Load a UTF-8 grapheme INTO the cell.
//    IMPORTANT: load() is a method on the CELL, not the plane.
//    You pass a POINTER to the plane as the argument.
my_cell.load(&my_plane, "🚀");    // Emoji work as grapheme clusters
my_cell.load(&my_plane, "█");     // Box-drawing characters work fine
my_cell.load(&my_plane, "A");     // Plain ASCII works fine

// 3. Set colours specific to this cell, independently of the plane's state
my_cell.set_fg_rgb(0xFF6600);     // Orange foreground for this cell
my_cell.set_bg_rgb(0x1E1E2E);     // Dark background for this cell

// 4. ... use the cell in your drawing operations ...

// 5. Release the cell when done.
//    IMPORTANT: release() is also a method on the CELL, not the plane.
//    Again, you pass a pointer to the plane as the argument.
my_cell.release(&my_plane);
```

> [!warning] Always Call `release()` — and Call It on the Cell
> Complex Unicode grapheme clusters (multi-codepoint emoji, ZWJ sequences, combining characters) require heap memory that is managed by the plane. The cell holds only a reference into that memory. If you call `load` and never call `release`, you leak that memory. Every `load` must have a matching `release`, and both are called on the **cell**, not on the plane.

#### When to Use `Cell` vs `putstr`

`putstr` is your workhorse for writing text and should be your default. Reach for `Cell` when you need to apply individual per-cell styling that is independent of the plane's current colour state, or when you are working with complex emoji or multi-codepoint grapheme clusters and need fine-grained control over a single character position.

---

## 8. Colours and Styling

### The Colour Model — 24-bit Truecolor

Notcurses uses **24-bit RGB** — the same colour model as HTML and CSS. Colours are expressed as unsigned integer values in `0xRRGGBB` format, giving you access to all 16 million colours that modern terminals support. A useful mental model: any colour you can express as a CSS hex code like `#FF6600` can be used in Notcurses as `0xFF6600`.

```cpp
my_plane.set_fg_rgb(0xFF0000);   // Pure red
my_plane.set_fg_rgb(0x00FF00);   // Pure green
my_plane.set_fg_rgb(0x0000FF);   // Pure blue
my_plane.set_fg_rgb(0xB4BEFE);   // Soft lavender (Catppuccin Lavender)
my_plane.set_bg_rgb(0x1E1E2E);   // Very dark navy (Catppuccin Base)
```

### Foreground vs Background

The foreground colour applies to the ==glyph itself== — the actual character being drawn. The background colour applies to the ==entire cell behind the glyph==, filling any space not covered by the character. When cells are transparent (as after `erase()`), the plane beneath shows through instead.

```cpp
my_plane.set_fg_rgb(0xCDD6F4);   // Light text colour
my_plane.set_bg_rgb(0x313244);   // Slightly lighter dark background
my_plane.putstr(1, 1, " Status: OK ");   // Text with a visible block of background colour
```

### Applying Colours at the Cell Level

You can also set colours directly on an `ncpp::Cell` rather than on the plane. This lets a specific cell have different colours from the plane's current defaults, without permanently altering the plane's colour state:

```cpp
ncpp::Cell special;
special.load(&my_plane, "★");
special.set_fg_rgb(0xF9E2AF);   // Gold star, regardless of the plane's current fg colour
special.set_bg_rgb(0x1E1E2E);   // Matching dark background
// ... place the cell ...
special.release(&my_plane);
```

---

## 9. Input Handling

### Reading Keys with `getc`

Input in Notcurses is event-driven. You call `getc` on your `NotCurses` context, and it hands you one event at a time. The return type is `char32_t` — a 32-bit character that is either a Unicode codepoint for ordinary keys, or a special `NCKEY_*` constant for everything else.

```cpp
ncinput ni;   // Struct that is populated with metadata about the event

// Blocking call — execution pauses here until input arrives
char32_t key = nc.getc(true, &ni);

// Non-blocking call — returns 0 immediately if nothing is waiting
char32_t key = nc.getc(false, &ni);

// Act on the result
if (key == 'q') {
    // User pressed the letter q
} else if (key == NCKEY_UP) {
    // User pressed the up arrow key
} else if (key == NCKEY_ENTER) {
    // User pressed Enter
}
```

### The `ncinput` Struct — Event Metadata

The `ncinput` struct that you pass to `getc` is populated with detailed information about the event beyond just which key was pressed. This is how you detect modifier keys and, for mouse events, the coordinates of the click.

```cpp
ncinput ni;
char32_t key = nc.getc(true, &ni);

if (ni.ctrl) {
    // Control was held during this event — e.g. Ctrl+C if key == 'c'
}
if (ni.alt) {
    // Alt (or Meta) was held
}

// For mouse events, ni.y and ni.x hold the click position
// in absolute terminal row and column coordinates
if (key == NCKEY_BUTTON1) {
    // Left mouse button clicked at (ni.y, ni.x)
}
```

### Special Key Constants

| Constant | Key |
|---|---|
| `NCKEY_UP` | ↑ Arrow |
| `NCKEY_DOWN` | ↓ Arrow |
| `NCKEY_LEFT` | ← Arrow |
| `NCKEY_RIGHT` | → Arrow |
| `NCKEY_ENTER` | Enter / Return |
| `NCKEY_BACKSPACE` | Backspace |
| `NCKEY_DEL` | Delete |
| `NCKEY_F01` through `NCKEY_F60` | Function keys |
| `NCKEY_PGUP` / `NCKEY_PGDOWN` | Page Up / Page Down |
| `NCKEY_HOME` / `NCKEY_END` | Home / End |
| `NCKEY_RESIZE` | Terminal was resized by the user |
| `NCKEY_BUTTON1` | Mouse left-click |
| `NCKEY_BUTTON3` | Mouse right-click |

### A Standard Event Loop

Most Notcurses applications follow the same basic structure: read one input event, update application state, redraw the affected planes, then render exactly once.

```cpp
bool running = true;

while (running) {
    // Block until the user does something
    ncinput ni;
    char32_t key = nc.getc(true, &ni);

    // React to the input by updating application state
    if (key == 'q' || key == 'Q') {
        running = false;
    } else if (key == NCKEY_UP) {
        selected_row--;
    } else if (key == NCKEY_DOWN) {
        selected_row++;
    } else if (key == NCKEY_RESIZE) {
        // The terminal was resized — re-query dimensions and reposition planes
    }

    // Redraw your planes to reflect the updated state
    // (your drawing functions go here)

    // Present the new frame — one render call per loop iteration
    nc.render();
}
```

---

## 10. A Complete Working Example

The following is a fully functional, annotated Notcurses C++ application that uses only the documented `ncpp` C++ binding API. It demonstrates the complete lifecycle: initialisation, the standard plane, child planes, colour and text, RAII scoped plane destruction, and an event loop.

```cpp
#include <notcurses/notcurses.hh>
#include <unistd.h>    // for sleep()

int main() {
    // ── 1. INITIALISE ────────────────────────────────────────────────────
    // Zero-initialise opts so all fields start at safe defaults.
    // By default, Notcurses uses the alternate screen and restores the
    // terminal automatically when the NotCurses context is destroyed.
    notcurses_options opts = {};
    ncpp::NotCurses nc(opts);


    // ── 2. GET THE STANDARD PLANE ────────────────────────────────────────
    // The standard plane always exists, fills the full terminal, and sits
    // at the bottom of the Z-stack. We NEVER delete this pointer.
    ncpp::Plane* std = nc.get_stdplane();

    // Write a header to the standard plane.
    // set_fg_rgb takes a 0xRRGGBB colour value.
    std->set_fg_rgb(0x00FF00);    // Bright green text
    std->putstr(0, 0, "Notcurses Demo — Standard Plane Background");

    // Add some explanatory text below the header
    std->set_fg_rgb(0x888888);    // Subdued grey
    std->putstr(2, 0, "The green text above is drawn on the standard plane.");
    std->putstr(3, 0, "The coloured box below is a separate child plane.");
    std->putstr(4, 0, "Press ENTER to dismiss the box.  Press Q to quit.");


    // ── 3. CREATE A CHILD PLANE IN A SCOPED BLOCK ────────────────────────
    // By placing this plane inside its own { } block, we guarantee that
    // the plane is automatically destroyed when execution leaves the block.
    // This is RAII in action — no manual cleanup call is needed.
    {
        ncplane_options popts = {
            .y        = 6,              // 6 rows from the standard plane's top
            .x        = 2,              // 2 columns from the standard plane's left
            .rows     = 7,
            .cols     = 44,
            .userptr  = nullptr,
            .name     = "floating_box",
            .resizecb = nullptr,
            .flags    = 0
        };

        // Parent is the standard plane pointer.
        // The child's .y and .x are relative to the standard plane's (0,0),
        // so in this case they are effectively absolute terminal coordinates.
        ncpp::Plane floating(std, popts);


        // ── 4. MUTATE THE CHILD PLANE ─────────────────────────────────────
        // Set background colour BEFORE calling erase(), so that erase()
        // fills every cell with the intended background rather than leaving
        // them transparent.
        floating.set_bg_rgb(0x00008B);    // Dark blue background
        floating.set_fg_rgb(0xFFFFFF);    // White text
        floating.erase();                  // Fill the entire plane with the bg colour

        // Write using plane-local coordinates.
        // (0,0) is the top-left of the floating plane, which appears
        // at terminal row 6, column 2.
        floating.putstr(1, 2, "I am a floating child plane!");
        floating.putstr(2, 2, "I sit above the standard plane.");
        floating.putstr(3, 2, "My background is dark blue.");

        // Change colour mid-plane to draw attention to a line
        floating.set_fg_rgb(0xFFFF00);    // Yellow text
        floating.putstr(5, 2, ">> Press ENTER to dismiss me");


        // ── 5. RENDER ─────────────────────────────────────────────────────
        // This is the FIRST point at which anything appears on screen.
        // Notcurses composites the standard plane and the floating plane,
        // then flushes the result to the terminal.
        nc.render();

        // Wait for the user to press Enter before we leave this scope
        ncinput ni;
        char32_t key;
        do {
            key = nc.getc(true, &ni);
        } while (key != NCKEY_ENTER && key != '\n');

    }   // ← 'floating' goes out of scope here.
        //   Its destructor is called automatically, destroying the underlying
        //   ncplane. The standard plane content is now the only thing visible.


    // Render again to show the floating plane is gone.
    // Update the standard plane with new instructions first.
    std->set_fg_rgb(0x00FF88);
    std->putstr(6, 0, "The floating plane was destroyed when it left scope.");
    std->putstr(7, 0, "Press Q to quit.");
    nc.render();

    // Simple quit loop — block until the user presses Q
    ncinput ni;
    while (true) {
        char32_t key = nc.getc(true, &ni);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }

    return 0;

    // ── 6. AUTOMATIC TEARDOWN ─────────────────────────────────────────────
    // 'nc' goes out of scope here. Its destructor calls notcurses_stop()
    // automatically, restoring the terminal to its previous state.
    // There is nothing to manually clean up.
}
```

---

## 11. Compiling Your Program

Notcurses ships with `pkg-config` metadata, which makes retrieving the correct compile flags and library paths completely automatic.

### With g++ or clang++

The C++ bindings require C++17 or later. Note that the `pkg-config` name for the C++ bindings is `notcurses++`, which is separate from the plain C `notcurses` package:

```bash
g++ -std=c++17 -o my_tui my_tui.cpp $(pkg-config --cflags --libs notcurses++)
```

```bash
clang++ -std=c++17 -o my_tui my_tui.cpp $(pkg-config --cflags --libs notcurses++)
```

### With CMake

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_tui)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NOTCURSES REQUIRED notcurses++)

add_executable(my_tui src/main.cpp)
target_include_directories(my_tui PRIVATE ${NOTCURSES_INCLUDE_DIRS})
target_link_libraries(my_tui PRIVATE ${NOTCURSES_LIBRARIES})
```

### Installing Notcurses

```bash
# Ubuntu / Debian
sudo apt install libnotcurses-dev libnotcurses++-dev

# Fedora / RHEL
sudo dnf install notcurses-devel

# Arch Linux
sudo pacman -S notcurses

# macOS (Homebrew)
brew install notcurses
```

> [!tip] Verify Your Installation
> Run `notcurses-demo` in your terminal after installing. It runs a full visual showcase of the library's capabilities and immediately confirms whether the installation is working correctly.

---

## 12. Common Pitfalls

> [!warning] Pitfall 1 — Drawing Without Rendering
> The most common mistake for beginners is to draw things and then be puzzled that nothing appears on screen. `putstr`, `set_fg_rgb`, `move`, and `erase` all operate on an in-memory buffer. The terminal is not touched until you call `render()`.
>
> ```cpp
> my_plane.set_fg_rgb(0x00FF00);
> my_plane.putstr(0, 0, "Hello!");
> // Nothing is visible yet. You must call:
> nc.render();
> // Now it appears.
> ```

> [!warning] Pitfall 2 — Deleting the Standard Plane
> The standard plane is owned by the `NotCurses` context. You did not allocate it and you must never attempt to `delete` it. Doing so is undefined behaviour.
>
> ```cpp
> ncpp::Plane* std = nc.get_stdplane();
> delete std;   // Never do this — crash or corruption will follow.
> ```

> [!warning] Pitfall 3 — Calling `load` and `release` on the Wrong Object
> This is the most common API confusion in `ncpp`. In the C++ bindings, `load` and `release` are methods on the **Cell**, not the plane. The plane is passed as a pointer argument. Getting this backwards is a compilation error, so the compiler will catch it — but it is worth understanding *why* the ownership works this way.
>
> ```cpp
> ncpp::Cell c;
>
> // WRONG — load and release do not exist as methods on Plane:
> // my_plane.load(&c, "A");
> // my_plane.release(&c);
>
> // CORRECT — load and release are methods on Cell:
> c.load(&my_plane, "A");
> // ... use c ...
> c.release(&my_plane);
> ```

> [!warning] Pitfall 4 — Forgetting to `release` Cells
> Complex Unicode grapheme clusters — emoji, ZWJ sequences, combining characters — require heap memory that is managed by the plane. The cell holds a reference into that memory. If you `load` and never `release`, you leak that memory on every call. Every `load` must have a matching `release`.
>
> ```cpp
> ncpp::Cell c;
> c.load(&my_plane, "🦀");
> // ... use c ...
> c.release(&my_plane);   // Never skip this line.
> ```

> [!warning] Pitfall 5 — Misunderstanding Plane-Local Coordinates
> Writing to `(0, 0)` on a plane writes to the top-left of *that plane*, not the terminal. If your plane is positioned at terminal row 10, column 5, then `putstr(0, 0, "Hi")` places text at terminal (10, 5) — not at the terminal origin.

> [!warning] Pitfall 6 — Colours Are Sticky
> Once you set a foreground or background colour on a plane, it remains set for all subsequent writes until you explicitly change it again. This is by design, but it surprises beginners who expect an automatic reset.
>
> ```cpp
> my_plane.set_fg_rgb(0xFF0000);   // Red
> my_plane.putstr(0, 0, "Red");
> my_plane.putstr(1, 0, "Also red — the colour carried forward!");
>
> my_plane.set_fg_rgb(0xFFFFFF);   // Explicitly set to white
> my_plane.putstr(2, 0, "Now white.");
> ```

> [!warning] Pitfall 7 — Ignoring `NCKEY_RESIZE`
> When the user resizes their terminal window, Notcurses delivers a `NCKEY_RESIZE` event from `getc`. If you ignore it, your layout will be wrong. At minimum, re-query dimensions from the standard plane and reposition any child planes whose size or location was calculated from the terminal dimensions.

---

## 13. Quick Reference Cheatsheet

### `ncpp::NotCurses` — The Context

| What You Want to Do | The Code |
|---|---|
| Initialise the library | `ncpp::NotCurses nc(opts);` |
| Get the standard plane | `ncpp::Plane* std = nc.get_stdplane();` |
| Flush all changes to the screen | `nc.render();` |
| Read input (blocking) | `char32_t k = nc.getc(true, &ni);` |
| Read input (non-blocking) | `char32_t k = nc.getc(false, &ni);` |

### `ncpp::Plane` — The Canvas

| What You Want to Do | The Code |
|---|---|
| Create a plane | `ncpp::Plane p(parent_ptr, popts);` |
| Write text at a position | `p.putstr(row, col, "text");` |
| Write at the current cursor | `p.putstr("text");` |
| Set foreground colour | `p.set_fg_rgb(0xRRGGBB);` |
| Set background colour | `p.set_bg_rgb(0xRRGGBB);` |
| Move the plane (relative to parent) | `p.move(new_y, new_x);` |
| Resize the plane | `p.resize(new_rows, new_cols);` |
| Clear all content (make transparent) | `p.erase();` |

### `ncpp::Cell` — The Glyph

| What You Want to Do | The Code |
|---|---|
| Declare an empty cell | `ncpp::Cell c;` |
| Load a grapheme (method on Cell) | `c.load(&my_plane, "🚀");` |
| Set the cell's foreground colour | `c.set_fg_rgb(0xRRGGBB);` |
| Set the cell's background colour | `c.set_bg_rgb(0xRRGGBB);` |
| Release cell memory (method on Cell) | `c.release(&my_plane);` |

### Input Key Constants

| Key | Constant |
|---|---|
| Arrow keys | `NCKEY_UP`, `NCKEY_DOWN`, `NCKEY_LEFT`, `NCKEY_RIGHT` |
| Enter | `NCKEY_ENTER` |
| Backspace | `NCKEY_BACKSPACE` |
| Delete | `NCKEY_DEL` |
| Function keys | `NCKEY_F01` through `NCKEY_F60` |
| Page Up / Down | `NCKEY_PGUP`, `NCKEY_PGDOWN` |
| Home / End | `NCKEY_HOME`, `NCKEY_END` |
| Terminal resize event | `NCKEY_RESIZE` |
| Mouse left click | `NCKEY_BUTTON1` |
| Mouse right click | `NCKEY_BUTTON3` |

---

## Summary Checklist

> [!success] Before You Ship — Verify These
> - [x] `NotCurses` is the first thing instantiated, with a zero-initialised options struct
> - [x] You never call `delete` on the standard plane pointer
> - [x] Every drawing call is followed eventually by `nc.render()` to make it visible
> - [x] You rely on C++ scope to destroy planes — no manual teardown calls
> - [x] `Cell::load()` and `Cell::release()` are called **on the Cell**, with the plane as the argument
> - [x] Every `Cell::load()` has a matching `Cell::release()` to prevent memory leaks
> - [x] Coordinates passed to `putstr()` and `move()` are relative to the plane's parent, not the terminal
> - [x] Colours are set explicitly before each new colour region — they do not reset automatically
> - [x] Your event loop handles `NCKEY_RESIZE` so that your layout updates when the terminal is resized

---

*Generated for Obsidian — internal links, admonition callouts, and fenced code blocks are fully compatible with core Obsidian rendering and the Admonition community plugin.*
