#include "main_view.hpp"
#include <gui/elements.h>

struct MainViewModel {
  uint32_t elapsed_time;
  uint32_t total_time;
  const char *step_name;
  const char *status_text;
  const char *movement_text;
  bool process_active;
  bool paused;
  bool waiting_for_user;
  const char *user_message;
  bool motor_running;
  const char *direction;
};

static void main_view_draw_callback(Canvas *canvas, void *_model) {
  MainViewModel *model = static_cast<MainViewModel *>(_model);

  canvas_clear(canvas);
  canvas_set_font(canvas, FontPrimary);

  // Draw title
  canvas_draw_str(canvas, 2, 12, "C41 Process");

  // Draw current step info
  canvas_set_font(canvas, FontSecondary);
  if (model->step_name) {
    canvas_draw_str(canvas, 2, 24, model->step_name);
  }

  // Draw status or user message
  if (model->waiting_for_user && model->user_message) {
    canvas_draw_str(canvas, 2, 36, model->user_message);
  } else if (model->status_text) {
    canvas_draw_str(canvas, 2, 36, model->status_text);
  }

  // Draw movement state if not waiting for user
  if (!model->waiting_for_user && model->movement_text) {
    canvas_draw_str(canvas, 2, 48, model->movement_text);
  }

  // Draw motor status
  canvas_draw_str(canvas, 2, 60, "Motor:");
  canvas_draw_str(canvas, 50, 60,
                  model->motor_running ? model->direction : "OFF");

  // Draw control hints
  if (model->process_active) {
    if (model->waiting_for_user) {
      elements_button_center(canvas, "Continue");
    } else if (model->paused) {
      elements_button_center(canvas, "Resume");
    } else {
      elements_button_center(canvas, "Pause");
    }
    elements_button_right(canvas, "Skip");
    elements_button_left(canvas, "Restart");
  } else {
    elements_button_center(canvas, "Start");
  }
}

static bool main_view_input_callback(InputEvent *event, void *context) {
  MainView *view = static_cast<MainView *>(context);
  bool consumed = false;

  if (event->type == InputTypeShort) {
    switch (event->key) {
    case InputKeyOk:
      if (view->ok_callback) {
        view->ok_callback(view->ok_context);
        consumed = true;
      }
      break;
    case InputKeyLeft:
      if (view->left_callback) {
        view->left_callback(view->left_context);
        consumed = true;
      }
      break;
    case InputKeyRight:
      if (view->right_callback) {
        view->right_callback(view->right_context);
        consumed = true;
      }
      break;
    default:
      break;
    }
  }

  return consumed;
}

MainView::MainView() {
  view = view_alloc();
  view_allocate_model(view, ViewModelTypeLocking, sizeof(MainViewModel));
  view_set_context(view, this);
  view_set_draw_callback(view, main_view_draw_callback);
  view_set_input_callback(view, main_view_input_callback);

  // Initialize callbacks
  ok_callback = nullptr;
  left_callback = nullptr;
  right_callback = nullptr;
  ok_context = nullptr;
  left_context = nullptr;
  right_context = nullptr;
}

MainView::~MainView() { view_free(view); }

View *MainView::get_view() { return view; }

void MainView::set_ok_callback(MainViewCallback callback, void *context) {
  ok_callback = callback;
  ok_context = context;
}

void MainView::set_left_callback(MainViewCallback callback, void *context) {
  left_callback = callback;
  left_context = context;
}

void MainView::set_right_callback(MainViewCallback callback, void *context) {
  right_callback = callback;
  right_context = context;
}

void MainView::update_state(const AgitationProcessInterpreter &interpreter,
                            bool process_active, bool paused) {
  with_view_model_cpp(
      view, MainViewModel *, model,
      {
        model->elapsed_time = interpreter.getCurrentMovementTimeElapsed();
        model->total_time = interpreter.getCurrentMovementDuration();

        const AgitationStepStatic *current_step = interpreter.getCurrentStep();
        model->step_name = current_step ? current_step->name : "Ready";

        char status_text[64];
        snprintf(status_text, sizeof(status_text), "%s Time: %lu/%lu",
                 paused ? "[PAUSED]" : "", model->elapsed_time,
                 model->total_time);
        model->status_text = status_text;

        char movement_text[32];
        snprintf(movement_text, sizeof(movement_text), "Movement: %s",
                 interpreter.getCurrentMovement() ? "Active" : "Idle");
        model->movement_text = movement_text;

        model->process_active = process_active;
        model->paused = paused;
        model->waiting_for_user = interpreter.isWaitingForUser();
        model->user_message = interpreter.getUserMessage();
      },
      true);
}