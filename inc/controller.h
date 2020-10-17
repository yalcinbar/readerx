#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

// TODO: base window_size updates on the Exposure event
#define INPUT_MASK ButtonPressMask | KeyPressMask | KeyReleaseMask | ExposureMask

// Navigation 1 (h, j, k, l)
#define KEY_LEFT      104
#define KEY_DOWN      106
#define KEY_UP        107
#define KEY_RIGHT     108

// Navigation 2 (left, up, right, down)
#define KEY_LEFT2     65361
#define KEY_UP2       65362
#define KEY_RIGHT2    65363  
#define KEY_DOWN2     65364

// Navigation 3
#define PAGE_UP       65365
#define PAGE_DOWN     65366
#define PAGE_UP2      98
#define PAGE_DOWN2    102

// Jump
#define HOME          65360
#define END           65367
#define HOME2         103
#define END2          71

// Redo
#define REDO          46

// Mouse actions
#define LEFT_CLICK    1
#define RIGHT_CLICK   3
#define SCROLL_UP     4
#define SCROLL_DOWN   5
#define SCROLL_LEFT   6
#define SCROLL_RIGHT  7

// Expose event
#define RESIZE        12

// Modifier keys
#define SHIFT_L       65505
#define SHIFT_R       65506
#define CTRL_L        65507
#define CTRL_R        65508
#define ALT           65513

// Continuity
#define CONTINUITY    99

// Zoom
#define ZOOM_FIT      102
#define ZOOM_IN       61
#define ZOOM_OUT      45

// Exit
#define EXIT          33

typedef struct {
  common_t *common;
  int input;
  int last_input;
  int ctrl_active;
  int input_active;
  int rep;
  event_t event;
} controller_t;

void set_window_size(controller_t *controller);
void get_input(controller_t *controller);
void generate_event(controller_t *controller);

#endif
