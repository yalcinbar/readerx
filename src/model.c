#include "model.h"
#include "util.h"

int get_scaling_index(int page_height, int screen_height)
{
  int index = 0;

  while ((index < ZOOM_LUT_LENGTH) 
      && ((zoom_lut[index] * page_height) < screen_height))
    index++;

  if (index > 0)
    index--;

  return index;
}

void *init_model(common_t *common)
{
  scene_t *scn;
  GError **gerror;
  PopplerPage *first_page;

  model_t *model = malloc(sizeof(model_t));
  model->common = common;

  model->doc = poppler_document_new_from_file(common->input_file, NULL, gerror);
  if (!model->doc) {
    LOG("Cannot open file %s", common->input_file);
    return NULL;
  }

  model->num_of_pages = poppler_document_get_n_pages(model->doc);

  // Always start with the first page
  model->page.number = 0;
  model->page.margin = 0;
  first_page = poppler_document_get_page(model->doc, 0);
  poppler_page_get_size(first_page, &model->page.dim.x, &model->page.dim.y);
  model->page.dim.x++;
  model->page.dim.y++;

  // Set default view options
  model->continuity = NONCONTINUOUS_VIEW;
  model->fit = FIT_PAGE;
  model->offset = 0;
  model->scaling_index = get_scaling_index(model->page.dim.y, HeightOfScreen(common->screen));
  model->scaling = zoom_lut[model->scaling_index];
  
  // Set window size 
  common->window_size.x = model->scaling * model->page.dim.x;
  common->window_size.y = model->scaling * model->page.dim.y;

  model->queue = malloc(sizeof(queue_t));
  model->queue->head = model->queue->tail = 0;

  g_object_unref(first_page);
  return model;
}

void deinit_model(void *data)
{
  model_t *model = (model_t *) data;

  g_object_unref(model->doc);
  free(model->queue);
  free(model);
}

// Helper functions
long get_document_length(model_t *model)
{
  fdim_t page_size;
  int page_number;

  long document_length = 0;

  for (page_number = 0; page_number < model->num_of_pages; page_number++) {
    page_size = get_page_size(model, page_number);
    document_length += page_size.y;
  }
  return document_length;
}

void set_page(model_t *model, int page_number)
{
  PopplerPage *page;
  model->page.number = page_number;
  page = poppler_document_get_page(model->doc, page_number);
  model->page.margin = 0;
  poppler_page_get_size(page, &model->page.dim.x, &model->page.dim.y);
  model->page.dim.x++;
  model->page.dim.y++;

  g_object_unref(page);
}

fdim_t get_page_size(model_t *model, int page_number)
{
  fdim_t page_dim;
  PopplerPage *page;
  page = poppler_document_get_page(model->doc, page_number);
  poppler_page_get_size(page, &page_dim.x, &page_dim.y);

  page_dim.x++;
  page_dim.y++;

  page_dim.x *= model->scaling;
  page_dim.y *= model->scaling;

  g_object_unref(page);

  return page_dim;
}

int get_visible_length(int window_length, int page_length, int margin)
{
  int visible_portion; 

  if (margin < 0)
    visible_portion = page_length + margin;
  else if ((page_length + margin) > window_length)
    visible_portion = window_length - margin;
  else
    visible_portion = page_length;

  return visible_portion;
}

void check_borders(model_t *model)
{
  fdim_t page_size;
  long document_length;
  dim_t window_size = model->common->window_size;
  page_size = get_page_size(model, model->page.number);

  if (model->continuity == NONCONTINUOUS_VIEW) {
    if (window_size.y + 1 >= page_size.y)
      model->page.margin = (window_size.y - page_size.y) / 2;
    else {
      if (model->page.margin > 0)
        model->page.margin = 0;
    }

    if (model->page.margin < 0) {
      if (page_size.y + model->page.margin - window_size.y < 0)
        model->page.margin = window_size.y - page_size.y - model->page.number;
    }
    LOG("Border set to: %d", model->page.margin);
  }

  if (model->continuity == CONTINUOUS_VIEW) {
    if (model->page.margin > 0)
      model->page.margin = 0;

    document_length = get_document_length(model);
    if (document_length + model->page.margin - window_size.y < 0)
      model->page.margin = window_size.y - document_length - model->page.number;

    if (window_size.y + 1 >= document_length)
      model->page.margin = (window_size.y - document_length) / 2;
  }

  if (window_size.x >= (int) page_size.x)
    model->offset = (window_size.x - page_size.x) / 2;

}

// Scene functions
scene_t *create_scene(model_t *model, int page, int offset_x, int offset_y)
{
  scene_t *scn = malloc(sizeof(scene_t));

  scn->page = poppler_document_get_page(model->doc, page);
  scn->visible = 1;
  scn->page_no = page;
  poppler_page_get_size(scn->page, &(scn->page_size.x), &(scn->page_size.y));
  scn->scaling.x = scn->scaling.y = model->scaling;
  scn->offset.x = offset_x;
  scn->offset.y = offset_y;

  return scn;
}

