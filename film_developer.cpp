#include "agitation_process_interpreter.hpp"
#include "agitation_processes.hpp"
#include "agitation_sequence.hpp"
#include "motor_controller.hpp"
#include "views/main_view.hpp"
#include <furi.h>
#include <furi_hal_gpio.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/view_port.h>

#ifdef HOST
#include "test-film_developer/mock_controller.hpp"
#else
#include "embedded/motor_controller_embedded.hpp"
#endif

#include "views/pause_menu_view.hpp"

typedef enum {
  FilmDeveloperViewMain,
  FilmDeveloperViewPauseMenu,
} FilmDeveloperView;

typedef struct {
  Gui *gui;
  ViewDispatcher *view_dispatcher;
  ViewPort *view_port;
  FuriEventLoop *event_loop;
  FuriEventLoopTimer *state_timer;

  // Add motor controller
  MotorController *motor_controller;

  // Process state
  AgitationProcessInterpreter process_interpreter;
  const AgitationProcessStatic *current_process;
  bool process_active;

  // Display info
  char status_text[64];
  char step_text[32];
  char movement_text[32];

  // Additional state tracking
  bool paused;

  PauseMenuView *pause_menu;
} FilmDeveloperApp;

// Add motor control callback wrappers
static void draw_callback(Canvas *canvas, void *context) {
  FilmDeveloperApp *app = (FilmDeveloperApp *)context;

  canvas_clear(canvas);
  canvas_set_font(canvas, FontPrimary);

  // Draw title
  canvas_draw_str(canvas, 2, 12, "C41 Process");

  // Draw current step info
  canvas_set_font(canvas, FontSecondary);
  canvas_draw_str(canvas, 2, 24, app->step_text);

  // Draw status or user message
  if (app->process_interpreter.isWaitingForUser()) {
    canvas_draw_str(canvas, 2, 36, app->process_interpreter.getUserMessage());
  } else {
    canvas_draw_str(canvas, 2, 36, app->status_text);
  }

  // Draw movement state if not waiting for user
  if (!app->process_interpreter.isWaitingForUser()) {
    canvas_draw_str(canvas, 2, 48, app->movement_text);
  }

  // Draw pin states
  canvas_draw_str(canvas, 2, 60, "CW:");
  canvas_draw_str(canvas, 50, 60,
                  app->motor_controller->isRunning() ? "ON" : "OFF");

  canvas_draw_str(canvas, 2, 70, "CCW:");
  canvas_draw_str(canvas, 50, 70,
                  app->motor_controller->isRunning() ? "ON" : "OFF");

  // Draw control hints
  if (app->process_active) {
    if (app->process_interpreter.isWaitingForUser()) {
      elements_button_center(canvas, "Continue");
    } else if (app->paused) {
      elements_button_center(canvas, "Resume");
    } else {
      elements_button_center(canvas, "Pause");
    }
    elements_button_right(canvas, "SkIP");
    elements_button_left(canvas, "Restart");
  } else {
    elements_button_center(canvas, "Start");
  }
}

static void timer_callback(void *context) {
  FilmDeveloperApp *app = (FilmDeveloperApp *)context;

  if (!app->paused) {
    bool still_active = app->process_interpreter.tick();

    // Update status texts
    const AgitationStepStatic *current_step =
        &app->current_process
             ->steps[app->process_interpreter.getCurrentStepIndex()];

    snprintf(app->step_text, sizeof(app->step_text), "Step: %s",
             current_step->name);

    // Show remaining time for current movement
    snprintf(app->status_text, sizeof(app->status_text), "%s Time: %lus/%lus",
             app->paused ? "[PAUSED]" : "",
             app->process_interpreter.getCurrentMovementTimeElapsed(),
             app->process_interpreter.getCurrentMovementDuration());

    // Update movement text based on motor controller state
    snprintf(app->movement_text, sizeof(app->movement_text), "Movement: %s",
             app->motor_controller->getDirectionString());

    app->process_active = still_active;
    if (!still_active) {
      app->motor_controller->stop();
    }
  }

  view_port_update(app->view_port);
}

static void input_callback(InputEvent *input_event, void *context) {
  FilmDeveloperApp *app = (FilmDeveloperApp *)context;

  if (input_event->type == InputTypeShort) {
    if (input_event->key == InputKeyOk) {
      if (!app->process_active) {
        // Start new process
        app->process_interpreter.init(&C41_FULL_PROCESS_STATIC,
                                      app->motor_controller);
        app->process_active = true;
        app->paused = false;
      } else if (app->process_interpreter.isWaitingForUser()) {
        // Handle user confirmation
        app->process_interpreter.confirm();
      } else {
        // Toggle pause and show pause menu
        app->paused = true;
        app->motor_controller->stop();
        app->pause_menu->update_time(
            app->process_interpreter.getCurrentMovementTimeElapsed(),
            app->process_interpreter.getCurrentMovementDuration());
        view_dispatcher_switch_to_view(app->view_dispatcher,
                                       FilmDeveloperViewPauseMenu);
      }
    } else if (app->process_active && input_event->key == InputKeyRight) {
      // Skip to next step (only if not waiting for user)
      if (!app->process_interpreter.isWaitingForUser()) {
        app->motor_controller->stop();
        app->process_interpreter.skipToNextStep();
        if (app->process_interpreter.getCurrentStepIndex() >=
            app->current_process->steps_length) {
          app->process_active = false;
        }
      }
    } else if (app->process_active && input_event->key == InputKeyLeft) {
      // Restart current step
      app->motor_controller->stop();
      app->process_interpreter.reset();
    }
  } else if (input_event->type == InputTypeShort &&
             input_event->key == InputKeyBack) {
    if (app->process_active) {
      // Stop process
      app->process_active = false;
      app->paused = false;
      app->motor_controller->stop();
    } else {
      furi_event_loop_stop(app->event_loop);
    }
  }
}

