#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <notification/notification.h>
#include "views/main_view.hpp"
#include "views/pause_menu_view.hpp"
#include "scenes/film_developer_scene.hpp"

typedef struct {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    // Views
    MainView* main_view;
    PauseMenuView* pause_menu;

    // Process state
    AgitationProcessInterpreter process_interpreter;
    const AgitationProcessStatic* current_process;
    bool process_active;
    bool paused;

    // Motor controller
    MotorController* motor_controller;
} FilmDeveloperApp;

typedef enum {
    FilmDeveloperViewMain,
    FilmDeveloperViewPauseMenu,
} FilmDeveloperView; 