#ifndef VIEW_H
#define VIEW_H

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include "common.h"
#include <X11/Xutil.h>

typedef struct {
  queue_t *scene_queue;
  queue_t *surface_queue;
  queue_t *cairo_queue;
} history_t;

typedef struct {
  common_t *common;
  XTextProperty window_title;
  XWMHints *wmhints;
  history_t history;
  queue_t *scene_queue;
} view_t;

void update_title(view_t *view);
void display_scene(view_t *view);

#endif