void update_model(model_t *model)
{
  scene_t *scn;
  int page_number, margin, capacity, page_length, visible_portion;
  fdim_t page_size;
  dim_t window_size = model->common->window_size;

  check_borders(model);

  if (model->continuity == NONCONTINUOUS_VIEW)
    enqueue(model->queue, create_scene(model, model->page.number, model->offset, model->page.margin));

  if (model->continuity == CONTINUOUS_VIEW) {
    
    // Seek to the first visible page, calculate margin
    margin = model->page.margin;
    for (page_number = 0; page_number < model->num_of_pages; page_number++) {
      page_size = get_page_size(model, page_number);
      if ((margin + page_size.y) > 0)
        break;
      margin += page_size.y;
    }
    model->page.number = page_number;

    // Create scenes to fill the view
    for (capacity = window_size.y; (capacity >= 0) && (page_number < model->num_of_pages); page_number++) {
      enqueue(model->queue, create_scene(model, page_number, model->offset, margin));
      page_size = get_page_size(model, page_number);
      capacity -= get_visible_length(window_size.y, page_size.y, margin);
      LOG("Page number: %d, Margin: %d, Capacity: %d", page_number, margin, capacity);
      margin += page_size.y;
    }
  }
  XClearWindow(model->common->display, model->common->drawable);
}

void update_window_title(model_t *model)
{
  if (model->continuity == CONTINUOUS_VIEW)
    sprintf(model->common->window_title, "readerX - [C, %s, %d/%d] - ", 
        zoom_string_lut[model->scaling_index], model->page.number + 1, model->num_of_pages);
  else
    sprintf(model->common->window_title, "readerX - [NC, %s, %d/%d] - ", 
        zoom_string_lut[model->scaling_index], model->page.number + 1, model->num_of_pages);
}

// Event handlers
void resize_event_handler(model_t *model)
{
  dim_t window_size = model->common->window_size;

  if (model->fit == FIT_PAGE)
    model->scaling = window_size.y / model->page.dim.y;

  if (model->fit == FIT_WIDTH)
    model->scaling = window_size.x / model->page.dim.x;
}

void previous_page_event_handler(model_t *model)
{
  int page_height, window_height;

  window_height = model->common->window_size.y;
  page_height = model->scaling * model->page.dim.y;

  if (model->continuity == CONTINUOUS_VIEW) { 
    model->page.margin += window_height;

    if (model->page.margin > 0)
      model->page.margin = 0;
  }

  if (model->continuity == NONCONTINUOUS_VIEW) {
    if (model->page.margin && page_height > window_height) {
      if (model->page.margin + window_height >= 0)
        model->page.margin = 0;
      else
        model->page.margin += window_height;
    }
    else {
      if (model->page.number > 0)
        set_page(model, --model->page.number);
    }
  }
}

void jump_event_handler(model_t *model, int rep)
{
  fdim_t page_dim;
  int page;

  // Wrap around
  if (rep < 0)
    rep += model->num_of_pages;

  if (rep <= 0 || rep > model->num_of_pages)
    return;

  rep--;

  if (model->continuity == NONCONTINUOUS_VIEW)
    model->page.number = rep;
  else {
    model->page.margin = 0;
    for (page = 0; page < rep; page++) {
      page_dim = get_page_size(model, page);
      model->page.margin -= page_dim.y;
    }
  }
}

void scroll_up_event_handler(model_t *model, int rep)
{
  while (rep > 0) {
    model->page.margin += VERTICAL_SCROLL_SPEED;

    if (model->page.margin > 0)
      model->page.margin = 0;
    rep--;
  }
}

void scroll_down_event_handler(model_t *model, int rep)
{
  int limit;

  while (rep > 0) {
    if (model->continuity == CONTINUOUS_VIEW)
      limit = model->common->window_size.y - get_document_length(model);
    else
      limit = model->common->window_size.y - model->page.dim.y * model->scaling;

    if (limit < 0) {
      model->page.margin -= VERTICAL_SCROLL_SPEED;

      if (model->page.margin < limit)
        model->page.margin = limit; 
    }
    rep--;
  }
}

void scroll_left_event_handler(model_t *model, int rep)
{
  while (rep > 0) {
    if ((model->page.dim.x * model->scaling) >= model->common->window_size.x) {
      model->offset += HORIZONTAL_SCROLL_SPEED;

      if (model->offset > 0)
        model->offset = 0;
    }
    rep--;
  }
}

void scroll_right_event_handler(model_t *model, int rep)
{
  while (rep > 0) {
    if ((model->page.dim.x * model->scaling) >= model->common->window_size.x) {
      model->offset -= HORIZONTAL_SCROLL_SPEED;

      if ((model->common->window_size.x - model->page.dim.x * model->scaling) > model->offset)
        model->offset = model->common->window_size.x - model->page.dim.x * model->scaling;
    }
    rep--;
  }
}

