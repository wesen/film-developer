#include "../agitation_processes.hpp"
#include "../film_developer_i.hpp"

static void main_view_ok_callback(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    
    if(!app->process_active) {
        app->process_interpreter.init(&C41_FULL_PROCESS_STATIC, app->motor_controller);
        app->process_active = true;
        app->paused = false;
    } else if(app->process_interpreter.isWaitingForUser()) {
        app->process_interpreter.confirm();
    } else {
        app->paused = true;
        app->motor_controller->stop();
        scene_manager_next_scene(app->scene_manager, FilmDeveloperScenePause);
    }
}

static void main_view_left_callback(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    if(app->process_active) {
        app->motor_controller->stop();
        app->process_interpreter.reset();
    }
}

static void main_view_right_callback(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    if(app->process_active && !app->process_interpreter.isWaitingForUser()) {
        app->motor_controller->stop();
        app->process_interpreter.skipToNextStep();
    }
}

void film_developer_scene_main_on_enter(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    
    app->main_view->set_ok_callback(main_view_ok_callback, app);
    app->main_view->set_left_callback(main_view_left_callback, app);
    app->main_view->set_right_callback(main_view_right_callback, app);
    
    view_dispatcher_switch_to_view(app->view_dispatcher, FilmDeveloperViewMain);
}

bool film_developer_scene_main_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void film_developer_scene_main_on_exit(void* context) {
    FilmDeveloperApp* app = static_cast<FilmDeveloperApp*>(context);
    app->main_view->set_ok_callback(nullptr, nullptr);
    app->main_view->set_left_callback(nullptr, nullptr);
    app->main_view->set_right_callback(nullptr, nullptr);
} 