// Add pause menu callbacks
static void pause_menu_rewind_callback(void *context) {
  FilmDeveloperApp *app = static_cast<FilmDeveloperApp *>(context);
  app->process_interpreter.reset();
  app->paused = false;
  view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewMain);
}

static void pause_menu_skip_callback(void *context) {
  FilmDeveloperApp *app = static_cast<FilmDeveloperApp *>(context);
  app->process_interpreter.skipToNextStep();
  app->paused = false;
  view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewMain);
}

static void pause_menu_back_callback(void *context) {
  FilmDeveloperApp *app = static_cast<FilmDeveloperApp *>(context);
  app->paused = false;
  view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewMain);
}

// Add view switching callbacks
static bool film_developer_navigation_event_callback(void *context) {
  FilmDeveloperApp *app = (FilmDeveloperApp *)context;
  if (app->paused) {
    app->paused = false;
    view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewMain);
    return true;
  }
  return false;
}

#ifdef __cplusplus
extern "C" {
#endif

int32_t film_developer_app(void *p) {
  UNUSED(p);
  FilmDeveloperApp *app = (FilmDeveloperApp *)malloc(sizeof(FilmDeveloperApp));

  // Initialize motor controller
  MotorControllerEmbedded motorController;
  motorController.initGpio();

#ifdef HOST
  app->motor_controller = new MockController();
#else
  app->motor_controller = &motorController;
#endif

  // Register app instance for callbacks
  furi_record_create("film_developer", app);

  // Create event loop
  app->event_loop = furi_event_loop_alloc();

  // Initialize GUI
  app->gui = (Gui *)furi_record_open(RECORD_GUI);

  // Initialize ViewDispatcher
  app->view_dispatcher = view_dispatcher_alloc();
  view_dispatcher_enable_queue(app->view_dispatcher);
  view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
  view_dispatcher_set_navigation_event_callback(
      app->view_dispatcher, film_developer_navigation_event_callback);

  // Initialize ViewPort for main view
  app->view_port = view_port_alloc();
  view_port_draw_callback_set(app->view_port, draw_callback, app);
  view_port_input_callback_set(app->view_port, input_callback, app);

  // Initialize pause menu
  app->pause_menu = new PauseMenuView();
  app->pause_menu->set_rewind_callback(pause_menu_rewind_callback, app);
  app->pause_menu->set_skip_callback(pause_menu_skip_callback, app);
  app->pause_menu->set_back_callback(pause_menu_back_callback, app);

  // Add views to dispatcher
  MainView *main_view = new MainView();
  view_dispatcher_add_view(app->view_dispatcher, FilmDeveloperViewMain,
                           main_view->get_view());
  view_dispatcher_add_view(app->view_dispatcher, FilmDeveloperViewPauseMenu,
                           app->pause_menu->get_view());

  // Attach ViewDispatcher to GUI
  view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui,
                                ViewDispatcherTypeFullscreen);

  // Set initial state
  app->current_process = &C41_FULL_PROCESS_STATIC;
  app->process_active = false;
  app->paused = false;
  snprintf(app->status_text, sizeof(app->status_text), "Press OK to start");
  snprintf(app->step_text, sizeof(app->step_text), "Ready");
  snprintf(app->movement_text, sizeof(app->movement_text), "Movement: Idle");

  // Switch to main view
  view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewMain);

  // Start timer
  app->state_timer = furi_event_loop_timer_alloc(
      app->event_loop, timer_callback, FuriEventLoopTimerTypePeriodic, app);
  furi_event_loop_timer_start(app->state_timer, 1000);

  // Run event loop
  view_dispatcher_run(app->view_dispatcher);

  // Cleanup
  furi_event_loop_timer_free(app->state_timer);
  view_dispatcher_remove_view(app->view_dispatcher, FilmDeveloperViewMain);
  view_dispatcher_remove_view(app->view_dispatcher, FilmDeveloperViewPauseMenu);
  view_port_enabled_set(app->view_port, false);
  view_port_free(app->view_port);
  delete app->pause_menu;
  view_dispatcher_free(app->view_dispatcher);
  furi_record_close(RECORD_GUI);
  furi_event_loop_free(app->event_loop);

  // Clean up motor controller
  motorController.deinitGpio();
  furi_record_destroy("film_developer");

  free(app);
  return 0;
}

#ifdef __cplusplus
}
#endif
