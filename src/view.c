#include "view.h"
#include "util.h"

#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "poppler.h"
#include "cairo.h"

static unsigned char icon_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x03, 0x03, 0x00, 0x06, 0x01, 0x00, 0x8c, 0x01, 0x64, 0xcc, 0x00,
  0x7c, 0x78, 0x00, 0x0c, 0x70, 0x00, 0x0c, 0x30, 0x00, 0x04, 0x78, 0x00,
  0x04, 0xd8, 0x00, 0x04, 0x8c, 0x00, 0x04, 0x86, 0x01, 0x04, 0x02, 0x03,
  0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void *init_view(common_t *common)
{
  view_t *view = malloc(sizeof(view_t));

  // Resize the window
  XResizeWindow(common->display, common->drawable, common->window_size.x, common->window_size.y);

  // Set the window icon
  view->wmhints = XAllocWMHints();
  view->wmhints->flags |= IconPixmapHint;
  view->wmhints->icon_pixmap = XCreatePixmapFromBitmapData(common->display,
      common->drawable, icon_bits, 20, 20, 0, 0xffffff, 
      DefaultDepth(common->display, 0));
  XSetWMHints(common->display, common->drawable, view->wmhints);

  XMapWindow(common->display, common->drawable);
  view->common = common;

  view->history.scene_queue = malloc(sizeof(queue_t));
  view->history.scene_queue->head = view->history.scene_queue->tail = 0;

  view->history.surface_queue = malloc(sizeof(queue_t));
  view->history.surface_queue->head = view->history.surface_queue->tail = 0;

  view->history.cairo_queue = malloc(sizeof(queue_t));
  view->history.cairo_queue->head = view->history.cairo_queue->tail = 0;

  return view;
}

void deinit_view(void *data)
{
  view_t *view = (view_t *) data;

  XFreePixmap(view->common->display, view->wmhints->icon_pixmap);
  XFree(view->wmhints);

  free(view->history.scene_queue);
  free(view->history.surface_queue);
  free(view->history.cairo_queue);

  free(view);
}

void update_title(view_t *view)
{
  // Set the window name, strip the "file:///" part and 
  // concatenate the program name with the filepath
  strcat(view->common->window_title, view->common->input_file + 7);
  view->window_title.value = view->common->window_title;
  view->window_title.encoding = XA_STRING;
  view->window_title.format = 8;
  view->window_title.nitems = strlen(view->window_title.value);

  XSetWMName(view->common->display, view->common->drawable, &(view->window_title));
}

void display_scene(view_t *view)
{
  scene_t *scene;
  cairo_surface_t *surface;
  cairo_t *cairo;

  // Process scene queue
  while (scene = (scene_t *) dequeue(view->scene_queue)) {
    surface = cairo_xlib_surface_create(view->common->display, 
      view->common->drawable, DefaultVisualOfScreen(view->common->screen), 0, 0);

    cairo_xlib_surface_set_drawable(surface, view->common->drawable, 
        scene->page_size.x * scene->scaling.x + scene->offset.x,
        scene->page_size.y * scene->scaling.y + scene->offset.y);

    cairo_surface_set_device_scale(surface, scene->scaling.x, scene->scaling.y);
    cairo_surface_set_device_offset(surface, scene->offset.x, scene->offset.y);

    cairo = cairo_create(surface);

    // PDF background color
    cairo_set_source_rgb(cairo, 1,1,1);
    cairo_rectangle(cairo, 0, 0, view->common->window_size.x, view->common->window_size.y);
    cairo_fill(cairo);

    poppler_page_render(scene->page, cairo);

    g_object_unref(scene->page);
    free(scene);

    /*
    cairo_destroy(cairo);
    free(cairo);
    cairo_surface_destroy(surface);
    */
  }
}

void view_main(void *data, queue_t *scene_queue)
{
  view_t *view = (view_t *) data;
  view->scene_queue = scene_queue;

  display_scene(view);
  update_title(view);
}
