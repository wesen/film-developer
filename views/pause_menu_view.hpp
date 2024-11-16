#pragma once
#include <gui/view.h>

class PauseMenuView {
public:
    typedef void (*PauseMenuCallback)(void* context);

    PauseMenuView();
    ~PauseMenuView();

    View* get_view();
    void set_rewind_callback(PauseMenuCallback callback, void* context);
    void set_skip_callback(PauseMenuCallback callback, void* context);
    void set_back_callback(PauseMenuCallback callback, void* context);
    void update_time(uint32_t elapsed_time, uint32_t total_time);

public:
    View* view;
    PauseMenuCallback rewind_callback;
    PauseMenuCallback skip_callback;
    PauseMenuCallback back_callback;
    void* rewind_context;
    void* skip_context;
    void* back_context;
}; 