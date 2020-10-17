#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "controller.h"
#include "util.h"

void *init_controller(common_t *common)
{
  controller_t *controller = malloc(sizeof(controller_t));
  controller->common = common;

  controller->input = 0;
  controller->last_input = 0;
  controller->ctrl_active = 0;
  controller->input_active = 0;
  controller->rep = 0;

  XSelectInput(common->display, common->drawable, INPUT_MASK);

  Atom wmDelete = XInternAtom(common->display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(common->display, common->drawable, &wmDelete, 1);

  return controller;
}

void deinit_controller(void *data)
{
  controller_t *controller = (controller_t *) data;

  /*
  XDestroyWindow(controller->common->display, controller->common->drawable);
  XCloseDisplay(controller->common->display);
  //XFree(controller->common->display);
  //XFree(controller->common->screen);
  */
  free(controller->common->input_file);
  free(controller->common);
  free(controller);
}

void set_window_size(controller_t *controller)
{
  common_t *common = controller->common;
  XWindowAttributes attr;
  XGetWindowAttributes(common->display, common->drawable, &attr);

  if (common->window_size.x != attr.width ||
      common->window_size.y != attr.height) {
    common->window_size.x = attr.width;
    common->window_size.y = attr.height;
    LOG("Window resized to %d x %d", (int) common->window_size.x, (int) common->window_size.y);
  }
}

void get_input(controller_t *controller)
{
  char keybuf[8];
  KeySym key;
  XEvent e;
  int input = 0;

  Display *dsp = controller->common->display;

  // Read the input from the user
  if (XPending(dsp)) {
    XNextEvent(dsp, &e);

    switch (e.type) {
      case ButtonPress:
        LOG("ButtonPress event received: %d", e.xbutton.button);
        input = (int) e.xbutton.button;
        break;
      case KeyPress:
        XLookupString(&e.xkey, keybuf, sizeof(keybuf), &key, NULL);
        LOG("KeyPress event received: %ld", key);
        input = (int) key;
        if (input == SHIFT_L || input == SHIFT_R)
          return;
        controller->last_input = controller->input;

        if (!controller->ctrl_active && (key == CTRL_L || key == CTRL_R))
          controller->ctrl_active = 1;

        if (controller->last_input < '0' || controller->last_input > '9')
          controller->rep = 0;

        if (input >= '0' && input <= '9')
          controller->rep = (controller->rep * 10) + input - '0';
        break;
      case KeyRelease:
        XLookupString(&e.xkey, keybuf, sizeof(keybuf), &key, NULL);
        LOG("KeyRelease event received: %ld", key);

        if (controller->ctrl_active && (key == CTRL_L || key == CTRL_R))
          controller->ctrl_active = 0;
        break;
      case Expose:
        LOG("Expose event received: %d", e.type);
        set_window_size(controller);
        input = RESIZE;
        break;
      case ClientMessage:
        LOG("Exit event received: %d", e.type);
        input = EXIT;
        break;
      default:
        return;
    }
    XSync(dsp, 1);
    if (input) {
      controller->input_active = 1;
      controller->input = input;
    }
  }
}

void generate_event(controller_t *controller)
{
  static event_t past_event;

  controller->event.type = Standby;
  controller->event.rep = 1;

  if (!controller->input_active)
    return;
  controller->input_active = 0;

  switch (controller->input)
  {
    case PAGE_DOWN:
      controller->event.type = NextPage;
      break;
    case PAGE_UP:
      controller->event.type = PreviousPage;
      break;
    case PAGE_UP2:
      if (controller->ctrl_active)
        controller->event.type = PreviousPage;
      else
        controller->event.type = Standby;
      break;
    case SCROLL_DOWN:
    case KEY_DOWN:
    case KEY_DOWN2:
      controller->event.type = ScrollDown;
      if (controller->rep > 0)
        controller->event.rep = controller->rep;
      break;
    case SCROLL_UP:
    case KEY_UP:
    case KEY_UP2:
      controller->event.type = ScrollUp;
      if (controller->rep > 0)
        controller->event.rep = controller->rep;
      break;
    case HOME2:
      if (controller->last_input == HOME2) {
        controller->input = 0;
        controller->event.type = Jump;
        controller->rep = 0;
      }
      break;
    case HOME:
      controller->event.type = Jump;
      controller->rep = 0;
      break;
    case END:
    case END2:
      controller->event.type = Jump;
      controller->event.rep = -1;
      if (controller->rep > 0)
        controller->event.rep = controller->rep;
      break;
    case KEY_LEFT:
    case KEY_LEFT2:
      controller->event.type = ScrollLeft;
      if (controller->rep > 0)
        controller->event.rep = controller->rep;
      break;
    case KEY_RIGHT:
    case KEY_RIGHT2:
      controller->event.type = ScrollRight;
      if (controller->rep > 0)
        controller->event.rep = controller->rep;
      break;
    case CONTINUITY:
      controller->event.type = Continuity;
      break;
    case ZOOM_FIT:
      if (controller->ctrl_active)
        controller->event.type = NextPage;
      else
        controller->event.type = ZoomFit;
      break;
    case ZOOM_IN:
      controller->event.type = ZoomIn;
      break;
    case ZOOM_OUT:
      controller->event.type = ZoomOut;
      break;
    case RESIZE:
      controller->event.type = Resize;
      break;
    case REDO:
      controller->event.type = past_event.type;
      controller->event.rep = past_event.rep;
      break;
    case EXIT:
      controller->event.type = Exit;
      break;
    default:
      controller->event.type = Standby;
  }

  past_event.type = controller->event.type;
  past_event.rep = controller->event.rep;
  LOG("Received event: %d", controller->event.type);
}

event_t controller_main(void *data)
{
  controller_t *controller = (controller_t *) data;

  // Get input and generate event
  get_input(controller);
  generate_event(controller);

  return controller->event;
}
