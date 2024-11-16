#include "../film_developer_i.hpp"

static void pause_menu_rewind_callback(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    app->process_interpreter.reset();
    app->paused = false;
    scene_manager_previous_scene(app->scene_manager);
}

static void pause_menu_skip_callback(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    app->process_interpreter.skipToNextStep();
    app->paused = false;
    scene_manager_previous_scene(app->scene_manager);
}

static void pause_menu_back_callback(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    app->paused = false;
    scene_manager_previous_scene(app->scene_manager);
}

void film_developer_scene_pause_on_enter(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    
    app->pause_menu->set_rewind_callback(pause_menu_rewind_callback, app);
    app->pause_menu->set_skip_callback(pause_menu_skip_callback, app);
    app->pause_menu->set_back_callback(pause_menu_back_callback, app);
    
    app->pause_menu->update_time(
        app->process_interpreter.getCurrentMovementTimeElapsed(),
        app->process_interpreter.getCurrentMovementDuration());
        
    view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewPauseMenu);
}

bool film_developer_scene_pause_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void film_developer_scene_pause_on_exit(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    app->pause_menu->set_rewind_callback(nullptr, nullptr);
    app->pause_menu->set_skip_callback(nullptr, nullptr);
    app->pause_menu->set_back_callback(nullptr, nullptr);
} 