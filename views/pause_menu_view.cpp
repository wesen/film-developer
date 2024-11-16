#include "pause_menu_view.hpp"
#include <gui/elements.h>
#include <gui/view.h>

struct PauseMenuModel {
  uint32_t elapsed_time;
  uint32_t total_time;
};

static void pause_menu_draw_callback(Canvas *canvas, void *_model) {
  PauseMenuModel *model = static_cast<PauseMenuModel *>(_model);

  // Draw title
  canvas_set_font(canvas, FontPrimary);
  canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignCenter, "PAUSED");

  // Draw time
  canvas_set_font(canvas, FontSecondary);
  char time_str[32];
  snprintf(time_str, sizeof(time_str), "Time: %lu/%lu", model->elapsed_time,
           model->total_time);
  canvas_draw_str_aligned(canvas, 64, 25, AlignCenter, AlignCenter, time_str);

  // Draw controls
  elements_button_left(canvas, "Rewind");
  elements_button_right(canvas, "Skip");
  elements_button_center(canvas, "Back");
}

static bool pause_menu_input_callback(InputEvent *event, void *context) {
  PauseMenuView *view = static_cast<PauseMenuView *>(context);
  bool consumed = false;

  if (event->type == InputTypeShort) {
    switch (event->key) {
    case InputKeyLeft:
      if (view->rewind_callback) {
        view->rewind_callback(view->rewind_context);
        consumed = true;
      }
      break;
    case InputKeyRight:
      if (view->skip_callback) {
        view->skip_callback(view->skip_context);
        consumed = true;
      }
      break;
    case InputKeyOk:
    case InputKeyBack:
      if (view->back_callback) {
        view->back_callback(view->back_context);
        consumed = true;
      }
      break;
    default:
      break;
    }
  }

  return consumed;
}

PauseMenuView::PauseMenuView() {
  view = view_alloc();
  view_allocate_model(view, ViewModelTypeLocking, sizeof(PauseMenuModel));
  view_set_context(view, this);
  view_set_draw_callback(view, pause_menu_draw_callback);
  view_set_input_callback(view, pause_menu_input_callback);
}

PauseMenuView::~PauseMenuView() { view_free(view); }

View *PauseMenuView::get_view() { return view; }

void PauseMenuView::set_rewind_callback(PauseMenuCallback callback,
                                        void *context) {
  rewind_callback = callback;
  rewind_context = context;
}

void PauseMenuView::set_skip_callback(PauseMenuCallback callback,
                                      void *context) {
  skip_callback = callback;
  skip_context = context;
}

void PauseMenuView::set_back_callback(PauseMenuCallback callback,
                                      void *context) {
  back_callback = callback;
  back_context = context;
}

void PauseMenuView::update_time(uint32_t elapsed_time, uint32_t total_time) {
  with_view_model_cpp(
      view, PauseMenuModel *, model,
      {
        model->elapsed_time = elapsed_time;
        model->total_time = total_time;
      },
      true);
}