void next_page_event_handler(model_t *model)
{
  int step, page_height, window_height, visibility;

  window_height = model->common->window_size.y;
  page_height = model->scaling * model->page.dim.y;

  step = window_height;
  visibility = page_height + model->page.margin - step;

  if (model->continuity == CONTINUOUS_VIEW) { 
    model->page.margin -= step;

    if ((get_document_length(model) + model->page.margin - window_height) < 0)
      model->page.margin = window_height - get_document_length(model);
  }

  if (model->continuity == NONCONTINUOUS_VIEW ) {
    if (visibility < step  && visibility != 0 && visibility > model->page.margin) 
      model->page.margin -= visibility;
    else if(visibility == 0 || visibility < model->page.margin) {
      if (model->page.number < (model->num_of_pages - 1))
        set_page(model, ++model->page.number);
    }
    else {
      model->page.margin -= step;
    }
  }
}

void continuity_event_handler(model_t *model)
{
  int margin, page_number;
  long document_length;
  fdim_t page_size;
  dim_t window_size = model->common->window_size;
    
  margin = model->page.margin;

  if (model->continuity == CONTINUOUS_VIEW) {
    for (page_number = 0; page_number < model->num_of_pages; page_number++) {
      page_size = get_page_size(model, page_number);
      if ((margin + page_size.y) > 0)
        break;
      margin += page_size.y;
    }

    set_page(model, page_number);
    model->page.margin = margin; 
    if (page_size.y > window_size.y) {
      if (margin > 0)
        model->page.margin = 0;
      else
        model->page.margin = window_size.y - page_size.y;
    }
    model->continuity = NONCONTINUOUS_VIEW;
  }
  else {
    for (page_number = 0; page_number < model->page.number; page_number++) {
      page_size = get_page_size(model, page_number);
      margin -= page_size.y;
    }
    set_page(model, 0);
    model->page.number = page_number;
    // Page length starts from 0
    model->page.margin = margin;
    if (window_size.y < page_size.y)
      model->page.margin -= page_number;

    model->continuity = CONTINUOUS_VIEW;
  }
}

void zoom_fit_event_handler(model_t *model)
{
  switch (model->fit) {
    case FIT_PAGE:
      model->fit = FIT_WIDTH;
      break;
    case FIT_WIDTH:
    case FIT_FREE:
      model->fit = FIT_PAGE;
      model->page.margin = 0;
      break;
  }
}

void zoom_in_event_handler(model_t *model)
{
  int prev_scaling_index = model->scaling_index;
  model->scaling_index++;

  if (model->scaling_index > ZOOM_LUT_LENGTH - 1)
    model->scaling_index = ZOOM_LUT_LENGTH - 1;

  LOG("Zoom in event, index: %d", model->scaling_index);
  model->scaling = zoom_lut[model->scaling_index];
  model->offset = (model->common->window_size.x - model->scaling * model->page.dim.x) / 2;
  model->page.margin = zoom_lut[model->scaling_index] * model->page.margin / zoom_lut[prev_scaling_index];

  model->fit = FIT_FREE;
}

void zoom_out_event_handler(model_t *model)
{
  int prev_scaling_index = model->scaling_index;
  model->scaling_index--;

  if (model->scaling_index < 0)
    model->scaling_index = 0;

  LOG("Zoom out event, index: %d", model->scaling_index);
  model->scaling = zoom_lut[model->scaling_index];
  model->offset = (model->common->window_size.x - model->scaling * model->page.dim.x) / 2;
  model->page.margin = zoom_lut[model->scaling_index] * model->page.margin / zoom_lut[prev_scaling_index];

  model->fit = FIT_FREE;
}

queue_t *model_main(void *data, event_t event)
{
  model_t *model = (model_t *) data;

  switch (event.type) {
    case Standby:
      break;
    case Resize:
      resize_event_handler(model);
      break;
    case PreviousPage:
      previous_page_event_handler(model);
      break;
    case Jump:
      jump_event_handler(model, event.rep);
      break;
    case ScrollUp:
      scroll_up_event_handler(model, event.rep);
      break;
    case ScrollDown:
      scroll_down_event_handler(model, event.rep);
      break;
    case ScrollLeft:
      scroll_left_event_handler(model, event.rep);
      break;
    case ScrollRight:
      scroll_right_event_handler(model, event.rep);
      break;
    case NextPage:
      next_page_event_handler(model);  
      break;
    case Continuity:
      continuity_event_handler(model);
      break;
    case ZoomFit:
      zoom_fit_event_handler(model);
      resize_event_handler(model);
      break;
    case ZoomIn:
      zoom_in_event_handler(model);
      break;
    case ZoomOut:
      zoom_out_event_handler(model);
      break;
  }

  if (event.type != Standby) {
    update_model(model);
    update_window_title(model);
  }

  return model->queue;
}
