#include "common.h"
#include "util.h"

common_t *init_common(char *filepath)
{ 
  common_t *common = malloc(sizeof(common_t));

  common->input_file = filepath;
  common->display = XOpenDisplay(NULL);
  common->screen = DefaultScreenOfDisplay(common->display);
  common->window_size.x = common->window_size.y = DEFAULT_WINDOW_DIM;
  common->drawable = XCreateSimpleWindow(common->display,
      DefaultRootWindow(common->display),
      0, 0, common->window_size.x, common->window_size.y,
      0, 0, READERX_BACKGROUND_LIGHT);

  if (!common || !common->input_file || !common->display || !common->screen)
    return NULL;

  return common;
}

readerx_t *init_readerx(char *filepath)
{
  readerx_t *readerx = malloc(sizeof(readerx_t));

  common_t *common = init_common(filepath);
  if (!common) {
    LOG("Failed to initialize common");
    return NULL;
  }

  readerx->controller  = init_controller(common);
  if (!readerx->controller) {
    LOG("Failed to initialize controller");
    return NULL;
  }

  readerx->model = init_model(common);
  if (!readerx->model) {
    LOG("Failed to initialize model");
    return NULL;
  }

  readerx->view = init_view(common);
  if (!readerx->view) {
    LOG("Failed to initialize view");
    return NULL;
  }

  return readerx;
}

int deinit_readerx(readerx_t *readerx)
{
  deinit_view(readerx->view);
  deinit_model(readerx->model);
  deinit_controller(readerx->controller);

  free(readerx);

  return 0;
}

int main(int argc, char *argv[])
{
  readerx_t *readerx;
  event_t event;
  queue_t *scene_queue;

  char *filepath = NULL;

  if (filepath = parse_input(argc, argv))
    if (readerx = init_readerx(filepath)) {
      while (1) {
        event = controller_main(readerx->controller);
        if (event.type == Exit)
          break;
        if (event.type != Standby) {
          scene_queue = model_main(readerx->model, event);
          view_main(readerx->view, scene_queue);
        }
      }
      deinit_readerx(readerx);
    }

  return 0;
}
