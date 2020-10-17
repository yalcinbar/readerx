#ifndef MODEL_H
#define MODEL_H

#include <poppler.h>

#include "common.h"

// Navigation settings
#define VERTICAL_SCROLL_SPEED   48
#define HORIZONTAL_SCROLL_SPEED 24

// Zoom settings
#define BASE_SCALING            2.25
#define ZOOM_LUT_LENGTH         23

const double zoom_lut[] = {
  0.050 * BASE_SCALING,
  0.060 * BASE_SCALING,
  0.070 * BASE_SCALING,
  0.080 * BASE_SCALING,
  0.090 * BASE_SCALING,
  0.100 * BASE_SCALING,
  0.150 * BASE_SCALING,
  0.175 * BASE_SCALING,
  0.200 * BASE_SCALING,
  0.250 * BASE_SCALING,
  0.300 * BASE_SCALING,
  0.350 * BASE_SCALING,
  0.425 * BASE_SCALING,
  0.500 * BASE_SCALING,
  0.700 * BASE_SCALING,
  0.850 * BASE_SCALING,
  1.000 * BASE_SCALING,
  1.250 * BASE_SCALING,
  1.500 * BASE_SCALING, 
  1.750 * BASE_SCALING,
  2.500 * BASE_SCALING,
  3.000 * BASE_SCALING,
  4.000 * BASE_SCALING
};

const char *zoom_string_lut[] = {
  "5%",
  "6%",
  "7%",
  "8%",
  "9%",
  "10%",
  "15%",
  "17.5%",
  "20%",
  "25%",
  "30%",
  "35%",
  "42.5%",
  "50%",
  "70%",
  "85%",
  "100%",
  "125%",
  "150%",
  "175%",
  "250%",
  "300%",
  "400%"
};

typedef enum fit_mode {
  FIT_PAGE,
  FIT_WIDTH,
  FIT_FREE
} fit_mode_t;

// Internal settings
#define NONCONTINUOUS_VIEW      0
#define CONTINUOUS_VIEW         1

typedef struct {
  int number;
  int margin;
  fdim_t dim;
} page_t;

typedef struct {
  common_t *common;
  PopplerDocument *doc;
  page_t page;
  int continuity;
  fit_mode_t fit;
  double scaling;
  int scaling_index;
  int offset;
  int num_of_pages;
  queue_t *queue;
} model_t;

int get_scaling_index(int page_height, int window_height);
long get_document_length(model_t *model);
void set_page(model_t *model, int page_number);
fdim_t get_page_size(model_t *model, int page_number);
int get_visible_length(int window_length, int page_length, int margin);

scene_t *create_scene(model_t *model, int page, int offset_x, int offset_y);
void update_model(model_t *model);
void update_window_title(model_t *model);

void resize_event_handler(model_t *model);
void previous_page_event_handler(model_t *model);
void jump_event_handler(model_t *model, int rep);
void scroll_up_event_handler(model_t *model, int rep);
void scroll_down_event_handler(model_t *model, int rep);
void scroll_left_event_handler(model_t *model, int rep);
void scroll_right_event_handler(model_t *model, int rep);
void next_page_event_handler(model_t *model);
void continuity_event_handler(model_t *model);
void zoom_in_event_handler(model_t *model);
void zoom_out_event_handler(model_t *model);
#endif
