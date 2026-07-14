#pragma once

static const char* tooltip_EMPTY = "";

// UI menus
static const char* tooltip_menu_camera = "to be removed";
static const char* tooltip_menu_debug = "turn debug features on or off,\nchange colors and sizes of stuff, etc.";
static const char* tooltip_menu_fx = "submenus: particle systems, wind, weather, lights.";
static const char* tooltip_menu_input = "change the input bindings for the editor.\ncovers all keys, mouse, and gamepad.\nthis is global (across all projects).\nyou can also change the input bindings for your current project,\nas well as rename/create/delete actions to bind inputs to.\nif you want the user to be able to do this in the finished project,\nthen you can choose the input mapping widget template in the ui menu.\nsubmenus: camera, input bindings, cinematics, controllers.";
static const char* tooltip_menu_lights = "to be removed";
static const char* tooltip_menu_main = "this is the main menu.\nyou can find the latest readme, FAQ, news, updates,\nand your local notes (.txt files in the engine's /notes folder you can use to write stuff down.";
static const char* tooltip_menu_meshes = "for all your geometry\n.you can change or make materials, textures, bake them.\nyou can change between 2d sprites and 3d models.\nyou can modify the meshes, the geometry, sculpt, etc.\nsubmenus: textures, meshes, sprites, terrain, materials.";
static const char* tooltip_menu_project = "load/save/delete/create projects.\nthere are templates to start from.\nconfigure your projects at the highest level here and build, test, cook, package, and publish.\nyour project assets (shaders, meshes, LODs, lightmaps, cubemaps, etc.) automatically refresh as you restart the engine or load a project,\nbut you can also manually do it here.";
static const char* tooltip_menu_render = "configure the rendering.\nthis includes all your pipelines, how theyre set up, and which shaders they use\n.";
static const char* tooltip_menu_shaders = "whatever shaders you have set up for the active project in the render menu appears here.\nadjust params on built-in shaders, edit shader code manually in the text editor, or write your own.\nshaders hot reload when you save inside the text editor.\n";
static const char* tooltip_menu_sounds = "all the audio.\nchange audio settings in-editor.\nconfigure the audio systems for your project.\nplace, move, bind sound sources in the world.\nconfigure sound zones for e.g. reverb in the world.";
static const char* tooltip_menu_terrain = "to be removed";
static const char* tooltip_menu_ui = "edit the in-editor UI,\nchange colors, fonts, sizes, etc.\nyou can also edit your project's UI,\nmeaning create and define UI contexts and behavior based on widget templates.\ne.g. your project can have different UI menus with settings, input bindings, inventory, world map, etc.";

// UI submenus

// UI sub-submenus

// UI params
static const char* tooltip_PARAM_INPUT_MOUSE_CURSOR_SENSITIVITY = "sensitivity of the software cursor.\nNOT the mouse cursor in the UI.\nmore like a multiplier to your OS cursor speed for whatever action you have bound the mouse cursor to in your input map.\nthe default control is of the in-editor camera.";
static const char* tooltip_PARAM_INPUT_UI_TUNER_BASE_SPEED = "initial speed of ui tuners.";
static const char* tooltip_PARAM_INPUT_UI_TUNER_MAX_SPEED = "max speed of ui tuners.";
static const char* tooltip_PARAM_INPUT_UI_TUNER_ACCEL_TIME = "acceleration of ui tuners.\nseconds it takes to reach max speed from base speed.";
static const char* tooltip_PARAM_RENDER_DEBUG_ON = "global toggle for debug visualizatios.\nworld grid, gizmos, axes, collision boxes, lights, etc.";
static const char* tooltip_PARAM_RENDER_LOD1_DISTANCE = "at what distance from the camera this LOD starts for meshes.";
static const char* tooltip_PARAM_RENDER_SPATIAL_STREAM_CHECK_INTERVAL = "how often (in seconds) to perform frustum and distance culling.\ncan be increased for performance gains but should be kept 0,\nbc higher values cause meshes to noticably pop in, especially with rapid camera movement.";
static const char* tooltip_PARAM_RENDER_SPATIAL_STREAM_RADIUS = "meshes in chunks outside of this radius will be distance culled.\naka view distance.\nkeeping this to low values is the main performance gainy thingy.";
static const char* tooltip_PARAM_RENDER_SHADERS_SKY_24_HOUR_CYCLE_SPEED = "0 = cycle is disabled.";

