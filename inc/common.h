#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <X11/Xlib.h>

/* RGB gray values */
#define READERX_BACKGROUND_DARK   0x7F7F7F
#define READERX_BACKGROUND_LIGHT  0xBCBCBC 

/* Default window dimensions are 100x100 */
#define DEFAULT_WINDOW_DIM        100

/* Max queue length */
#define MAX_QUEUE_LENGTH          10

/* Dimension datatypes */
typedef struct {
  double x;
  double y;
} fdim_t;

typedef struct {
  int x;
  int y;
} dim_t;

/* Error messages */
enum error_t {
  FNOTFOUND_ERROR = -1,
  FNOTOPEN_ERROR = -2,
  FEMPTY_ERROR = -3
};

/* Event datatype */
typedef enum event_type {
  NextPage = 1,
  PreviousPage,
  ScrollDown,
  ScrollUp,
  ScrollLeft,
  ScrollRight,
  Jump,
  Resize,
  Standby,
  Continuity,
  ZoomFit,
  ZoomIn,
  ZoomOut,
  Exit
} event_type_t;

/* Event type */
typedef struct {
  event_type_t type;
  int rep;
} event_t;

/* Scene datatype */
typedef struct {
  void *page;
  int visible;
  int page_no;
  fdim_t page_size;
  dim_t offset;
  fdim_t scaling;
} scene_t;

typedef struct {
  Display *display;
  Screen *screen;
  Drawable drawable;
  dim_t window_size;
  char *input_file;
  char window_title[200];
} common_t;

/* Queue for passing scenes */
typedef struct {
  int head;
  int tail;
  void *q[MAX_QUEUE_LENGTH];
} queue_t;

/* controller functions */
void *init_controller(common_t *common);
void deinit_controller(void *data);
event_t controller_main(void *data);

/* model functions */
void *init_model(common_t *common);
void deinit_model(void *data);
queue_t *model_main(void *data, event_t);

/* view functions */
void *init_view(common_t *common);
void deinit_view(void *data);
void view_main(void *data, queue_t *scene_queue);

/* readerx datatype */
typedef struct {
  void *controller;
  void *model;
  void *view;
} readerx_t;

#endif

