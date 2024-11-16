#pragma once
#include "../agitation_process_interpreter.hpp"
#include <gui/view.h>

class MainView {
public:
  typedef void (*MainViewCallback)(void *context);

  MainView();
  ~MainView();

  View *get_view();
  void set_ok_callback(MainViewCallback callback, void *context);
  void set_left_callback(MainViewCallback callback, void *context);
  void set_right_callback(MainViewCallback callback, void *context);
  void update_state(const AgitationProcessInterpreter &interpreter,
                    bool process_active, bool paused);

  MainViewCallback ok_callback;
  MainViewCallback left_callback;
  MainViewCallback right_callback;
  void *ok_context;
  void *left_context;
  void *right_context;

private:
  View *view;
};